#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->browseSampleButton, SIGNAL(clicked()), this, SLOT(browseSampleImage()));
    connect(ui->browseTestButton, SIGNAL(clicked()), this, SLOT(browseTestImage()));
    connect(ui->recognizeButton, SIGNAL(clicked()), this, SLOT(recognize()));
    connect(ui->adjustButton, SIGNAL(clicked()), this, SLOT(adjustImage()));

    ui->recognizeButton->setEnabled(false);
    ui->noiseSpinBox->setEnabled(false);
    ui->adjustButton->setEnabled(false);

    this->setFixedSize(this->size());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browseSampleImage()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open sample file"),
                                                    QDir::homePath(),
                                                    tr("Images (*.png *.bmp *.jpg)"));
    if (!filePath.isEmpty()) {
        sampleImagePath = filePath;

        if (!sampleImage.load(sampleImagePath)) {
            QMessageBox::information(this,
                                     tr("Error"),
                                     tr("Failed loading %1").arg(sampleImagePath));
            sampleImagePath = "";
            return;
        }

        ui->sampleImageLabel->setPixmap(QPixmap::fromImage(sampleImage.scaled(200, 200)));

        // both images are already opened
        if (!testImagePath.isEmpty()) {
            ui->recognizeButton->setEnabled(true);
        }
    }
}

void MainWindow::browseTestImage()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open test file"),
                                                    QDir::homePath(),
                                                    tr("Images (*.png *.bmp *.jpg)"));
    if (!filePath.isEmpty()) {
        testImagePath = filePath;

        if (!testImage.load(testImagePath)) {
            QMessageBox::information(this,
                                     tr("Error"),
                                     tr("Failed loading %1").arg(testImagePath));
            testImagePath = "";
            return;
        }
        defaultTestImage.load(testImagePath);

        ui->testImageLabel->setPixmap(QPixmap::fromImage(testImage.scaled(200, 200)));

        // both images are already opened
        if (!sampleImagePath.isEmpty()) {
            ui->recognizeButton->setEnabled(true);
        }

        ui->noiseSpinBox->setEnabled(true);
        ui->adjustButton->setEnabled(true);
    }
}

void MainWindow::recognize()
{
    HopfieldNetwork network;

    network.teach(sampleImage);
    QImage resultImage = network.recognize(testImage);

    ui->resultImageLabel->setPixmap(QPixmap::fromImage(resultImage).scaled(200, 200));
}

void MainWindow::adjustImage()
{
    int noiseTreshold = ui->noiseSpinBox->value();
    qsrand(time(NULL));

    testImage = defaultTestImage;

    // invert some random pixels / add noise to image
    for (int i = 0; i < testImage.height(); i++) {
        for (int j = 0; j < testImage.width(); j++) {

            if ((qrand() % 100) < noiseTreshold) {

                QColor pixel = testImage.pixel(i, j);

                pixel.setRgb(255 - pixel.red(), 255 - pixel.green(), 255 - pixel.blue());
                testImage.setPixel(i, j, pixel.rgb());
            }
        }
    }

    ui->testImageLabel->setPixmap(QPixmap::fromImage(testImage.scaled(200, 200)));
}
