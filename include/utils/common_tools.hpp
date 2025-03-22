

#ifndef COMMON_TOOLS_HPP
#define COMMON_TOOLS_HPP

#include <QImage>
#include <opencv2/core.hpp>

inline cv::Mat qimage2cvmat(const QImage& qiamge) {
    QImage swapped 
        = qiamge.convertToFormat(QImage::Format_RGB888)
            .rgbSwapped();
    cv::Mat mat = cv::Mat( swapped.height(), swapped.width()
                         , CV_8UC3
                         , const_cast<uchar*>(swapped.constBits())
                         , static_cast<size_t>(swapped.bytesPerLine())
                         );

    return mat.clone();
}

#endif
