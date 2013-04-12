#ifndef COMPETITIVENETWORK_H
#define COMPETITIVENETWORK_H

#include <QList>
#include <QVector>
#include <QDebug>
#include <QImage>
#include <cmath>
#include <time.h>

class CompetitiveNetwork
{
private:
    int distributionLayerSize; // the first one
    int outputLayerSize;       // the third one

    QList<QVector<double> >w;
    QVector<double> wins;

public:
    CompetitiveNetwork(int pixelsCount, int classAmount);
    CompetitiveNetwork();

    void setParameters(int pixelsCount, int classAmount);

    void teach(QList<QImage> samples);
    int recognize(const QImage &image);
};

#endif // COMPETITIVENETWORK_H
