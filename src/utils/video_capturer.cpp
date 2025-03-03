
#include <qobject.h>
#include <qpixmap.h>
#include <utils/video_capturer.h>

video_capturer::video_capturer(QThread* parent)
    : QThread{parent}
    , m_capturer{0} { }

video_capturer::~video_capturer() {
    stop();
    m_capturer.release();
}

void video_capturer::run() {
    if (!m_capturer.isOpened()) {
        /* error msg */
        return;
    }
    while (!m_stop) {
        cv::Mat frame;
        m_capturer >> frame;
        if (frame.empty()) break;

        QImage cap_frame = QImage ( frame.data
                                  , frame.cols
                                  , frame.rows
                                  , frame.step
                                  , QImage::Format_RGB888
                                  ) ;
        cap_frame = cap_frame.rgbSwapped();
        emit frame_captured(cap_frame);
    }
    m_capturer.release();
}

void video_capturer::stop() {
    m_stop = true;
    if (isRunning()) {
        wait(1000);
        if (isRunning()) {
            terminate();
        }
    } 
}
