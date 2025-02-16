#include <components/camera_wrapper.h>

camera_wrapper::camera_wrapper(QWidget* parent)
    : QWidget{parent}
    { init_camera(); }

camera_wrapper::~camera_wrapper() {
    if (m_camera && m_camera->isActive()) {
        m_camera->stop();
    }
}

void camera_wrapper::init_camera() {
    m_video_widget = new QVideoWidget(this);
    QVBoxLayout* camera_layout = new QVBoxLayout();
    camera_layout->setContentsMargins(0, 0, 0, 0);
    camera_layout->addWidget(m_video_widget);

    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (!cameras.isEmpty()) {
        m_camera.reset(new QCamera(cameras.first()));
        m_capture_session.setCamera(m_camera.data());
        m_capture_session.setVideoOutput(m_video_widget);
        m_camera->start();
    }
}

