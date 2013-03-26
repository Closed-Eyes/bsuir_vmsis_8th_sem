#include "hopfieldnetwork.h"

HopfieldNetwork::HopfieldNetwork()
{    
}

HopfieldNetwork::~HopfieldNetwork()
{
    for (int i = 0; i < height; i++) {
        delete [] weightMatrix[i];
    }

    delete [] weightMatrix;
}

void HopfieldNetwork::teach(const QImage &image)
{
    height = image.height();
    width = image.width();

    int pixelNumber = height * width;

    int *values = new int[pixelNumber];

    // image to "-1/1" form
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            values[i*height + j] = qGray(image.pixel(i, j)) == 255 ? 1 : -1;
        }
    }

    // create wight matrix
    weightMatrix = new int*[pixelNumber];

    for (int i = 0; i < pixelNumber; i++) {
        weightMatrix[i] = new int[pixelNumber];
    }

    // fill weight matrix
    for (int i = 0; i < pixelNumber; i++) {
        for (int j = 0; j < pixelNumber; j++) {
            if (i == j) {
                weightMatrix[i][j] = 0;
            }
            else {
                weightMatrix[i][j] = values[i] * values[j];
            }

        }
    }

    delete [] values;
}

QImage HopfieldNetwork::recognize(const QImage &image)
{
    int pixelNumber = height * width;
    int *values = new int[pixelNumber];

    // image to "-1/1" form
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            values[i*height + j] = qGray(image.pixel(i, j)) == 255 ? -1 : 1;
        }
    }

    // result vector
    int *result = new int[pixelNumber];
    for (int i = 0; i < pixelNumber; i++) {
        result[i] = 0;
    }

    bool completed = false;

    while (!completed) {

        for (int i = 0; i < pixelNumber; i++) {
            for (int j = 0; j < pixelNumber; j++) {
                result[i] += weightMatrix[i][j] * values[j];
            }
        }

        // activation function
        for (int i = 0; i < pixelNumber; i++) {
            result[i] = result[i] > 0 ? 1 : -1;
        }

        completed = true;

        for (int i = 0; i < pixelNumber; i++) {
            if (result[i] != values[i])
                completed = false;
        }

        for (int i = 0; i < pixelNumber; i++) {
            values[i] = result[i];
        }
    }

    QRgb black = qRgb(0,0,0);
    QRgb white = qRgb(255,255,255);

    QImage resultImage(width, height, QImage::Format_RGB32);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {

            QRgb currentColor;

            if (result[i*height + j] == -1) {
                currentColor = white;
            }
            else {
                currentColor = black;
            }

            resultImage.setPixel(i, j, currentColor);
        }
    }

    delete [] values;
    delete [] result;

    return resultImage;
}
