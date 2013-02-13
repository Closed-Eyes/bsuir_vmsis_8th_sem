#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
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
        cv::Mat image_gray;
        cv::cvtColor(image_rgb, image_gray, CV_RGB2GRAY); // to grayscale
        //cv::bilateralFilter()

        cv::Mat image_blured;
        cv::medianBlur(image_gray, image_blured, 9); // ADJUSTABLE


        cv::Mat image_bw;

        image_bw = image_blured > 120; // binarization ADJUSTABLE

        cv::imwrite("image_bw.jpg", image_bw);

        Analiser analiser;
        analiser.createLabelsMap(image_bw);
        analiser.paintImage(image_rgb);
        analiser.claster(4);

        image = Converter::Mat2QImage(image_rgb);

        ui->imageLabel->setPixmap(QPixmap::fromImage(image));

        ui->imageLabel->adjustSize();
        this->resize(image.width(), image.height() + ui->menuBar->height() + ui->statusBar->height());
    }
}
