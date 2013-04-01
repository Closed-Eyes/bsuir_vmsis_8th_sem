#include "selection.h"

Selection::Selection()
{
}

void Selection::addSample(QImage &sample)
{
    QVector<double> input(sample.height() * sample.width());
    for (int i = 0; i < sample.height(); i++) {
        for (int j = 0; j < sample.width(); j++) {
            input[i*sample.height() + j] = qGray(sample.pixel(i, j)) == 255 ? 1 : 0;
        }
    }
    samples.append(input);
}

void Selection::setClassIndex(int classIndex)
{
    this->classIndex = classIndex;
}

int Selection::getClassIndex()
{
    return classIndex;
}

QVector<double>& Selection::getSample(int index)
{
    if(index < 0) {

    }

    if(index > samples.count()) {

    }

    return samples[index];
}

int Selection::count()
{
    return samples.count();
}

void Selection::setClassCenter(QVector<double> center)
{
    classCenter = center;
}

QVector<double> Selection::getClassCenter()
{
    return classCenter;
}

double Selection::calculateSigma(QVector<double> anotherClassCenter)
{
    double result = 0;

    for(int i = 0; i < classCenter.count(); i++) {
        result += pow(classCenter[i] - anotherClassCenter[i], 2);
    }

    return sqrt(result);
}

double Selection::calculateRangeFromSample(int sampleIndex, QVector<double> anotherCenter)
{
    double result = 0;

    for(int i = 0; i < anotherCenter.count(); i++) {
        result += pow(samples[sampleIndex][i] - anotherCenter[i], 2);
    }
    return result > 0 ? result : result * -1.0;
}

