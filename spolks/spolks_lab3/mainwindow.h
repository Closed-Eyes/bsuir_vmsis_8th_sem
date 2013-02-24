#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QShortcut>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define PORT 49151
#define MESSAGE_REGULAR 0

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void joinChat();
    void sendMessage();
    void recvMessage();
    
private:
    Ui::MainWindow *ui;
    QTimer timer;

    struct sockaddr_in remoteAddress;
    struct sockaddr_in serverAddress;
    int sockfd;

    QString nickname;
};

#endif // MAINWINDOW_H
