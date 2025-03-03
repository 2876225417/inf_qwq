#pragma once

#include <QPixmap>
#include <QTimer>
#include <QThread>
#include <opencv2/opencv.hpp>
#include <qthread.h>
#include <atomic>
class video_capturer: public QThread {
    Q_OBJECT
public:
    explicit video_capturer(QThread* parent = nullptr);
    ~video_capturer();

    void stop();
    void run() override;
signals:
    void frame_captured(QImage frame);
private:
    cv::VideoCapture m_capturer;
    std::atomic_bool m_stop{false};
};

