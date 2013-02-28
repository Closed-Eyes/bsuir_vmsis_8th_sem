#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui->detectButton, SIGNAL(clicked()), this, SLOT(detect()));
    connect(ui->adjustButton, SIGNAL(clicked()), this, SLOT(adjust()));

    this->setFixedSize(this->size());

    maxImageHeight = ui->imageLabel->height();
    maxImageWidth = ui->imageLabel->width();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile()
{
    QString fileToOpen = QFileDialog::getOpenFileName(this,
                                            tr("Open file"),
                                            QDir::homePath());
    if(!fileToOpen.isEmpty()) {
        cv::Mat image_rgb = cv::imread(fileToOpen.toStdString());

        if(image_rgb.data == NULL) {
            QMessageBox::information(this,
                                     tr("Error"),
                                     tr("Cannot load %1.").arg(fileToOpen));
            return;
        }

        QImage img = Converter::Mat2QImage(image_rgb);
        img = img.scaled(img.width() < maxImageWidth ? img.width() : maxImageWidth,
                         img.height() < maxImageHeight ? img.height() : maxImageHeight,
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);

        ui->imageLabel->setPixmap(QPixmap::fromImage(img));

        fileName = fileToOpen;
    }
}

void MainWindow::detect()
{
    cv::Mat image_rgb = cv::imread(fileName.toStdString());

    if(image_rgb.data == NULL) {
        QMessageBox::information(this,
                                 tr("Error"),
                                 tr("Cannot load %1.").arg(fileName));
        return;
    }

    Analiser analiser;
    analiser.claster(imageToProcess, ui->clasterSpinBox->value());
    analiser.paintClasters(image_rgb);

    QImage img = Converter::Mat2QImage(image_rgb);
    img = img.scaled(img.width() < maxImageWidth ? img.width() : maxImageWidth,
                     img.height() < maxImageHeight ? img.height() : maxImageHeight,
                     Qt::KeepAspectRatio,
                     Qt::SmoothTransformation);

    ui->imageLabel->setPixmap(QPixmap::fromImage(img));

    ui->detectButton->setEnabled(false);
    ui->clasterSpinBox->setEnabled(false);

    QMessageBox::information(this,
                             "Info",
                             QString("Number of iterations: %1").arg(analiser.getIterationsNum()));
}

void MainWindow::adjust()
{
    cv::Mat imageRgb = cv::imread(fileName.toStdString());

    if(imageRgb.data == NULL) {
        QMessageBox::information(this,
                                 tr("Error"),
                                 tr("Cannot load %1.").arg(fileName));
        return;
    }
    /*
    cv::Mat image_smoothed;
    cv::bilateralFilter(image_rgb, image_smoothed, ui->bDiameterSpinBox->value(), ui->bSigmaColorSpinBox->value(), ui->bSigmaSpaceSpinBox->value());
    cv::Mat image_blured;
    cv::medianBlur(image_gray, image_blured, ui->mbKernelSpinBox->value());
    */

    cv::Mat imageBlured;
    cv::medianBlur(imageRgb, imageBlured, ui->medianKernelSpinBox->value());

    cv::Mat imageGray;
    cv::cvtColor(imageBlured, imageGray, CV_RGB2GRAY);

    cv::Mat imageSmoothed;
    cv::GaussianBlur(imageGray,
                     imageSmoothed,
                     cv::Size(0, 0),
                     ui->gaussSigmaXSpinBox->value(),
                     ui->gaussSigmaYSpinBox->value());

    cv::Mat imageBw;
    imageBw = imageSmoothed > ui->binSpinBox->value(); // binarization


    // Calculate amount of bw pixels
    // so we can know, which color backgroung is

    int whitePixels = 0;
    int blackPixels = 0;

    for(int i = 0; i < imageBw.rows; i++) {
        for(int j = 0; j < imageBw.cols; j++) {
            if(imageBw.data[imageBw.channels()*(imageBw.cols*i + j)] == 255)
                whitePixels++;
            if(imageBw.data[imageBw.channels()*(imageBw.cols*i + j)] == 0)
                blackPixels++;
        }
    }

    // If there are more white pixels, so background is white
    // and we need to inverse colors

    if(whitePixels > blackPixels) {
        for(int i = 0; i < imageBw.rows; i++) {
            for(int j = 0; j < imageBw.cols; j++) {
                imageBw.data[imageBw.channels()*(imageBw.cols*i + j)] = 255 - imageBw.data[imageBw.channels()*(imageBw.cols*i + j)];
            }
        }
    }

    QImage img = Converter::Mat2QImage(imageBw);
    img = img.scaled(img.width() < maxImageWidth ? img.width() : maxImageWidth,
                     img.height() < maxImageHeight ? img.height() : maxImageHeight,
                     Qt::KeepAspectRatio,
                     Qt::SmoothTransformation);

    ui->imageLabel->setPixmap(QPixmap::fromImage(img));

    imageToProcess = imageBw;

    ui->detectButton->setEnabled(true);
    ui->clasterSpinBox->setEnabled(true);
}
