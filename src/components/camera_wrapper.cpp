

#include <components/camera_wrapper.h>
#include <qimage.h>
#include <qpixmap.h>



camera_wrapper::camera_wrapper(QWidget* parent)
    : QWidget{parent}
    , m_camera_layout{new QHBoxLayout()}
    , m_video_stream{new QLabel("Wait for streaming...", this)}
    , m_draw_overlay{new draw_overlay} 
    {
    m_video_capturer = new video_capturer();  
    m_video_stream->installEventFilter(this);
    m_video_stream->setStyleSheet("QLabel { border: 2px solid #808080; }"); 
    m_draw_overlay->setParent(m_video_stream); 
    m_draw_overlay->resize(m_video_stream->size());

    connect ( m_draw_overlay
            , &draw_overlay::selected
            , [this](const QRect& rect) {
                QRect video_rect = rect2coords(rect);
                QImage cropped = m_current_frame.copy(video_rect);
                // debug
                // qDebug() << "video rect coords: " << video_rect;
            });
    connect( m_video_capturer 
           , &video_capturer::frame_captured
           , this
           , [this](QImage frame) {
                m_video_stream->setPixmap(QPixmap::fromImage(frame));
                m_current_frame = frame.copy();
           });
    m_camera_layout->addWidget(m_video_stream);
    m_video_capturer->start(); 
    setLayout(m_camera_layout);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


QRect camera_wrapper::rect2coords(const QRect& rect) {
    QSize video_size = m_video_stream->pixmap().size();
    QSize widgets_size = m_video_stream->size();

    qreal x_scale = video_size.width() / (qreal)widgets_size.width();
    qreal y_scale = video_size.height() / (qreal)widgets_size.height();

    return QRect(
        rect.x() * x_scale,
        rect.y() * y_scale,
        rect.width() * x_scale,
        rect.height() * y_scale
    );
}

bool camera_wrapper::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_video_stream && event->type() == QEvent::Resize) {
        m_draw_overlay->resize(m_video_stream->size());
    }
    return QWidget::eventFilter(watched, event);
}


camera_wrapper::~camera_wrapper() {

}

