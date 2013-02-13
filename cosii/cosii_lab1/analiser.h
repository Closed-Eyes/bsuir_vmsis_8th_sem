#ifndef ANALISER_H
#define ANALISER_H

#include <QList>
#include <QMap>
#include <QPoint>
#include <qmath.h>
#include <QDebug>

#include <time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

struct Object
{
    int id;

    int area;
    QPoint massCenter;
    int perimeter;
    double density;
    double mainAxisOrient;
    double elognation;

    QList<QPoint> coords;
};

class Analiser
{    
private:
    QList<struct Object> objects;
    int **labelsMap;
    int rows, cols;

    QMap<int, cv::Scalar> labelsColors;

    void fillLabels(const cv::Mat &img, int x, int y, int labelNum);

    bool hasNeighbor(int neighbor, int x, int y);    

    double Mij(int iPow, int jPow, QList<QPoint> points, QPoint center);



public:
    Analiser();
    ~Analiser();
    int** createLabelsMap(const cv::Mat &img);
    void paintImage(cv::Mat &img);
    QMap<int, cv::Scalar> getLabelColors();
    void attributeDetection();
    void claster(int clasterAmount);
};

#endif // ANALISER_H
