#include <windows/mainwindow.h>

mainwindow::mainwindow(QWidget* parent)
    : QMainWindow{parent}
    // mainwindow_layout
    , m_mainwindow_layout{new QWidget()}
    , m_mainwindow_layout_wrapper{new QHBoxLayout()}
    // camera_layout
    , m_mainwindow_camera_layout{new QGroupBox("Camera")}
    , m_mainwindow_camera_layout_wrapper{new QHBoxLayout()}
    { 
    m_camera = new camera_wrapper();

    m_tool_bar = new tool_bar();
    addToolBar(Qt::TopToolBarArea, m_tool_bar);

    m_status_bar = new status_bar();
    setStatusBar(m_status_bar);

    m_mainwindow_camera_layout_wrapper->addWidget(m_camera);
    m_mainwindow_camera_layout_wrapper->setAlignment(m_camera, Qt::AlignLeft | Qt::AlignTop);
    m_mainwindow_camera_layout_wrapper->setContentsMargins(0, 0, 0, 0);
    m_mainwindow_camera_layout->setLayout(m_mainwindow_camera_layout_wrapper);

    m_mainwindow_layout_wrapper->addWidget(m_mainwindow_camera_layout);

    m_mainwindow_layout->setLayout(m_mainwindow_layout_wrapper);
    setCentralWidget(m_mainwindow_layout);
}

mainwindow::~mainwindow() {
       
}