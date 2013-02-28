#ifndef ANALISER_H
#define ANALISER_H

#include <QList>
#include <QMap>
#include <QPoint>
#include <qmath.h>
#include <QDebug>

#include "calc.h"
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
     //int perimeter;
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

    int iterations;

    QMap<int, cv::Scalar> labelsColors;

    void fillLabels(const cv::Mat &img, int x, int y, int labelNum);
    int hasNeighbors(int neighbor, int x, int y);
    void attributeDetection();
    void createLabelsMap(const cv::Mat &img);

public:
    Analiser();
    ~Analiser();

    void claster(const cv::Mat img_bw, int clasterAmount);
    void paintClasters(cv::Mat &img);
    void paintObjects(cv::Mat &img);

    int getIterationsNum();
};

#endif // ANALISER_H
