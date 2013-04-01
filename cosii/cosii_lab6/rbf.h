#ifndef RBF_H
#define RBF_H

#include <time.h>
#include <QImage>
#include <qmath.h>
#include <QVector>
#include <QList>
#include <QDebug>

#include "selection.h"

class RBF
{
private:
    int distributionLayerSize; // the first one
    int hiddenLayerSize;       // the second one
    int outputLayerSize;       // the third one

    QList<QVector<double> >w;

    QList<QVector<double> > t;
    QVector<double> sigma;

public:
    RBF(int pixelsCount, int classAmount);

    ~RBF();

    void teach(QList<Selection> allSelections);
    QVector<double> recognize(const QImage &image);
};

#endif // RBF_H
