#include <windows/mainwindow.h>

mainwindow::mainwindow(QWidget* parent)
    : QMainWindow{parent}
    { 
    m_mainwindow_layout = new QWidget();
    m_mainwindow_layout_wrapper = new QHBoxLayout(m_mainwindow_layout);
    
    m_camera = new camera_wrapper(this);
    m_mainwindow_layout_wrapper->addWidget(m_camera);
    m_mainwindow_layout_wrapper->setContentsMargins(0, 0, 0, 0);

    
    setCentralWidget(m_mainwindow_layout);
}

mainwindow::~mainwindow() {
       
}