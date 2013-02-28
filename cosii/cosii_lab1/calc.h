#ifndef CALC_H
#define CALC_H

#include <QList>
#include <QPoint>
#include <qmath.h>

class Calc
{
private:
    Calc();

public:
    static double tanimotoRange(double x1, double y1, double z1, double x2, double y2, double z2);
    static double euklidRange(double x1, double y1, double z1, double x2, double y2, double z2);
    static double hammingRange(double x1, double y1, double z1, double x2, double y2, double z2);
    static double manhattanRange(double x1, double y1, double z1, double x2, double y2, double z2);
    static double Mij(int iPow, int jPow, QList<QPoint> points, QPoint center);
};

#endif // CALC_H
