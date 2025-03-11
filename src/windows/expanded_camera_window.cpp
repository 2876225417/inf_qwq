#include "components/camera_wrapper.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <windows/expanded_camera_window.h>


expanded_camera_window::expanded_camera_window( camera_wrapper* org_cam
                                              , QWidget* parent
                                              ):QWidget{parent, Qt::Window}
                                              , m_expanded_camera_window_layout{new QVBoxLayout()}
                                              {
    m_close_window_button = new QPushButton("close", this);
    m_title_label = new QLabel(this);
    m_org_camera = org_cam;
    setWindowTitle("Expanded Camera View");
    resize(800, 600);

    QHBoxLayout* title_layout = new QHBoxLayout();
    title_layout->addWidget(m_title_label);
    title_layout->addStretch();
    title_layout->addWidget(m_close_window_button);

    m_camera = new camera_wrapper(org_cam->get_cam_id(), this);
    
    m_camera->set_rtsp_stream(org_cam->get_rtsp_url());

    m_expanded_camera_window_layout->addWidget(m_camera);

    setLayout(m_expanded_camera_window_layout);

    connect (m_close_window_button, &QPushButton::clicked, this, &expanded_camera_window::close);

    setAttribute(Qt::WA_DeleteOnClose);
}
 
