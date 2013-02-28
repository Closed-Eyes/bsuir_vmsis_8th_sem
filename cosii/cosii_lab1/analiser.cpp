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

void Analiser::createLabelsMap(const cv::Mat &img)
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

    // for debug
    for(int i = 0; i< objects.count(); i++) {
        qDebug() << "Area:        " << objects[i].area;
        qDebug() << "Perimeter:   " << objects[i].perimeter;
        qDebug() << "Density:     " << objects[i].density;
        qDebug() << "Elognation:  " << objects[i].elognation;
        qDebug() << "Main axis:   " << objects[i].mainAxisOrient;
        qDebug() << "Mass center: " << objects[i].massCenter.y() << " " << objects[i].massCenter.x();
        qDebug() << "---------------------";
    }
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
            objects[i].perimeter += hasNeighbors(0,
                                                 objects[i].coords[j].x(),
                                                 objects[i].coords[j].y());
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
        objects[i].density = pow(objects[i].perimeter, 2)/(double)objects[i].area;
    }

    //elognation
    for(int i = 0; i < objects.count(); i++) {
        double m20 = Calc::Mij(2,0, objects[i].coords, objects[i].massCenter);
        double m02 = Calc::Mij(0,2, objects[i].coords, objects[i].massCenter);
        double m11 = Calc::Mij(1,1, objects[i].coords, objects[i].massCenter);
        objects[i].elognation = (m20 + m02 + sqrt(pow(m20 - m02, 2) + 4.0 * m11 * m11))/
                                (m20 + m02 - sqrt(pow(m20 - m02, 2) + 4.0 * m11 * m11));
    }

    //main axis orientation
    for(int i = 0; i < objects.count(); i++) {
        double m20 = Calc::Mij(2,0, objects[i].coords, objects[i].massCenter);
        double m02 = Calc::Mij(0,2, objects[i].coords, objects[i].massCenter);
        double m11 = Calc::Mij(1,1, objects[i].coords, objects[i].massCenter);
        objects[i].mainAxisOrient = 0.5 * atan((2.0 * m11)/(m20 - m02));
    }
}

int Analiser::hasNeighbors(int neighbor, int x, int y)
{
    int neighborhood = 0;

    if(x - 1 >= 0) {
        if(labelsMap[x-1][y] == neighbor) {
            neighborhood ++;
        }
    }
    if(x + 1 < rows ) {
        if(labelsMap[x+1][y] == neighbor) {
            neighborhood++;
        }
    }
    if(y - 1 >= 0) {
        if(labelsMap[x][y - 1] == neighbor) {
            neighborhood ++;
        }
    }
    if(y + 1 < cols ) {
        if(labelsMap[x][y + 1] == neighbor) {
            neighborhood++;
        }
    }

    return neighborhood;
}

