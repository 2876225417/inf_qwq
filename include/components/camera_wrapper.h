#pragma once

#include <QMediaCaptureSession>
#include <QCamera>
#include <QWidget>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QMediaDevices>

class camera_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit camera_wrapper(QWidget* parent = nullptr);
    ~camera_wrapper();
private:
    void init_camera();

    QScopedPointer<QCamera> m_camera;
    QMediaCaptureSession m_capture_session;
    QVideoWidget* m_video_widget;
};