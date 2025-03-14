
#include "components/actions_wrapper.h"
#include <format>
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <qmutex.h>
#include <qobject.h>
#include <qpixmap.h>
#include <utils/video_capturer.h>
#include <QMutexLocker>


video_capturer::video_capturer(int cam_id, QThread* parent)
    : QThread{parent}
    //, m_capturer{0}
    , m_cam_id{cam_id}
    , m_current_camera_index{0}
    , m_scale_factor{0.5f} { }
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
        m_rtsp_url = rtsp_url;
        start();
        return true;
    }
    emit camera_error("无法连接到RTSP流: " + rtsp_url);
    return false;
}

QString video_capturer::get_rtsp_url() const { return m_rtsp_url; }

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

    /* if gpu
    m_capturer = cv::VideoCapture();
    std::string rtsp_url = url.toStdString();

    std::string pipeline = "ffmpeg -hwaccel cuda -rtsp_transport tcp -i " + rtsp_url;

    bool success = m_capturer.open(rtsp_url, cv::CAP_FFMPEG);
    
    if (success) {
        m_capturer.set(cv::CAP_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_ANY);
    }
    */
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
        #ifdef ENABLE_GPU
        if (!m_use_cuda) {
            m_gpu_frame.upload(frame);
            cv::cuda::GpuMat resized_gpu_frame;
            cv::cuda::resize(m_gpu_frame, resized_gpu_frame, cv::Size(), scale, scale, cv::INTER_LINEAR);
            cv::Mat resized_frame;
            resized_gpu_frame.download(resized_frame);

            QImage cap_frame = QImage(resized_frame.data, resized_frame.cols, resized_frame.rows, resized_frame.step, QImage::Format_RGB888);

            cap_frame = cap_frame.rgbSwapped();
            emit frame_captured(cap_frame);
        } else {
        #endif

        cv::resize(frame, frame, cv::Size(), scale, scale, cv::INTER_LINEAR);
    
        std::string cam_id_text = "Camera ID: " + std::to_string(m_cam_id + 1);

        int font_face = cv::FONT_HERSHEY_SIMPLEX;
        double font_scale = 0.7;
        int thickness = 2;
        cv::Scalar text_color(0, 255, 0);


        cv::Size text_size = cv::getTextSize(cam_id_text, font_face, font_scale, thickness, nullptr);
        
        int padding = 10;
        int rect_x = frame.cols - text_size.width - padding * 2;
        int rect_y = frame.rows - text_size.height - padding * 2;
        int text_x = rect_x + padding;
        int text_y = rect_y + text_size.height + padding;

        cv::Rect bg_rect(rect_x, rect_y, text_size.width + padding * 2, text_size.height + padding * 2);

        cv::Mat overlay;
        frame.copyTo(overlay);

        //cv::rectangle(frame, bg_rect, cv::Scalar(0, 0, 0), -1);
       
        double alpha = 0.0;
        
        cv::addWeighted( overlay(bg_rect)
                       , alpha
                       , frame(bg_rect)
                       , 1.0 - alpha
                       , 0, frame(bg_rect)
                       ) ;

        // cv::putText(  frame
        //             , cam_id_text, cv::Point(text_x, text_y)
        //             , font_face, font_scale
        //             , text_color, thickness
        //             ) ;
        //
        QImage cap_frame = QImage ( frame.data
                                  , frame.cols
                                  , frame.rows
                                  , frame.step
                                  , QImage::Format_RGB888
                                  ) ;
        cap_frame = cap_frame.rgbSwapped();
        emit frame_captured(cap_frame);
        #ifdef ENABLE_GPU
        }
        #endif
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
