#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QDebug>

#include "hopfieldnetwork.h"

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
    void browseSampleImage();
    void browseTestImage();
    void adjustImage();
    void recognize();
    
private:
    Ui::MainWindow *ui;
    QString sampleImagePath;
    QString testImagePath;

    QImage sampleImage;
    QImage testImage;
    QImage defaultTestImage;
};

#endif // MAINWINDOW_H
