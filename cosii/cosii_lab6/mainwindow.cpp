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
    //connect(ui->teachButton, SIGNAL(clicked()), this, SLOT(teach()));

    ui->recognizeButton->setEnabled(false);
    ui->noiseSpinBox->setEnabled(false);
    ui->adjustButton->setEnabled(false);

    imageLabels[0] = ui->sample00ImageLabel;
    imageLabels[1] = ui->sample01ImageLabel;
    imageLabels[2] = ui->sample02ImageLabel;
    imageLabels[3] = ui->sample03ImageLabel;
    imageLabels[4] = ui->sample10ImageLabel;
    imageLabels[5] = ui->sample11ImageLabel;
    imageLabels[6] = ui->sample12ImageLabel;
    imageLabels[7] = ui->sample13ImageLabel;
    imageLabels[8] = ui->sample20ImageLabel;
    imageLabels[9] = ui->sample21ImageLabel;
    imageLabels[10] = ui->sample22ImageLabel;
    imageLabels[11] = ui->sample23ImageLabel;


    similarityLabels[0] = ui->similarity0Label;
    similarityLabels[1] = ui->similarity1Label;
    similarityLabels[2] = ui->similarity2Label;    

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

        for(int i = 0; i < CLASS_AMOUNT; i++) {
            QDir imagesDir(dirPath);

            QStringList filters;
            filters << QString("%1.*.png").arg(i);
            imagesDir.setNameFilters(filters);

            QStringList list = imagesDir.entryList();

            if (list.count() < SAMPLES_PER_CLASS) {
                QMessageBox::information(this,
                                         tr("Error"),
                                         tr("Not enough images in this directory"));
                sampleImagePath = "";
                return;
            }

            for (int j = i * SAMPLES_PER_CLASS; j < i * SAMPLES_PER_CLASS + SAMPLES_PER_CLASS; j++) {
                QString path = dirPath + "/" + list.takeFirst();

                if (!sampleImages[j].load(path)) {
                    QMessageBox::information(this,
                                             tr("Error"),
                                             tr("Failed loading %1").arg(path));
                    sampleImagePath = "";
                    return;
                }
                imageLabels[j]->setPixmap(QPixmap::fromImage(sampleImages[j].scaled(50, 50)));
            }
        }

        if (!testImagePath.isEmpty()) {
            ui->recognizeButton->setEnabled(true);
        }

        QList<QImage> allImages;

        for(int i = 0; i < SAMPLE_AMOUNT; i++) {
            allImages.append(sampleImages[i]);
        }

        // Teach network ONLY ONECE to recognize THIS selection!

        cn.setParameters(IMAGE_SIZE, CLASS_AMOUNT);
        cn.teach(allImages);
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
    int result = cn.recognize(testImage);

    QMessageBox::information(this, tr("Result"), tr("This is %1 class").arg(result));
}

void MainWindow::teach()
{
    QList<QImage> allImages;

    for(int i = 0; i < SAMPLE_AMOUNT; i++) {
        allImages.append(sampleImages[i]);
    }

    // Teach network ONLY ONECE to recognize THIS selection!

    cn.setParameters(IMAGE_SIZE, CLASS_AMOUNT);
    cn.teach(allImages);
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
