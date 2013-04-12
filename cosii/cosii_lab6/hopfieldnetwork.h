#ifndef HOPFIELDNETWORK_H
#define HOPFIELDNETWORK_H

#include <QImage>
#include <QDebug>

class HopfieldNetwork
{
public:
    HopfieldNetwork();
    ~HopfieldNetwork();

    void teach(const QImage &image);
    QImage recognize(const QImage &image);

private:
    int **weightMatrix;
    int width, height;
};

#endif // HOPFIELDNETWORK_H
