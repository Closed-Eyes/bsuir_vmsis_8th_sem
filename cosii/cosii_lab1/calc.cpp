#include "calc.h"

Calc::Calc()
{
}

double Calc::Mij(int iPow, int jPow, QList<QPoint> points, QPoint center)
{
    double result = 0;

    for(int i = 0; i < points.count(); i++) {
        result += pow(points[i].x() - center.x(), iPow) * pow(points[i].y() - center.y(), jPow);
    }

    return result;
}

double Calc::euklidRange(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1, 2) + pow(z2-z1, 2));
}

double Calc::hammingRange(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return (abs(x1-x2) + abs(y1 - y2) + abs(z1-z2))/3.0;
}

double Calc::tanimotoRange(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return (x1 * x2 + y1 * y2 + z1 * z2)/(pow(x1,2) + pow(x2,2) + pow(y1,2) + pow(y2,2) + pow(z1,2) + pow(z2,2) - (x1 * x2 + y1 * y2 + z1 * z2));
}

double Calc::manhattanRange(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return x1 - y1 - z1 + x2 - y2 - z2;
}
