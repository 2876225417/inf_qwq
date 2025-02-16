#pragma once

#include <QMediaCaptureSession>
#include <QCamera>
#include <QWidget>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QMediaDevices>
#include <QPainter>
#include <QMouseEvent>
#include <QVideoSink>
#include <QVideoFrame>
class camera_wrapper_ffmpeg: public QWidget {
    Q_OBJECT
public:
    explicit camera_wrapper_ffmpeg(QWidget* parent = nullptr);
    ~camera_wrapper_ffmpeg();
protected:

void mousePressEvent(QMouseEvent* e) override;
void mouseMoveEvent(QMouseEvent* e) override;
void mouseReleaseEvent(QMouseEvent* e) override;
void paintEvent(QPaintEvent* e) override;

private:
    QImage m_currentFrame;  // 存储当前视频帧
    QVideoSink* m_sink;     // 用于捕捉帧数据
    void init_camera();
    QRect m_selectionRect;
    QPoint m_selectionStart;
    bool m_isSelecting = false;
    QScopedPointer<QCamera> m_camera;
    QMediaCaptureSession m_capture_session;
    QVideoWidget* m_video_widget;
};