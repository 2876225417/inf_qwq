#pragma once

#include <QPixmap>
#include <QTimer>
#include <QThread>
#include <opencv2/opencv.hpp>
#include <qthread.h>
#include <atomic>
#include <QString>
#include <QMutex>
class video_capturer: public QThread {
    Q_OBJECT
public:
    explicit video_capturer(int cam_id, QThread* parent = nullptr);
    ~video_capturer();

    void stop();
    void run() override;

    bool switch_camera(int camera_index);
    bool switch_rtsp_stream(const QString& rtsp_url);
    bool is_running() const { return QThread::isRunning(); }

    void set_scale_factor(double factor);
    double get_scale_factor() const;
    QString get_rtsp_url() const;
signals:
    void frame_captured(QImage frame);
    void camera_error(const QString& error_msg);
private:
    int m_cam_id;
    QString m_rtsp_url;
    cv::VideoCapture m_capturer;
    std::atomic_bool m_stop{false};
    std::atomic_bool m_swtich_pending{false};
    int m_current_camera_index{0};
    QString m_current_rtsp_url;

    double m_scale_factor{1.0};
    mutable QMutex m_scale_mutex;

    bool open_camera(int index);
    bool open_rtsp_stream(const QString& url);
    
};

