

#include "components/draw_overlay.h"
#include "utils/video_capturer.h"
#include <components/camera_wrapper.h>
#include <qimage.h>
#include <qnamespace.h>
#include <qpixmap.h>

#include <QResizeEvent>

#include <QLabel>

camera_wrapper::camera_wrapper(int cam_id, QWidget* parent)
    : QWidget{parent}
    , m_camera_layout{new QHBoxLayout()}
    , m_video_stream{new QLabel("Wait for streaming...", this)}
    , m_draw_overlay{new draw_overlay(cam_id)}
    , m_timer{new QTimer(this)}
    , m_cam_id{cam_id}
    {
    m_camera_layout->setContentsMargins(0, 0, 0, 0);
    m_video_stream->setAlignment(Qt::AlignCenter);
    m_video_capturer = new video_capturer(cam_id);  
    m_video_stream->installEventFilter(this);
    m_video_stream->setStyleSheet("QLabel { background: #1A1A1A; }"); 
    m_draw_overlay->setParent(m_video_stream); 
    
    // connect(m_video_stream, &QLabel::resize, [=]{
    //     m_draw_overlay->resize(m_video_stream->size());
    // }); // realtime redraw
    m_draw_overlay->resize(m_video_stream->size());
   
    connect ( m_draw_overlay
            , &draw_overlay::expand_camera_request
            , this, [this](int cam_id){
                emit cam_expand_req(cam_id); 
            });


    m_camera_layout->addWidget(m_video_stream);
    
    

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(m_camera_layout);

    connect ( m_draw_overlay
            , &draw_overlay::selected
            , [this](QVector<rect_data>& rects) {
                rect2image(rects);
                emit img_cropped(m_cropped_images);
            });


    connect ( m_draw_overlay
            , &draw_overlay::update_frame_moving
            , [this](QVector<rect_data>& rects) {
                rect2image(rects); 
                // debug
                // qDebug() << "video rect number: " 
                //          << rect.number 
                //          << "and coords: " << video_rect;
                emit img_cropped(m_cropped_images);
            });

    connect (m_draw_overlay, &draw_overlay::timer_timeout_update, [this](QVector<rect_data>& rects) {
                qDebug() << "Frame updated";
                rect2image(rects);
                emit img_cropped4inf(m_cropped_images);
            });

    // camera stream  
    connect( m_video_capturer 
           , &video_capturer::frame_captured
           , this
           , [this](QImage frame) {
                m_current_frame = frame.copy();
                QSize label_size = m_video_stream->size();
                QSize scaled_size = frame.size();

                scaled_size.scale( label_size
                                 , Qt::KeepAspectRatio
                                 ) ;
                QImage scaled_image = frame.scaled( scaled_size
                                                  , Qt::KeepAspectRatio
                                                  , Qt::SmoothTransformation
                                                  ) ;
                
                QImage background(label_size, QImage::Format_RGB888);
                background.fill(Qt::black);

                QPainter painter(&background);
                int x = (label_size.width() - scaled_size.width()) / 2;
                int y = (label_size.height() - scaled_size.height()) / 2;
                painter.drawImage(x, y, scaled_image);
                    

                m_video_stream->setPixmap(QPixmap::fromImage(background));
                m_current_frame = frame.copy();
           }); m_video_capturer->start(); 
}

void camera_wrapper::set_scale_factor(double factor) {
    m_video_capturer->set_scale_factor(factor);
}

bool camera_wrapper::set_rtsp_stream(const QString& rtsp_url) {
    qDebug() << "Try to switch to rtsp_url: " << rtsp_url;
    return m_video_capturer->switch_rtsp_stream(rtsp_url);
}

QString camera_wrapper::get_rtsp_url() const {
    return m_video_capturer->get_rtsp_url();
} 



int camera_wrapper::get_cam_id() const {
    return this->m_cam_id;
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

void camera_wrapper::rect2image(QVector<rect_data>& rects) {
    m_cropped_images.clear();
    for (auto& rect: rects) {
        QRect video_rect = rect2coords(rect.rect);                
        QImage cropped = m_current_frame.copy(video_rect);
        cropped_image tmp{rect.number, cropped};
        m_cropped_images.append(tmp);
    }
}

bool camera_wrapper::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_video_stream && event->type() == QEvent::Resize) {
        m_draw_overlay->resize(m_video_stream->size());
    }
    return QWidget::eventFilter(watched, event);
}

camera_wrapper::~camera_wrapper() { }

