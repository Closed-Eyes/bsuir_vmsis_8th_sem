#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QDir>
#include <QVector>
#include <QDebug>

#include "hopfieldnetwork.h"
#include "perceptron.h"

#define SAMPLE_AMOUNT 5

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
    void browseSampleImages();
    void browseTestImage();
    void adjustImage();
    void recognize();
    
private:
    Ui::MainWindow *ui;
    QString sampleImagePath;
    QString testImagePath;

    QLabel *imageLabels[SAMPLE_AMOUNT];
    QLabel *similarityLabels[SAMPLE_AMOUNT];

    QImage sampleImages[SAMPLE_AMOUNT];
    QImage testImage;
    QImage defaultTestImage;
};

#endif // MAINWINDOW_H
