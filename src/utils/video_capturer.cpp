
#include "components/actions_wrapper.h"
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <qmutex.h>
#include <qobject.h>
#include <qpixmap.h>
#include <utils/video_capturer.h>
#include <QMutexLocker>


video_capturer::video_capturer(QThread* parent)
    : QThread{parent}
    , m_capturer{0} 
    , m_current_camera_index{0}
    , m_scale_factor{1.0} { }
    // , m_capturer{"rtsp://admin:w12345678@169.254.36.138:554/Streaming/Channels/101"} { }

video_capturer::~video_capturer() {
    stop();
    m_capturer.release();
}

void video_capturer::set_scale_factor(double factor) {
    QMutexLocker locker(&m_scale_mutex);

    if (factor > 0.1 && factor <= 5.0) m_scale_factor = factor;
}

double video_capturer::get_scale_factor() const {
    QMutexLocker lock(&m_scale_mutex);
    return m_scale_factor;
}

bool video_capturer::switch_camera(int camera_index) {
    if (is_running()) {
        stop();
        wait();
    }

    if (open_camera(camera_index)) {
        m_current_camera_index = camera_index;
        m_current_rtsp_url = "";
        m_stop = false;
        start();
        return true;
    }

    emit camera_error("无法打开摄像头 #" + QString::number(camera_index));
    return false;
}

bool video_capturer::switch_rtsp_stream(const QString& rtsp_url) {
   if (is_running()) {
        stop();
        wait();
    } 

    if (open_rtsp_stream(rtsp_url)) {
        m_current_camera_index = -1;
        m_current_rtsp_url = rtsp_url;
        m_stop = false;
        start();
        return true;
    }
    emit camera_error("无法连接到RTSP流: " + rtsp_url);
    return false;
}

bool video_capturer::open_camera(int index) {
    if (m_capturer.isOpened()) {
        m_capturer.release();
    }

    bool success = m_capturer.open(index);
    if (success) {
        m_capturer.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        m_capturer.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    }
    return success;
}

bool video_capturer::open_rtsp_stream(const QString& url) {
    if (m_capturer.isOpened()) { m_capturer.release(); }

    bool success = m_capturer.open(url.toStdString());
    return success;
}


void video_capturer::run() {
    if (!m_capturer.isOpened()) {
        emit camera_error("摄像头未打开");
        return;
    }
    while (!m_stop) {
        cv::Mat frame;
        if (!m_capturer.read(frame)) {
            emit camera_error("无法从摄像头读取帧");
            break;
        }

        if (frame.empty()) continue;
        //
        // m_capturer >> frame;
        // if (frame.empty()) break;
        
        double scale;
        {
            QMutexLocker locker(&m_scale_mutex);
            scale = m_scale_factor;
        }
            
        cv::resize(frame, frame, cv::Size(), scale, scale, cv::INTER_LINEAR);

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
