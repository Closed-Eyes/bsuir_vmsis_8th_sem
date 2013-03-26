#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include <time.h>
#include <QImage>
#include <qmath.h>
#include <QVector>
#include <QList>

#include <QDebug>

class Perceptron
{
private:
    int distributionLayerSize; // the first one
    int hiddenLayerSize; // the second one
    int outputLayerSize; // the third one

    double **v;
    double **w;

    //QMatrix asd;

    QVector<double> Q;
    QVector<double> T;

public:
    Perceptron(int pixelsCount, int sampleAmount);
    ~Perceptron();

    void teach(QList<QImage> samples);
    QVector<double> recognize(const QImage &image);
};

#endif // PERCEPTRON_H
