#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <signal.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <errno.h>

using namespace std;

#define	DEFDATALEN	(64-ICMP_MINLEN)
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	RECV_PACK_LEN	4096
#define SEND_PACK_LEN DEFDATALEN + ICMP_MINLEN

string destination = "";
string source = "";
int ttl = 0;

// Statistic variables
int sentPackets;
int recvPackets;
double averageTime;

void tv_sub(struct timeval *out, struct timeval *in)
{
	if ( (out->tv_usec -= in->tv_usec) < 0) {	/* out -= in */
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

uint16_t in_cksum(uint16_t *addr, unsigned len)
{
	uint16_t answer = 0;
	/*
	* Our algorithm is simple, using a 32 bit accumulator (sum), we add
	* sequential 16 bit words to it, and at the end, fold back all the
	* carry bits from the top 16 bits into the lower 16 bits.
	*/
	uint32_t sum = 0;
	while (len > 1) {
		sum += *addr++;
		len -= 2;
	}

	// mop up an odd byte, if necessary
	if (len == 1) {
		*(unsigned char *)&answer = *(unsigned char *)addr ;
		sum += answer;
	}

	// add back carry outs from top 16 bits to low 16 bits
	sum = (sum >> 16) + (sum & 0xffff); // add high 16 to low 16
	sum += (sum >> 16); // add carry
	answer = ~sum; // truncate to 16 bits
	return answer;
}

int ping()
{
    int s,         /* socket descriptor             */
		ret,
		iplen,     /* IP header lenght              */
		fromlen;        
		
	struct sockaddr_in from;
	
	struct ip *ip;
	struct icmp *icmp;	
	
	char inPacket[RECV_PACK_LEN];	

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)	{
		cerr << "Error: Can't creat socket. You're probably not a superuser" << endl;
		return -1;
	}	
	
	sentPackets = 0;
	recvPackets = 0;
	averageTime = 0;	
	
	struct timeval end;	
	struct timeval *start;
	
	fd_set rfds;
	
	while(true)	{		
		fromlen = sizeof(sockaddr_in);
		
		ret = recvfrom(s, inPacket, RECV_PACK_LEN, 0,(struct sockaddr *)&from, (socklen_t*)&fromlen);
		if (ret < 0) {
			cerr << "Error: recvfrom" << endl;
			return -1;
		}

		/* IP header */
		ip = (struct ip *) inPacket; 
		iplen = ip->ip_hl << 2; 			

		/* ICMP part */ 
		icmp = (struct icmp *)(inPacket + iplen); 
		
		if( ret - iplen < 8 ) {
			cerr << "Error: icmp packet lenght = " << ret - iplen <<endl;
		}			
			
		if (icmp->icmp_type != ICMP_ECHOREPLY) { // not echo reply
			continue;
		}
		
		if (icmp->icmp_id != getpid()) {    // not our echo reply
			continue;
		}						
			
		gettimeofday(&end, NULL);
		start = (struct timeval *) icmp->icmp_data;
			
		tv_sub( &end, start);
			
		double result = end.tv_sec * 1000.0 + end.tv_usec / 1000.0; // to milliseconds	
						
		cout << "Echo reply from "<< inet_ntoa(from.sin_addr) << " Seq_num = " << icmp->icmp_seq << " Time = " << result <<"ms" <<endl;	
		fflush(stdout);
			
		/* statistics */
		averageTime += result;
		recvPackets ++;				
	}
	return 0;
}

void catcher(int signum)
{
	if(signum == SIGINT) {		
		cout << endl <<"---- Statistics ---- " << endl;
	
		cout << "Packets sent:      " << sentPackets << endl;
		cout << "Packets recieved:  " << recvPackets << endl;
		cout << "Packets lost:      " << sentPackets - recvPackets << endl;
		cout << "Average time:      " << averageTime / (double)recvPackets << " ms"<<endl;
		exit(0);
	}
	
	if(signum == SIGALRM) {
		
		int s,         /* socket descriptor             */ 
			cc,        /* icmp packet size for checksum */
			ret,	   /* return value for functions    */	
			iplen,     /* IP header lenght              */
			fromlen; 
		
		struct sockaddr_in to, from;
	
		struct ip *ip;
		struct icmp *icmp;	
	
		char outPacket[SEND_PACK_LEN];
	
		to.sin_family = AF_INET;
		to.sin_addr.s_addr = inet_addr(destination.c_str());
		from.sin_family = AF_INET;
		from.sin_addr.s_addr = inet_addr(source.c_str());
	
		if(to.sin_addr.s_addr == INADDR_NONE) {
			cerr << "Error: Enter correct address!" << endl;
			exit(1);
		}		

		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)	{
			cerr << "Error: Can't creat socket. You're probably not a superuser" << endl;
			exit(1);
		}
		
		int on = 1;
		if(setsockopt(s, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0) {
			cerr << "Error: Can't set IP_HDRINCL option" << endl;
			exit(1);
		}		
		
		bzero((void*)outPacket, sizeof(outPacket));
		
		/* fill up ip header */			
		ip = (struct ip *)outPacket;		
		ip->ip_v   = 4;			            /*4-bit version*/
		ip->ip_hl  = 5;	    	            /*4-bit header lenght in 32 bit words*/
		ip->ip_len = htons(sizeof(outPacket));	/*16-bit total packet lenght*/
		ip->ip_id  = htons(getpid());		/*16-bit packet unique number*/
		ip->ip_ttl = ttl;		            /*8-bit ttl*/
		ip->ip_p   = IPPROTO_ICMP;		    /*8-bit high level protocol*/
		
		ip->ip_sum = (unsigned short)in_cksum((unsigned short *)ip, sizeof(struct ip));
		
		ip->ip_src = from.sin_addr;  /*32-bit source address*/
		ip->ip_dst = to.sin_addr;	/*32-bit destination address*/
		
		/* fill up icmp header */
		icmp = (struct icmp *)(outPacket + sizeof(struct ip));
		icmp->icmp_type  = ICMP_ECHO;
		icmp->icmp_code  = 0;
		icmp->icmp_cksum = 0;           /* must be 0!                   */
		icmp->icmp_seq   = sentPackets;	/* seq and id must be reflected */
		icmp->icmp_id    = getpid();
		
		gettimeofday((struct timeval *) icmp->icmp_data, NULL);
		
		cc = SEND_PACK_LEN - 20;
		icmp->icmp_cksum = in_cksum((unsigned short *)icmp,cc);			

		ret = sendto(s, outPacket, cc, 0, (struct sockaddr*)&to, (socklen_t)sizeof(struct sockaddr_in));
		if (ret < 0 || ret != cc) {				
			cerr << "Error: sendto" << endl;			
		}
					
		/* statistics */
		sentPackets ++;		
		
		signal(SIGALRM, catcher);
	}
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		cout << "Usage: ping -d <host address> -s <source address> -t <ttl>" << endl;
		exit(1);
	}
	
	for(int i = 0; i < argc - 1; i++) {		
		if(strcmp(argv[i], "-d") == 0) {
			destination = argv[i + 1];			
		}
		
		if(strcmp(argv[i], "-s") == 0) {
			source = argv[i + 1];
		}
		if(strcmp(argv[i],"-t") == 0) {
			ttl = atoi(argv[i+1]);
		}		
	}	
	
	if(ttl <= 0) {
		ttl = 255;
	}	
	
	signal(SIGINT,  catcher);
	signal(SIGALRM, catcher);
	
	struct itimerval newTimer, oldTimer;	
	
	newTimer.it_interval.tv_sec = 1;
	newTimer.it_interval.tv_usec = 0;
	newTimer.it_value.tv_sec = 0;
	newTimer.it_value.tv_usec = 100;
		
	getitimer(ITIMER_REAL, &oldTimer);
	setitimer(ITIMER_REAL, &newTimer, &oldTimer);
		
	ping();	
	
	return 0;
}