void Analiser::claster(const cv::Mat img_bw, int clasterAmount)
{
    this->clasterAmount = clasterAmount;
    createLabelsMap(img_bw);

    QList<struct Claster> clasters;
    QList<QList<struct Claster> > clasterSets;
    QList<int> clasterSummaryDist;

    QList<struct Object> clasterPretends;
    clasterPretends = objects;

    double *ranges = new double[clasterAmount];
    int lastUsedID = 0;

    // Try all possibilities

    for(int set = 0; set < objects.count(); set++) {
        clasterPretends = objects;
        lastUsedID = 0;

        struct Claster newClaster;
        newClaster.clasterId = lastUsedID;
        newClaster.area = clasterPretends[set].area;
        newClaster.density = clasterPretends[set].density;
        newClaster.elognation = clasterPretends[set].elognation;
        clasters.append(newClaster);
        lastUsedID++;
        clasterPretends.removeAt(set);

        // finder the most different claster
        while(clasters.count() < clasterAmount) {

            // ----INIT----
            double **rangesFromClasters = new double*[clasters.count()];
            for(int i = 0; i < clasters.count(); i++) {
                rangesFromClasters[i] = new double[clasterPretends.count()];
            }
            // ------------

            // calculate range from each claster to each object
            for (int i = 0; i < clasters.count(); i++) {
                for(int j = 0; j < clasterPretends.count(); j++) {
                    rangesFromClasters[i][j] = Calc::euklidRange(clasterPretends[j].area * 6.0,
                                                                 clasterPretends[j].density * 4.0,
                                                                 clasterPretends[j].elognation * 3.0,
                                                                 clasters[i].area * 6.0,
                                                                 clasters[i].density * 4.0,
                                                                 clasters[i].elognation * 3.0);
                    /*
                    rangesFromClasters[i][j] = Calc::tanimotoRange(clasterPretends[j].area*6.0,
                                                                   clasterPretends[j].density*4.0,
                                                                   clasterPretends[j].elognation*3.0,
                                                                   clasters[i].area*6.0,
                                                                   clasters[i].density*4.0,
                                                                   clasters[i].elognation*3.0);
                    */
                }
            }

            // find the farest one from each claster

            double *candidateRanges = new double[clasterPretends.count()];

            for(int i = 0; i < clasterPretends.count(); i++) {
                candidateRanges[i] = 0;
                for(int j = 0; j < clasters.count(); j++) {
                    candidateRanges[i] += pow(rangesFromClasters[j][i], 2);
                }
                candidateRanges[i] = sqrt(candidateRanges[i]);
            }

            int distance = 0;
            int candidateNum = 0;

            for(int i = 0; i < clasterPretends.count(); i++) {
                if(candidateRanges[i] > distance) {
                    distance = candidateRanges[i];
                    candidateNum = i;
                }
            }

            newClaster.clasterId = lastUsedID;
            newClaster.area = clasterPretends[candidateNum].area;
            newClaster.density = clasterPretends[candidateNum].density;
            newClaster.elognation = clasterPretends[candidateNum].elognation;

            clasterPretends.removeAt(candidateNum);

            clasters.append(newClaster);

            // ----CLEAR----
            for(int i = 0; i < clasters.count() - 1; i++) {
                delete [] rangesFromClasters[i];
            }
            delete [] rangesFromClasters;
            delete [] candidateRanges;
            // -------------
        }

        int distance = 0;

        for(int i = 0; i < clasters.count(); i++) {
            for(int j = 0; j < clasters.count(); j++) {
                if(i != j) {
                    distance += Calc::euklidRange(clasters[j].area * 6.0,
                                                  clasters[j].density * 4.0,
                                                  clasters[j].elognation * 3.0,
                                                  clasters[i].area * 6.0,
                                                  clasters[i].density * 4.0,
                                                  clasters[i].elognation * 3.0);
                }
            }
        }

        clasterSets.append(clasters);

        clasterSummaryDist.append(distance);
        clasters.clear();
    }

    int optimalSet = 0;
    int maxSumRange = 0;

    for(int i = 0; i < clasterSummaryDist.count(); i++) {
        if(clasterSummaryDist[i] > maxSumRange) {
            optimalSet = i;
            maxSumRange = clasterSummaryDist[i];
        }
    }

    clasters = clasterSets[optimalSet];

    /*
    lastUsedID = 0;
    clasterPretends = objects;
    // Get glaster with optimal set

    struct Claster newClaster;
    newClaster.clasterId = lastUsedID;
    newClaster.area = clasterPretends[optimalSet].area;
    newClaster.density = clasterPretends[optimalSet].density;
    newClaster.elognation = clasterPretends[optimalSet].elognation;
    clasters.append(newClaster);
    lastUsedID++;
    clasterPretends.removeAt(optimalSet);

    // finder the most different claster
    while(clasters.count() < clasterAmount) {

        // ----INIT----
        double **rangesFromClasters = new double*[clasters.count()];
        for(int i = 0; i < clasters.count(); i++) {
            rangesFromClasters[i] = new double[clasterPretends.count()];
        }
        // ------------

        // calculate range from each claster to each object
        for (int i = 0; i < clasters.count(); i++) {
            for(int j = 0; j < clasterPretends.count(); j++) {
                rangesFromClasters[i][j] = Calc::euklidRange(clasterPretends[j].area * 6.0,
                                                             clasterPretends[j].density * 4.0,
                                                             clasterPretends[j].elognation * 3.0,
                                                             clasters[i].area * 6.0,
                                                             clasters[i].density * 4.0,
                                                             clasters[i].elognation * 3.0);
//                rangesFromClasters[i][j] = Calc::tanimotoRange(clasterPretends[j].area*6.0, clasterPretends[j].density*4.0, clasterPretends[j].elognation*3.0,
//                                                               clasters[i].area*6.0, clasters[i].density*4.0, clasters[i].elognation*3.0);
            }
        }

        // find the farest one from each claster

        double *candidateRanges = new double[clasterPretends.count()];

        for(int i = 0; i < clasterPretends.count(); i++) {
            candidateRanges[i] = 0;
            for(int j = 0; j < clasters.count(); j++) {
                candidateRanges[i] += pow(rangesFromClasters[j][i], 2);
            }
            candidateRanges[i] = sqrt(candidateRanges[i]);
        }

        int distance = 0;
        int candidateNum = 0;

        for(int i = 0; i < clasterPretends.count(); i++) {
            if(candidateRanges[i] > distance) {
                distance = candidateRanges[i];
                candidateNum = i;
            }
        }

        newClaster.clasterId = lastUsedID;
        newClaster.area = clasterPretends[candidateNum].area;
        newClaster.density = clasterPretends[candidateNum].density;
        newClaster.elognation = clasterPretends[candidateNum].elognation;

        clasterPretends.removeAt(candidateNum);

        clasters.append(newClaster);

        // ----CLEAR----
        for(int i = 0; i < clasters.count() - 1; i++) {
            delete [] rangesFromClasters[i];
        }
        delete [] rangesFromClasters;
        delete [] candidateRanges;
        // -------------
    }
    */
    /*
    int minArea = objects[0].area;
    int maxArea = 0;
    double minDens = objects[0].density;
    double maxDens = 0;
    double minElog = objects[0].elognation;
    double maxElog = 0;


    for(int i = 0; i < objects.count(); i++) {
        if(objects[i].area > maxArea)
            maxArea = objects[i].area;
        if(objects[i].area < minArea)
            minArea = objects[i].area;
        if(objects[i].density > maxDens)
            maxDens = objects[i].density;
        if(objects[i].density < minDens)
            minDens = objects[i].density;
        if(objects[i].elognation > maxElog)
            maxElog = objects[i].elognation;
        if(objects[i].elognation < minElog)
            minElog = objects[i].elognation;
    }

    qDebug() << "Min area: " << minArea;
    qDebug() << "Max area: " << maxArea;
    qDebug() << "Min dens: " << minDens;
    qDebug() << "Max dens: " << maxDens;
    qDebug() << "Min elog : " << minElog;
    qDebug() << "Max elog : " << maxElog;
    qDebug() << "-------";

    double averageArea = (maxArea - minArea)/(double)clasterAmount;
    double averageDens = (maxDens - minDens)/(double)clasterAmount;
    double averageElog = (maxElog - minElog)/(double)clasterAmount;

    for(int i = 0; i < clasterAmount; i++) {
        clasters[i].area       = minArea + averageArea/2.0;
        clasters[i].density    = minDens + averageDens/2.0;
        clasters[i].elognation = minElog + averageElog/2.0;

        minArea += averageArea;
        minDens += averageDens;
        minElog += averageElog;
    }
    */

    iterations = 0;
    bool continueCalc = true;

    while(continueCalc) {
        continueCalc = false;
        iterations++;

        // calculate range
        for(int i = 0; i < objects.count(); i++) {

            // calculate ranges from objects to clasters
            for(int j = 0; j < clasterAmount; j++) {
                ranges[j] = Calc::euklidRange(objects[i].area * 6.0,
                                              objects[i].density * 4.0,
                                              objects[i].elognation * 3.0,
                                              clasters[j].area * 6.0,
                                              clasters[j].density * 4.0,
                                              clasters[j].elognation * 3.0);                                              
            }

            // find minimal range
            double maxRange = ranges[0];
            double clasterId = 0;
            for(int j = 1; j < clasterAmount; j++) {
                if(maxRange > ranges[j]) {
                    maxRange = ranges[j];
                    clasterId = j;
                }
            }
            objects[i].clasterId = clasterId;
        }

        // recalculate each claster's center
        for(int i = 0; i < clasterAmount; i++) {
            QList<int> areas;
            QList<double> densities;
            QList<double> eloginations;

            for(int j = 0; j < objects.count(); j++) {
                if(objects[j].clasterId == i) {
                    areas.append(objects[j].area);
                    densities.append(objects[j].density);
                    eloginations.append(objects[j].elognation);
                }
            }

            int size = areas.count();

            if(size == 0) {
                // bad claster
                continue;
            }
            qSort(areas);
            qSort(densities);
            qSort(eloginations);

            int newArea;
            double newDensity;
            double newElognation;

            //New values are medians
            if(size %2 == 0) {
                size -= 2;
                newArea = (areas[size/2] + areas[(size+2)/2]) / 2;
                newDensity = (densities[size/2] + densities[(size+2)/2]) / 2.0;
                newElognation = (eloginations[size/2] + eloginations[(size+2)/2]) / 2.0;
            }
            else {
                newArea = areas[size/2];
                newDensity = densities[size/2];
                newElognation = eloginations[size/2];
            }

            if(clasters[i].area != newArea) {
                continueCalc = true;
                clasters[i].area = newArea;
            }
            if(clasters[i].density != newDensity) {
                continueCalc = true;
                clasters[i].density = newDensity;
            }
            if(clasters[i].elognation != newElognation) {
                continueCalc = true;
                clasters[i].elognation = newElognation;
            }

            if(iterations > 50)
                continueCalc = false;
        }
    }

    delete [] ranges;
}

