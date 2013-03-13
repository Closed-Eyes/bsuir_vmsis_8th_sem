#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->browseSampleButton, SIGNAL(clicked()), this, SLOT(browseSampleImages()));
    connect(ui->browseTestButton, SIGNAL(clicked()), this, SLOT(browseTestImage()));
    connect(ui->recognizeButton, SIGNAL(clicked()), this, SLOT(recognize()));
    connect(ui->adjustButton, SIGNAL(clicked()), this, SLOT(adjustImage()));

    ui->recognizeButton->setEnabled(false);
    ui->noiseSpinBox->setEnabled(false);
    ui->adjustButton->setEnabled(false);

    imageLabels[0] = ui->sample0ImageLabel;
    imageLabels[1] = ui->sample1ImageLabel;
    imageLabels[2] = ui->sample2ImageLabel;
    imageLabels[3] = ui->sample3ImageLabel;
    imageLabels[4] = ui->sample4ImageLabel;

    similarityLabels[0] = ui->similarity0Label;
    similarityLabels[1] = ui->similarity1Label;
    similarityLabels[2] = ui->similarity2Label;
    similarityLabels[3] = ui->similarity3Label;
    similarityLabels[4] = ui->similarity4Label;

    this->setFixedSize(this->size());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browseSampleImages()
{

    QString dirPath = QFileDialog::getExistingDirectory(this,
                                                        tr("Open sample file"),
                                                        QDir::homePath());
    if (!dirPath.isEmpty()) {
        sampleImagePath = dirPath;

        QDir imagesDir(dirPath);

        QStringList filters;
        filters << "*.png";
        imagesDir.setNameFilters(filters);

        QStringList list = imagesDir.entryList();

        if (list.count() < SAMPLE_AMOUNT) {
            QMessageBox::information(this,
                                     tr("Error"),
                                     tr("Not enough images in this directory"));
            sampleImagePath = "";
            return;
        }

        for (int i = 0; i < SAMPLE_AMOUNT; i++) {
            QString path = dirPath + "/" + list.takeFirst();

            if (!sampleImages[i].load(path)) {
                QMessageBox::information(this,
                                         tr("Error"),
                                         tr("Failed loading %1").arg(path));
                sampleImagePath = "";
                return;
            }
            imageLabels[i]->setPixmap(QPixmap::fromImage(sampleImages[i].scaled(150, 150)));

        }

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
                                                    tr("Images (*.png)"));
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

        ui->testImageLabel->setPixmap(QPixmap::fromImage(testImage.scaled(150, 150)));

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
    Perceptron perceptron(testImage.height() * testImage.width(), SAMPLE_AMOUNT);

    QList<QImage> images;

    for(int i = 0; i < SAMPLE_AMOUNT; i++) {
        images.append(sampleImages[i]);
    }

    perceptron.teach(images);

    QVector<double> result = perceptron.recognize(testImage);

    for(int i = 0; i < SAMPLE_AMOUNT; i++) {
        similarityLabels[i]->setText(QString("%1").arg(result[i]));
    }

    //QVector<double>::iterator it = std::max_element(result.begin(), result.end());
    int resultIndex = result.indexOf(*std::max_element(result.begin(), result.end()));

    ui->resultImageLabel->setPixmap(QPixmap::fromImage(sampleImages[resultIndex].scaled(150, 150)));

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

    ui->testImageLabel->setPixmap(QPixmap::fromImage(testImage.scaled(150, 150)));
}
