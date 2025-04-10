

#ifndef COMMON_TOOLS_HPP
#define COMMON_TOOLS_HPP

#include <QImage>
#include <opencv2/core.hpp>
#include <utils/common.h>

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


inline std::ostream& operator<<(std::ostream& os, rtsp_config& config) {
    os << "|----- RTSP Connnection Info -----|" 
        << "rtsp_id: "   << config.rtsp_id
        << "username: "  << config.username.toStdString()
        << "password: "  << config.password.toStdString()
        << "ip: "        << config.ip.toStdString()
        << "port: "      << config.port.toStdString()
        << "channel: "   << config.channel.toStdString()
        << "subtype: "   << config.subtype.toStdString()
        << "rtsp_url: "  << config.rtsp_url.toStdString();

    return os;
} 



#endif
