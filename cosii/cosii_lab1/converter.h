#ifndef CONVERTER_H
#define CONVERTER_H

#include <QImage>
#include <opencv2/core/core.hpp>

class Converter
{
public:
    static QImage Mat2QImage(const cv::Mat &mat);

private:
    Converter();
};

#endif // CONVERTER_H
