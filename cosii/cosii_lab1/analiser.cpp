#include "analiser.h"

Analiser::Analiser()
{
}

Analiser::~Analiser()
{
    if(labelsMap != NULL) {
        for(int i = 0; i < rows; i++) {
            delete [] labelsMap[i];
        }
        delete [] labelsMap;
    }
}

int **Analiser::createLabelsMap(const cv::Mat &img)
{
    int labelNum = 1;
    int x, y;

    labelsMap = new int*[img.rows];
    for(int i = 0; i < img.rows; i++) {
        labelsMap[i] = new int[img.cols];
    }

    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
            labelsMap[i][j] = 0;
        }
    }

    rows = img.rows;
    cols = img.cols;

    for(y = 0; y < img.cols; y++) {
        for(x = 0; x < img.rows; x++) {
            fillLabels(img, x, y, labelNum++);
        }
    }

    cv::RNG rng(0xFFFFFFFF);

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            if(!labelsColors.contains(labelsMap[i][j]) && labelsMap[i][j] != 0) {

                int icolor = (unsigned) rng;
                labelsColors.insert(labelsMap[i][j], cv::Scalar( icolor&255, (icolor>>8)&255, (icolor>>16)&255 ));

                struct Object obj;
                obj.id = labelsMap[i][j];
                obj.area = 0;
                obj.density = 0;
                obj.elognation = 0;
                obj.mainAxisOrient = 0;
                obj.massCenter.setX(0);
                obj.massCenter.setY(0);
                obj.perimeter = 0;

                objects.append(obj);
            }
        }
    }

    qDebug() << "Found " << objects.count() << " objects";

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            for(QList<struct Object>::iterator it = objects.begin(); it != objects.end(); it++) {
                if(it->id == labelsMap[i][j]) {
                    QPoint point;
                    point.setX(i);
                    point.setY(j);
                    it->coords.append(point);
                }
            }
        }
    }

    attributeDetection();
    //claster();

    /*
    for(int i = 0; i < objects.count(); i++) {
        qDebug() << i << " Elognation " << objects[i].elognation;
        qDebug() << i << " Area" << objects[i].area;
    }
    */

    return labelsMap;
}

void Analiser::attributeDetection()
{
    //calculate area
    for(int i = 0; i < objects.count(); i++) {
        objects[i].area = objects[i].coords.count();
    }

    //calculate perimeter
    for(int i = 0; i < objects.count(); i++) {
        for(int j = 0; j < objects[i].coords.count(); j++) {
            if(hasNeighbor(0, objects[i].coords[j].x(), objects[i].coords[j].y()) == true) {
                objects[i].perimeter++;
            }
        }
    }

    //mass center
    for(int i = 0; i < objects.count(); i++) {
        for(int j = 0; j < objects[i].coords.count(); j++) {
            objects[i].massCenter.setX(objects[i].massCenter.x() + objects[i].coords[j].x());
            objects[i].massCenter.setY(objects[i].massCenter.y() + objects[i].coords[j].y());
        }
        objects[i].massCenter.setX(objects[i].massCenter.x()/objects[i].area);
        objects[i].massCenter.setY(objects[i].massCenter.y()/objects[i].area);
    }

    //density
    for(int i = 0; i < objects.count(); i++) {
        objects[i].density = (double)(objects[i].perimeter*objects[i].perimeter)/(double)objects[i].area;
    }

    //elognation
    for(int i = 0; i < objects.count(); i++) {
        double m20 = Mij(2,0, objects[i].coords, objects[i].massCenter);
        double m02 = Mij(0,2, objects[i].coords, objects[i].massCenter);
        double m11 = Mij(1,1, objects[i].coords, objects[i].massCenter);
        objects[i].elognation = (m20 + m02 + sqrt(pow(m20 - m02, 2) + 4.0 * m11 * m11))
                               /(m20 + m02 - sqrt(pow(m20 - m02, 2) + 4.0 * m11 * m11));
    }

    //main axis orientation
    for(int i = 0; i < objects.count(); i++) {
        double m20 = Mij(2,0, objects[i].coords, objects[i].massCenter);
        double m02 = Mij(0,2, objects[i].coords, objects[i].massCenter);
        double m11 = Mij(1,1, objects[i].coords, objects[i].massCenter);
        objects[i].mainAxisOrient = 0.5 * atan((2.0 * m11)/(m20 - m02));
    }
}

double Analiser::Mij(int iPow, int jPow, QList<QPoint> points, QPoint center)
{
    double result;

    for(int i = 0; i < points.count(); i++) {
        result += pow(points[i].x() - center.x(), iPow) * pow(points[i].y() - center.y(), jPow);
    }

    return result;
}

bool Analiser::hasNeighbor(int neighbor, int x, int y)
{
    for(int i = x - 1; i < x + 2; i++) {
        for(int j = y - 1; j < y + 2; j++) {
            if(i >= 0 && j >= 0 && i < rows && j < cols) {
                if(labelsMap[i][j] == neighbor) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Analiser::claster(int clasterAmount)
{
    QList<struct Object> clasters;

    qsrand(time(NULL));
    for(int i = 0; i < clasterAmount; i++) {
        int rnd = qrand()%objects.count();
        //qDebug() << "Random: " << rnd;
        clasters.append(objects[rnd]);
    }


}

void Analiser::fillLabels(const cv::Mat &img, int x, int y, int labelNum)
{
    if(labelsMap[x][y] == 0) {
        if (img.channels() == 1) //because image is grayscale
        {
            if (img.at<uchar>(x,y) == 255) {

                labelsMap[x][y] = labelNum;

                if( x > 0 )
                    fillLabels(img, x-1, y, labelNum);
                if(x < img.rows - 1)
                    fillLabels(img, x+1, y, labelNum);
                if(y > 0)
                    fillLabels(img, x, y-1, labelNum);
                if(y < img.cols - 1)
                    fillLabels(img, x, y+1, labelNum);
            }
        }

    }
}

void Analiser::paintImage(cv::Mat &img)
{
    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
            if(labelsMap[i][j] != 0) {
                img.data[img.channels()*(img.cols*i + j) + 0] = labelsColors.value(labelsMap[i][j])[0];
                img.data[img.channels()*(img.cols*i + j) + 1] = labelsColors.value(labelsMap[i][j])[1];
                img.data[img.channels()*(img.cols*i + j) + 2] = labelsColors.value(labelsMap[i][j])[2];
            }
        }
    }
}

QMap<int, cv::Scalar> Analiser::getLabelColors()
{
    return labelsColors;
}
