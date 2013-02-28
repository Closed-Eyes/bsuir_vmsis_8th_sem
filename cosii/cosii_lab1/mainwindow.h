#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QMap>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QList>

#include "converter.h"
#include "analiser.h"

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
    void openFile();
    void detect();
    void adjust();   
    
private:
    Ui::MainWindow *ui;

    cv::Mat imageToProcess;
    QString fileName;

    int maxImageWidth;
    int maxImageHeight;

};

#endif // MAINWINDOW_H
