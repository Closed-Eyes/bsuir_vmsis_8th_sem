#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QDir>
#include <QVector>
#include <QList>
#include <QDebug>
#include <algorithm>

#include "hopfieldnetwork.h"
#include "perceptron.h"
#include "rbf.h"
#include "selection.h"

#define SAMPLE_AMOUNT 12
#define CLASS_AMOUNT 3
#define SAMPLES_PER_CLASS 4

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
    QLabel *similarityLabels[CLASS_AMOUNT];

    QImage sampleImages[SAMPLE_AMOUNT];
    QImage testImage;
    QImage defaultTestImage;   
};

#endif // MAINWINDOW_H
