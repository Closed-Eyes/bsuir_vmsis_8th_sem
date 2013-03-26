#ifndef SELECTION_H
#define SELECTION_H

#include <QList>
#include <QVector>
#include <QImage>
#include <qmath.h>

class Selection
{
private:
    int classIndex;    
    QList<QVector<double> > samples;
    QVector<double> classCenter;

public:
    Selection();

    void addSample(QImage &image);
    QVector<double>& getSample(int index);

    void setClassCenter(QVector<double> center);
    QVector<double> getClassCenter();

    void setClassIndex(int classIndex);
    int getClassIndex();

    int count();

    double calculateSigma(QVector<double> anotherClassCenter);
};

#endif // SELECTION_H
