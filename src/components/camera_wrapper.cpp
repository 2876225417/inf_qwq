

#include <components/camera_wrapper.h>
#include <qimage.h>
#include <qpixmap.h>



camera_wrapper::camera_wrapper(QWidget* parent)
    : QWidget{nullptr}
    , m_camera_layout{new QHBoxLayout()}
    , m_video_stream{new QLabel("Wait for streaming...")}
    {

    m_video_capturer = new video_capturer();

    connect( m_video_capturer 
           , &video_capturer::frame_captured
           , this
           , [this](QImage frame) {
                qDebug() << "Captured!";
           });
    m_video_capturer->run();
    m_camera_layout->addWidget(m_video_stream);

    setLayout(m_camera_layout);
}

camera_wrapper::~camera_wrapper() {

}

