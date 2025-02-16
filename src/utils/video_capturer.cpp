
#include <qobject.h>
#include <qpixmap.h>
#include <utils/video_capturer.h>

video_capturer::video_capturer(QObject* parent)
    : QObject{parent}
    , m_capturer{0}
    {
    if (m_capturer.isOpened()) {
        m_timer = new QTimer();
        connect (m_timer
                , &QTimer::timeout
                , this
                , &video_capturer::capture_frame
                ) ;
        m_timer->start(33);
    }
}

video_capturer::~video_capturer() {
    if (m_capturer.isOpened()) {
        m_capturer.release();
    }
}

void video_capturer::start() { m_timer->start(); }
void video_capturer::stop()  { m_timer->stop();  } 

void video_capturer::capture_frame() {
    cv::Mat frame;
    if (m_capturer.read(frame)) {
        QImage image( frame.data
                    , frame.cols
                    , frame.rows
                    , frame.step
                    , QImage::Format_BGR888
                    ) ;
        emit frame_captured(QPixmap::fromImage(image));
    }
}