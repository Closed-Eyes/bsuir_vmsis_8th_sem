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
    int clasterId;

    int area;
    QPoint massCenter;
    int perimeter;
    double density;
    double mainAxisOrient;
    double elognation;

    QList<QPoint> coords;
};
struct Claster
{
     int clasterId;

     int area;
     double density;
     double elognation;
};

class Analiser
{    
private:
    QList<struct Object> objects;
    int **labelsMap;
    int rows, cols;    
    int clasterAmount;

    QMap<int, cv::Scalar> labelsColors;

    void fillLabels(const cv::Mat &img, int x, int y, int labelNum);
    bool hasNeighbor(int neighbor, int x, int y);
    double Mij(int iPow, int jPow, QList<QPoint> points, QPoint center);
    double calculateRange(double x1, double y1, double z1, double x2, double y2, double z2);


public:
    Analiser();
    ~Analiser();
    int** createLabelsMap(const cv::Mat &img);
    void paintImage(cv::Mat &img);
    QMap<int, cv::Scalar> getLabelColors();
    void attributeDetection();

    void claster(const cv::Mat img_bw, int clasterAmount);
    void paintClasters(cv::Mat &img);
};

#endif // ANALISER_H
