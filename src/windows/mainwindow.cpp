#include <windows/mainwindow.h>

mainwindow::mainwindow(QWidget* parent)
    : QMainWindow{parent}
    { 
    mainwindow_layout = new QHBoxLayout(this);
    m_camera = new camera_wrapper(this);

    mainwindow_layout->addWidget(m_camera);

    setLayout(mainwindow_layout);
}

mainwindow::~mainwindow() {
       
}