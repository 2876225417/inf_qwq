#include "components/camera_wrapper_ffmpeg.h"

camera_wrapper_ffmpeg::camera_wrapper_ffmpeg(QWidget* parent)
    : QWidget{parent}
    { init_camera(); }

camera_wrapper_ffmpeg::~camera_wrapper_ffmpeg() {
    if (m_camera && m_camera->isActive()) {
        m_camera->stop();
    }
}

void camera_wrapper_ffmpeg::init_camera() {
    m_video_widget = new QVideoWidget(this);
    m_video_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_video_widget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
    QVBoxLayout* camera_layout = new QVBoxLayout();
    camera_layout->addWidget(m_video_widget);
    m_video_widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (!cameras.isEmpty()) {
        m_camera.reset(new QCamera(cameras.first()));
        m_capture_session.setCamera(m_camera.data());
        m_capture_session.setVideoOutput(m_video_widget);
        m_camera->start();
    }
    setLayout(camera_layout);
}

void camera_wrapper_ffmpeg::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_isSelecting = true;
        m_selectionStart = e->pos();
        m_selectionRect = QRect();
    }
    QWidget::mousePressEvent(e);
}

void camera_wrapper_ffmpeg::mouseMoveEvent(QMouseEvent* e) {
    if (m_isSelecting) {
        m_selectionRect = QRect(m_selectionStart, e->pos()).normalized();
        update();  // 触发重绘
    }
    QWidget::mouseMoveEvent(e);
}

void camera_wrapper_ffmpeg::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && m_isSelecting) {
        m_isSelecting = false;
        // 这里可以获得最终框选区域
        qDebug() << "Selected area:" << m_selectionRect;
    }
}

// 3. 添加绘制逻辑
void camera_wrapper_ffmpeg::paintEvent(QPaintEvent*) {
    if (!m_selectionRect.isEmpty()) {
        QPainter painter(this);
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(m_selectionRect);
        
        // 半透明填充（可选）
        painter.setBrush(QColor(255, 0, 0, 50));
        painter.drawRect(m_selectionRect);
    }
}
