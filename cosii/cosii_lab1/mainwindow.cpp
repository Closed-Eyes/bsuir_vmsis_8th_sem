#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->detectButton, &QPushButton::pressed, this, &MainWindow::detect);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile()
{
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open file"),
                                            QDir::currentPath());
    if(!fileName.isEmpty()) {
        cv::Mat image_rgb = cv::imread(fileName.toStdString());

        if(image_rgb.data == NULL) {
            QMessageBox::information(this,
                                     tr("Error"),
                                     tr("Cannot load %1.").arg(fileName));
            return;
        }

        image = Converter::Mat2QImage(image_rgb);

        ui->imageLabel->setPixmap(QPixmap::fromImage(image));

        ui->imageLabel->adjustSize();
        this->resize(image.width(), image.height() + ui->menuBar->height() + ui->clasterSpinBox->height() + 5);

        ui->detectButton->setEnabled(true);
        ui->clasterSpinBox->setEnabled(true);
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
    cv::Mat image_gray;
    cv::cvtColor(image_rgb, image_gray, CV_RGB2GRAY); // to grayscale

    cv::Mat image_blured;
    cv::medianBlur(image_gray, image_blured, 7); // ADJUSTABLE

    cv::Mat image_smoothed;
    cv::bilateralFilter(image_blured, image_smoothed, 5, 120, 100);

    cv::imwrite("image_sm.jpg", image_smoothed);

    cv::Mat image_bw;

    image_bw = image_blured > 180; // binarization ADJUSTABLE

    cv::imwrite("image_bw.jpg", image_bw);

    Analiser analiser;
    analiser.claster(image_bw, ui->clasterSpinBox->value());
    analiser.paintClasters(image_rgb);

    image = Converter::Mat2QImage(image_rgb);

    ui->imageLabel->setPixmap(QPixmap::fromImage(image));

    ui->imageLabel->adjustSize();
    this->resize(image.width(), image.height() + ui->menuBar->height() + ui->clasterSpinBox->height() + 5);
}
