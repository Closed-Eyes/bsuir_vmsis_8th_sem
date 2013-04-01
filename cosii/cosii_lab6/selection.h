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
    void setClassIndex(int classIndex);
    int getClassIndex();

    QVector<double>& getSample(int index);

    int count();

    void setClassCenter(QVector<double> center);

    double calculateSigma(QVector<double> anotherClassCenter);
    QVector<double> getClassCenter();
    double calculateRangeFromSample(int sampleIndex, QVector<double> anotherCenter);
};

#endif // SELECTION_H