void Analiser::fillLabels(const cv::Mat &img, int x, int y, int labelNum)
{
    if(labelsMap[x][y] == 0) {
        if (img.channels() == 1) //because image is grayscale
        {
            if (img.at<uchar>(x,y) == 255) {

                labelsMap[x][y] = labelNum;

                if(x > 0)
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

void Analiser::paintObjects(cv::Mat &img)
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

void Analiser::paintClasters(cv::Mat &img)
{
    QList<cv::Scalar> colorsStack;
    /*
    colorsStack.append(cv::Scalar(102, 51, 255));
    colorsStack.append(cv::Scalar(204, 51, 255));
    colorsStack.append(cv::Scalar(255, 51, 102));
    colorsStack.append(cv::Scalar(204, 255, 51));
    colorsStack.append(cv::Scalar(138, 184, 0));
    colorsStack.append(cv::Scalar(255, 255, 255));
    colorsStack.append(cv::Scalar(55, 232, 24));
    colorsStack.append(cv::Scalar(12,234,255));
    colorsStack.append(cv::Scalar(522,13,25));
    */

    cv::RNG rng(0xFFFFFFFF);

    for(int i = 0; i < clasterAmount; i++) {
        int icolor = (unsigned) rng;
       colorsStack.append(cv::Scalar( icolor&255, (icolor>>8)&255, (icolor>>16)&255 ));
    }

    for(int i = 0; i < objects.count(); i++) {
        for(int j = 0; j < objects[i].coords.count(); j++) {
            int x = objects[i].coords[j].x();
            int y = objects[i].coords[j].y();
            img.data[img.channels()*(img.cols*x + y) + 0] = colorsStack[objects[i].clasterId][0];
            img.data[img.channels()*(img.cols*x + y) + 1] = colorsStack[objects[i].clasterId][1];
            img.data[img.channels()*(img.cols*x + y) + 2] = colorsStack[objects[i].clasterId][2];
        }
    }
}

int Analiser::getIterationsNum()
{
    return iterations;
}
