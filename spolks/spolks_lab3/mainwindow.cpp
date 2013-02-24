#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //ui->chatTextBox->setEnabled(false);
    ui->sendTextBox->setEnabled(false);
    ui->sendPushButton->setEnabled(false);

    ui->chatTextBox->setReadOnly(true);

    connect(ui->joinPushButton, SIGNAL(clicked()), this, SLOT(joinChat()));
    connect(ui->sendPushButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(recvMessage()));

    /*Fill in remote sockaddr_in */
    bzero(&remoteAddress, sizeof(remoteAddress));
    remoteAddress.sin_family = PF_INET;
    remoteAddress.sin_port = htons(PORT);
    remoteAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);//inet_addr("192.168.0.255");

    /*Fill in server's sockaddr_in */
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = PF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);


    sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1)
        QMessageBox::information(this, "Error: socket", strerror(errno), QMessageBox::Ok);

    int broadcastOn = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastOn, 4) == -1)
        QMessageBox::information(this, "Error: setsockopt", strerror(errno), QMessageBox::Ok);

    if (bind(sockfd, (struct sockaddr* ) &serverAddress, sizeof(serverAddress)) == -1)
        QMessageBox::information(this, "Error: bind", strerror(errno), QMessageBox::Ok);

    this->setFixedSize(this->size());
}

MainWindow::~MainWindow()
{
    ::close(sockfd);
    delete ui;
}

void MainWindow::joinChat()
{
    if (ui->nickTextBox->text() == "") {
        nickname = "Anonymous";
        ui->nickTextBox->setText(nickname);
    } else {
        nickname = ui->nickTextBox->text();
    }

    ui->sendTextBox->setEnabled(true);
    ui->sendPushButton->setEnabled(true);

    ui->nickTextBox->setEnabled(false);
    ui->joinPushButton->setEnabled(false);

    timer.start(100);
}

void MainWindow::sendMessage()
{
    if(ui->sendTextBox->toPlainText() == "") // nothing to send
        return;

    struct Message
    {
        char type;
        char nickname[16];
        char data[249];
    } *message;

    char data[256];

    bzero(&data, sizeof(data));
    message = (struct Message*)data;

    message->type = MESSAGE_REGULAR;
    strcpy(message->nickname, nickname.toStdString().c_str());
    strcpy(message->data, ui->sendTextBox->toPlainText().toStdString().c_str());

    int ret = sendto(sockfd, data, sizeof(data), 0, (struct sockaddr *) &remoteAddress, (socklen_t)sizeof(remoteAddress));
    if (ret == -1)
        QMessageBox::information(this, "Error: sendto", strerror(errno), QMessageBox::Ok);

    ui->sendTextBox->setText("");
}

void MainWindow::recvMessage()
{
    struct Message
    {
        char type;
        char nickname[16];
        char data[249];
    } *message;

    char data[256];

    fd_set socks;
    struct timeval t;
    FD_ZERO(&socks);
    FD_SET(sockfd, &socks);
    t.tv_sec = 0;
    t.tv_usec = 50000; //50 ms
    if (select(sockfd + 1, &socks, NULL, NULL, &t)) {
        int fromLength = sizeof(serverAddress);
        recvfrom(sockfd, data, 256, 0, (struct sockaddr *) &serverAddress, (socklen_t *)&fromLength);

        message = (struct Message*)data;

        if (message->type == MESSAGE_REGULAR) {

            QString string;
            string += message->nickname;
            string += " > ";
            string += message->data;

            ui->chatTextBox->append(string);
        }
    }
}
