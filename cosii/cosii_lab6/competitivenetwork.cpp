#include "competitivenetwork.h"

CompetitiveNetwork::CompetitiveNetwork(int pixelsCount, int classAmount)
{
    distributionLayerSize = pixelsCount;
    outputLayerSize = classAmount;

    QVector<double> blank(distributionLayerSize);
    for(int i = 0; i < outputLayerSize; i++) {
        w.append(blank);
    }

    wins.fill(1.0, outputLayerSize);

    // 1st step: fill everything with random values

    int low = -1;
    int high = 1;

    //qsrand(time(NULL));
    for (int i = 0; i < outputLayerSize; i++) {
        for(int j = 0; j < distributionLayerSize; j++) {
            double X = (rand()/((double)RAND_MAX + 1)) * (high - low) + low;// * 2 - 1;

            w[i][j] = X;
        }
    }
}

CompetitiveNetwork::CompetitiveNetwork()
{
}

void CompetitiveNetwork::teach(QList<QImage> samples)
{
    QList<QVector<double> > allInputs;

    int pixelsCount = distributionLayerSize;

    // images to "0/1" form. Create arrays with all inputs and all ideal outputs
    for(int index = 0; index < samples.count(); index++) {
        QVector<double> input(distributionLayerSize);
        QImage sample = samples[index];

        for (int i = 0; i < sample.height(); i++) {
            for (int j = 0; j < sample.width(); j++) {
                input[i*sample.height() + j] = qGray(sample.pixel(i, j)) == 255 ? 0 : 1;
            }
        }

        allInputs.append(input);
    }

    bool exit = false;
    int iteration = 0;

    while(exit == false) {
        QList<double> winnersOut;

        iteration++;

        // Find winner

        for(int i = 0; i < allInputs.count(); i++) {
             QList<double> allResults;

            for(int j = 0; j < outputLayerSize; j++) {

                double result = 0.0;

                for(int k = 0; k < pixelsCount; k++) {
                    result += pow(allInputs[i][k] - w[j][k],2);
                }
                result = sqrt(result);

                result *= wins[j];
                allResults.append(result);
            }        

            int winnerIndex = allResults.indexOf(*std::min_element(allResults.begin(), allResults.end()));


            winnersOut.append(allResults[winnerIndex]/wins[winnerIndex]);
            wins[winnerIndex]++;

            // Correct network's knowledge

            double beta = 0.1;

            for(int j = 0; j < pixelsCount; j++) {
                w[winnerIndex][j] = w[winnerIndex][j] + beta * (allInputs[winnerIndex][j] - w[winnerIndex][j]);
            }
            allResults.clear();
        }

        // check if we can leave loop
        double D = 5.5;

        if(*std::max_element(winnersOut.begin(), winnersOut.end()) < D) {
            exit = true;
        }
        qDebug() << *std::max_element(winnersOut.begin(), winnersOut.end());

        winnersOut.clear();
    }
    qDebug() << "Iterations: " << iteration;

}

int CompetitiveNetwork::recognize(const QImage &image)
{
    QVector<double> input(distributionLayerSize);

    int pixelsCount = image.height() * image.width();

    // image to "0/1" form
    for (int i = 0; i < image.height(); i++) {
        for (int j = 0; j < image.width(); j++) {
            input[i*image.height() + j] = qGray(image.pixel(i, j)) == 255 ? 0 : 1; // OR -1 1 ??
        }
    }

    QVector<double> allResults(outputLayerSize);

    // Find winner

    for(int j = 0; j < outputLayerSize; j++) {

        double result = 0.0;

        for(int k = 0; k < pixelsCount; k++) {
            result += pow(input[k] - w[j][k],2);
        }
        result = sqrt(result);

        //result *= wins[j];
        allResults[j] = result;
    }

    int winnerIndex = allResults.indexOf(*std::min_element(allResults.begin(), allResults.end()));

    return winnerIndex;
}

void CompetitiveNetwork::setParameters(int pixelsCount, int classAmount)
{
    wins.clear();
    w.clear();

    distributionLayerSize = pixelsCount;
    outputLayerSize = classAmount;

    QVector<double> blank(distributionLayerSize);
    for(int i = 0; i < outputLayerSize; i++) {
        w.append(blank);
    }

    wins.fill(1.0, outputLayerSize);

    // 1st step: fill up everything with random values

    int low = -1;
    int high = 1;

    //qsrand(time(NULL));
    for (int i = 0; i < outputLayerSize; i++) {
        for(int j = 0; j < distributionLayerSize; j++) {
            w[i][j] = -0.5;
        }
    }
}
