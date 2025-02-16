#pragma once

#include <QPixmap>
#include <QObject>
#include <QTimer>

#include <opencv2/opencv.hpp>


class video_capturer: public QObject {
    Q_OBJECT
public:
    explicit video_capturer(QObject* parent = nullptr);
    ~video_capturer();

    void start();
    void stop();
signals:
    void frame_captured(QPixmap frame);
private slots:
    void capture_frame();

private:
    cv::VideoCapture m_capturer;
    QTimer* m_timer;
};

