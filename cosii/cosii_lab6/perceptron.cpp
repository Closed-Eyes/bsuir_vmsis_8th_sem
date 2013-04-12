#include "perceptron.h"

Perceptron::Perceptron(int pixelsCount, int sampleAmount)
{
    distributionLayerSize = pixelsCount;
    hiddenLayerSize = pixelsCount / 2;
    outputLayerSize = sampleAmount;

    v = new double*[distributionLayerSize];

    for (int i = 0; i < distributionLayerSize; i++) {
        v[i] = new double[hiddenLayerSize];
    }

    w = new double*[hiddenLayerSize];

    for (int i = 0; i < hiddenLayerSize; i++) {
        w[i] = new double[outputLayerSize];
    }

    Q.fill(0.0, hiddenLayerSize);
    T.fill(0.0, outputLayerSize);

    // 1st step: fill everything with random values

    qsrand(time(NULL));
    for (int i = 0; i < distributionLayerSize; i++) {
        for(int j = 0; j < hiddenLayerSize; j++) {
            double X = ((double)qrand()/(double)RAND_MAX);

            if ((qrand() % 100) < 50) { X *= -1; }

            v[i][j] = X;
        }
    }

    for (int i = 0; i < hiddenLayerSize; i++) {
        for(int j = 0; j < outputLayerSize; j++) {
            double X = ((double)qrand()/(double)RAND_MAX);

            if ((qrand() % 100) < 50) { X *= -1; }

            w[i][j] = X;
        }
    }

    for (int i = 0; i < hiddenLayerSize; i++) {
        double X = ((double)qrand()/(double)RAND_MAX);

        if ((qrand() % 100) < 50) { X *= -1; }

        Q[i] = X;
    }

    for (int i = 0; i < outputLayerSize; i++) {
        double X = ((double)qrand()/(double)RAND_MAX);

        if ((qrand() % 100) < 50) { X *= -1; }

        T[i] = X;
    }
}

Perceptron::~Perceptron()
{
    for (int i = 0; i < distributionLayerSize; i++) {
        delete [] v[i];
    }
    delete [] v;

    for (int i = 0; i < hiddenLayerSize; i++) {
        delete [] w[i];
    }
    delete [] w;    
}

void Perceptron::teach(QList<QImage> samples)
{
    QVector<double> input(distributionLayerSize);
    QVector<double> idealY(outputLayerSize);
    QVector<double> d(outputLayerSize);

    QList<QVector<double> > allInputs;
    QList<QVector<double> > allIdealYs;
    QList<QVector<double> > allds;

    // images to "0/1" form. Create arrays with all inputs and all ideal outputs
    for(int index = 0; index < samples.count(); index++) {
        QImage sample = samples[index];

        for (int i = 0; i < sample.height(); i++) {
            for (int j = 0; j < sample.width(); j++) {
                input[i*sample.height() + j] = qGray(sample.pixel(i, j)) == 255 ? 1 : 0;
            }
        }

        allInputs.append(input);

        idealY.fill(0);
        idealY[index] = 1.0;

        allIdealYs.append(idealY);
        allds.append(d);
    }

    bool exit = false;

    double alpha    = 0.1; // learning speed
    double beta     = 0.1; // and this one too
    double accuracy = 0.2;

    QVector<double> g(hiddenLayerSize);
    QVector<double> y(outputLayerSize);

    while(!exit) {

        // 2nd step (for each sample)

        for (int index = 0; index < samples.count(); index++) {
            // 2.1 calculate g[j] and y[k]

            input = allInputs[index];
            idealY = allIdealYs[index];

            for (int j = 0; j < hiddenLayerSize; j++) {
                g[j] = 0.0;
                for (int i = 0; i < distributionLayerSize; i++) {
                    g[j] += v[i][j]*input[i];
                }
                g[j] += Q[j];

                g[j] = 1.0/(1.0 + exp(-g[j]));
            }

            for (int k = 0; k < outputLayerSize; k++) {
                y[k] = 0.0;
                for (int j = 0; j < hiddenLayerSize; j++) {
                    y[k] += w[j][k]*g[j];
                }
                y[k] += T[k];

                y[k] = 1.0/(1.0 + exp(-y[k]));
                //qDebug() << y[k];
            }

            // 2.2 Correct network's knowledge

            for (int k = 0; k < outputLayerSize; k++) {
                d[k] = idealY[k] - y[k];
            }

            for (int j = 0; j < hiddenLayerSize; j++) {
                for (int k = 0; k < outputLayerSize; k++) {
                    w[j][k] = w[j][k] + alpha * y[k] * (1.0 - y[k]) * d[k] * g[j];
                }
            }

            for (int k = 0; k < outputLayerSize; k++) {
                T[k] = T[k] + alpha * y[k] * (1.0 - y[k]) * d[k];
            }

            for (int i = 0; i < distributionLayerSize; i++) {
                for (int j = 0; j < hiddenLayerSize; j++) {

                    double ej = 0;

                    for (int k = 0; k < outputLayerSize; k++) {
                        ej += d[k] * y[k] * (1.0 - y[k]) * w[j][k];
                    }

                    v[i][j] = v[i][j] + beta * g[j] * (1.0 - g[j]) * ej * input[i];
                }
            }

            for (int j = 0; j < hiddenLayerSize; j++) {
                double ej = 0;

                for (int k = 0; k < outputLayerSize; k++) {
                    ej += d[k] * y[k] * (1.0 - y[k]) * w[j][k];
                }

                Q[j] = Q[j] + beta * g[j] * (1.0 - g[j]) * ej;
            }

            allds[index] = d;
        }

        // 3. Check if can leave loop

        double max = 0;// = d[0];

        for (int i = 0; i < samples.count(); i++) {
            for (int j = 0; j < outputLayerSize; j++) {
                if (max < allds[i][j]) {
                    max = allds[i][j];
                }
            }
        }

        if(max < accuracy) {
            // leave loop;
            exit = true;
        }
        qDebug() << "Max: " << max << " D: " << accuracy;
    }
}

QVector<double> Perceptron::recognize(const QImage &image)
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
        for (int i = 0; i < distributionLayerSize; i++) {
            g[j] += v[i][j]*input[i];
        }
        g[j] += Q[j];

        g[j] = 1.0/(1.0 + exp(-g[j]));
    }

    for (int k = 0; k < outputLayerSize; k++) {
        y[k] = 0.0;
        for (int j = 0; j < hiddenLayerSize; j++) {
            y[k] += w[j][k]*g[j];
        }
        y[k] += T[k];

        y[k] = 1.0/(1.0 + exp(-y[k]));
        //qDebug() << y[k];
    }

    QVector<double> result(outputLayerSize);

    for (int i = 0; i < outputLayerSize; i++) {
        result[i] = y[i];
    }

    return result;
}
