#include "rbf.h"

RBF::RBF(int pixelsCount, int classAmount)
{
    distributionLayerSize = pixelsCount;
    hiddenLayerSize = classAmount;
    outputLayerSize = classAmount;

    QVector<double> blank(outputLayerSize);
    for(int i = 0; i < hiddenLayerSize; i++) {
        w.append(blank);
    }

    // 1st step: fill everything with random values

    for (int i = 0; i < hiddenLayerSize; i++) {
        for(int j = 0; j < outputLayerSize; j++) {
            double X = ((double)qrand()/(double)RAND_MAX);

            if ((qrand() % 100) < 50) { X *= -1; }

            w[i][j] = X;
        }
    }
}

RBF::~RBF()
{
}

void RBF::teach(QList<Selection> allSelections)
{
    int classAmount = allSelections.count();
    // Prepare stage
    // Caclulate each class center

    //t.fill(0.0, outputLayerSize);
    sigma.fill(0.0, outputLayerSize);

    int samplesPerClass = allSelections[0].count();
    int pixelsCount = distributionLayerSize;

    for (int i = 0; i < allSelections.count(); i++) {
        QVector<double> center;

        center.fill(0.0, pixelsCount);

        for(int j = 0; j < allSelections[i].count(); j++) {
            for(int k = 0; k < pixelsCount; k++) {
                center[k] += allSelections[i].getSample(j)[k];
            }
        }

        for(int j = 0; j < pixelsCount; j++) {
            center[j] /= (double)allSelections[i].count();
            //center[j] = center[j] > 0.5 ? 1 : 0;
        }

        //add center to selection
        allSelections[i].setClassCenter(center);
        t.append(center);
    }

    // calculate sigmas
    for(int i = 0; i < allSelections.count(); i++) {
        QVector<double> allSigmas;
        for(int j = 0; j < allSelections.count(); j++) {
            if(i != j) {                
                double result = 0;

                for(int k = 0; k < pixelsCount; k++) {
                    result += pow(allSelections[i].getClassCenter()[k] - allSelections[j].getClassCenter()[k], 2);
                }

                result = sqrt(result);
                allSigmas.append(result);
            }
        }

        // then get the minimal sigma/2

        double resultSigma;
        resultSigma = *std::min_element(allSigmas.begin(), allSigmas.end());
        resultSigma /= 2.0;

        sigma[i] = resultSigma;
    }


    // Step 2. For each pair xr yr
    QList<QVector<double> > allDks;

    bool exit = false;

    while(exit == false) {

        for(int sampleIndex = 0; sampleIndex < samplesPerClass; sampleIndex++) {
            for(int classIndex = 0; classIndex < classAmount; classIndex++) {

                QVector<double> g(allSelections.count());
                QVector<double> y(allSelections.count());
                QVector<double> d(outputLayerSize);

                QVector<double> currentSample = allSelections[classIndex].getSample(sampleIndex);

                for (int j = 0; j < hiddenLayerSize; j++) {
                    g[j] = 0.0;

                    double result = 0.0;
                    for(int i = 0; i < currentSample.count(); i++) {
                        result += pow(currentSample[i] - t[j][i], 2);
                    }
                    //result = result > 0 ? result : result * -1;

                    g[j] = exp(-result/pow(sigma[j],2));
                }

                // calculate network's output
                for(int k = 0; k < outputLayerSize; k++) {
                    double result = 0;
                    for(int j = 0; j < hiddenLayerSize; j++) {
                        result += w[j][k] * g[j];
                    }
                    y[k] = result;

                    y[k] = 1.0/(1.0 + exp(-y[k]));
                }

                //calculate deviation

                QVector<double> idealY(outputLayerSize);
                idealY.fill(0.0);
                idealY[allSelections[classIndex].getClassIndex()] = 1.0;

                for (int k = 0; k < outputLayerSize; k++) {
                    d[k] = idealY[k] - y[k];
                }

                allDks.append(d);

                double alpha = 0.01;

                // configure network's knowledge
                for(int j = 0; j < hiddenLayerSize; j++) {
                    for(int k = 0; k < outputLayerSize; k++) {
                        w[j][k] = w[j][k] + alpha * d[k] * g[j];
                        //qDebug() << "w[" << j << "][" << k << "]" << " " << w[j][k];
                    }
                }
            }
        }

        // get max dk
        double maxDk = 0.0;

        foreach(QVector<double> d, allDks) {
            foreach(double dk, d) {
                if(maxDk < dk) {
                    maxDk = dk;
                }
            }
        }

        /*
        for(int i = 0; i < allDks.count(); i++) {
            for(int j = 0; j < allDks[i].count(); j++) {
                if(maxDk < allDks[i][j]) {
                    maxDk = allDks[i][j];
                }
            }
        }
        */

        if(maxDk < 0.2) {
            exit = true;
        }

        qDebug() << maxDk;

        allDks.clear();
    }
}

QVector<double> RBF::recognize(const QImage &image)
{
    QVector<double> input(distributionLayerSize);

    // image to "0/1" form
    for (int i = 0; i < image.height(); i++) {
        for (int j = 0; j < image.width(); j++) {
            input[i*image.height() + j] = qGray(image.pixel(i, j)) == 255 ? 1 : 0; // OR -1 1 ??
        }
    }

    QVector<double> g(hiddenLayerSize);
    QVector<double> y(outputLayerSize);

    // Calculate g[j] and y[k]

    for (int j = 0; j < hiddenLayerSize; j++) {
        g[j] = 0.0;

        double result = 0.0;
        for(int i = 0; i < input.count(); i++) {
            result += pow(input[i] - t[j][i], 2);
        }
        //result = result > 0 ? result : result * -1;

        g[j] = exp(-result/pow(sigma[j],2));
    }

    for (int k = 0; k < outputLayerSize; k++) {
        y[k] = 0.0;
        for (int j = 0; j < hiddenLayerSize; j++) {
            y[k] += w[j][k]*g[j];
        }

        y[k] = 1.0/(1.0 + exp(-y[k]));
        //qDebug() << y[k];
    }

    return y;
}
