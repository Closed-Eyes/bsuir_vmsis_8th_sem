#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <intrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <Windows.h>

inline __int64 GetCpuClocks()
{
	// Counter
	struct {__int32 low, high;} counter;
	
	// Use RDTSC instruction to get clocks count
	__asm push EAX
	__asm push EDX
	__asm __emit 0fh __asm __emit 031h //RDTSC
	__asm mov counter.low, EAX
	__asm mov counter.high, EDX
	__asm pop EDX
	__asm pop EAX

	// Return result
	return *(__int64*)(&counter);
}

unsigned __int64 getTicks(void)
{
	return __rdtsc();
}

float ProcSpeedCalc()
{
/*	RdTSC:
	It's the Pentium instruction "ReaD Time Stamp Counter". It measures the
	number of clock cycles that have passed since the processor was reset, as a
	64-bit number.  */

#define RdTSC __asm _emit 0x0f __asm _emit 0x31

	// variables for the clock-cycles:
	__int64 cyclesStart = 0, cyclesStop = 0;
	// variables for the High-Res Preformance Counter:
	unsigned __int64 nCtr = 0, nFreq = 0, nCtrStop = 0;


    // retrieve performance-counter frequency per second:
    if(!QueryPerformanceFrequency((LARGE_INTEGER *) &nFreq)) return 0;

    // retrieve the current value of the performance counter:
    QueryPerformanceCounter((LARGE_INTEGER *) &nCtrStop);

    // add the frequency to the counter-value:
    nCtrStop += nFreq;


    _asm
        {// retrieve the clock-cycles for the start value:
            RdTSC
            mov DWORD PTR cyclesStart, eax
            mov DWORD PTR [cyclesStart + 4], edx
        }

        do{
        // retrieve the value of the performance counter
        // until 1 sec has gone by:
             QueryPerformanceCounter((LARGE_INTEGER *) &nCtr);
          }while (nCtr < nCtrStop);

    _asm
        {// retrieve again the clock-cycles after 1 sec. has gone by:
            RdTSC
            mov DWORD PTR cyclesStop, eax
            mov DWORD PTR [cyclesStop + 4], edx
        }

// stop-start is speed in Hz divided by 1,000,000 is speed in MHz
return    ((float)cyclesStop-(float)cyclesStart);
}

int getIndex(int x, int y, int z, int k)
{
	return k*(150*150*8) + z*(150*150)+ y*8 + x;
}

int main()
{
	double *mtx1 = (double*)_aligned_malloc(150*150*8*4*sizeof(double), 32);
	
	double *mtx2 = (double*)_aligned_malloc(150*150*4*8*sizeof(double), 32);
			
	double *mtx3 = (double*)_aligned_malloc(150*150*8*8*sizeof(double), 32);
		
	unsigned __int64 tick1, tick2, averageTicks;

	std::cout << "Test. 1 pass" << std::endl; 


		
	srand(time(NULL));

	for(int i = 0; i < 150; i++){
		for(int j = 0; j < 150; j++){
			for(int k = 0; k < 8; k++){
				for(int m = 0; m < 4; m++){
					mtx1[i*150*8*4 + j*8*4 + k*4 + m] = i; // rand()%150;
				}
			}
		}
	}

	for(int i = 0; i < 150; i++){
		for(int j = 0; j < 150; j++){
			for(int k = 0; k < 4; k++){
				for(int m = 0; m < 8; m++){
					mtx2[i*150*4*8 + j*4*8 + k*8 + m] = j;
				}
			}
		}
	}

	averageTicks = 0;

	for(int pass = 0; pass < 1; pass++){

		for(int i = 0; i < 150; i++){
			for(int j = 0; j < 150; j++){
				for(int k = 0; k < 8; k++){
					for(int m = 0; m < 8; m++){
						mtx3[i*150*8*8 + j*8*8 + k*8 + m] = 0;
					}
				}
			}
		}		

		std::cout << "Calculation has started. Pass number " << pass <<std::endl;

		__m128d a, b1,b2,b3,b4, c1,c2,c3,c4, p1,p2,p3,p4, r1,r2,r3,r4;		

		tick1 = getTicks();
	
		for(int i = 0; i < 150; i++){
			for(int j = 0; j < 150; j++){			
				for(int k = 0; k < 150; k++){
					
					for(int k0 = 0; k0 < 4; k0 ++){		
						double *ptr2 = mtx2 + k*150*8*4 + j*8*4 + k0*8;
						for(int i0 = 0; i0 < 8; i0++){		
							double temp1 = mtx1[i*150*8*4 + k*8*4 + i0*4 + k0];
							double *ptr1 = mtx3 + i*150*8*8 + j*8*8 + i0*8;						
							
							a = _mm_set_pd(temp1, temp1);
							
							b1 = _mm_load_pd(ptr2);  // 0 - 1
							b2 = _mm_load_pd(ptr2 + 2); // 2-3
							b3 = _mm_load_pd(ptr2 + 4); // 4-5	
							b4 = _mm_load_pd(ptr2 + 6); // 6-7

							p1 = _mm_load_pd(ptr1);  // 0 - 1
							p2 = _mm_load_pd(ptr1 + 2); // 2-3
							p3 = _mm_load_pd(ptr1 + 4); // 4-5	
							p4 = _mm_load_pd(ptr1 + 6); // 6-7
							
							c1 = _mm_mul_pd(a,b1);
							c2 = _mm_mul_pd(a,b2);
							c3 = _mm_mul_pd(a,b3);
							c4 = _mm_mul_pd(a,b4);

							r1 = _mm_add_pd(p1, c1);
							r2 = _mm_add_pd(p2, c2);
							r3 = _mm_add_pd(p3, c4);
							r4 = _mm_add_pd(p4, c4);

							_mm_storeu_pd(ptr1, r1);
							_mm_storeu_pd(ptr1 + 2, r2);	
							_mm_storeu_pd(ptr1 + 4, r3);
							_mm_storeu_pd(ptr1 + 6, r4);	
						}
					}					
				}
			}
		}

		tick2 = getTicks();

		averageTicks += tick2 - tick1;
	}
	
	double time = (double)(averageTicks/1.0)/(ProcSpeedCalc());

	std::cout << std::endl;
	std::cout << "Time:  " << time << std::endl;
	std::cout << "Ticks: " << averageTicks/1 << std::endl;
	std::cout << mtx3[4*150*8*8 + 3*8*8 + 2*8 + 3] << std::endl;
	system("pause");

	return 0;
}