

#include <components/camera_wrapper.h>
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
           , [this](QPixmap frame) {
                m_video_stream->setPixmap(frame);
           });
    m_camera_layout->addWidget(m_video_stream);

    setLayout(m_camera_layout);
}

camera_wrapper::~camera_wrapper() {

}

