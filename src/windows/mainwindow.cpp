#include <windows/mainwindow.h>

mainwindow::mainwindow(QWidget* parent)
    : QMainWindow{parent}
    // mainwindow_layout
    , m_mainwindow_layout{new QWidget()}
    , m_mainwindow_layout_wrapper{new QHBoxLayout()}
    , m_camera_rel_layout{new QVBoxLayout()}
    // camera_layout
    , m_mainwindow_camera_layout{new QGroupBox("Camera")}
    , m_mainwindow_camera_layout_wrapper{new QHBoxLayout()}
    // --camera_cropped_layout
    , m_camera_cropped_layout{new QGroupBox("Cropped")}
    , m_camera_cropped_layout_wrapper{new QHBoxLayout()}
    { 
    m_camera = new camera_wrapper();

    m_tool_bar = new tool_bar();
    addToolBar(Qt::TopToolBarArea, m_tool_bar);

    m_status_bar = new status_bar();
    setStatusBar(m_status_bar);

    m_cropped_img = new cropped_img_wrapper();

    m_actions_wrapper = new actions_wrapper();

    connect ( m_camera
            , &camera_wrapper::img_cropped
            , this
            , [this](QImage& img) {
                m_cropped_img->set_cropped_image(img);
            });

    m_camera_cropped_layout_wrapper->addWidget(m_cropped_img);
    m_camera_cropped_layout->setLayout(m_camera_cropped_layout_wrapper);
    
    m_mainwindow_camera_layout_wrapper->addWidget(m_camera);
    m_mainwindow_camera_layout_wrapper->setAlignment(m_camera, Qt::AlignLeft | Qt::AlignTop);
    m_mainwindow_camera_layout_wrapper->setContentsMargins(0, 0, 0, 0);
    m_mainwindow_camera_layout->setLayout(m_mainwindow_camera_layout_wrapper);
    m_mainwindow_camera_layout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_camera_rel_layout->addWidget(m_mainwindow_camera_layout);
    m_camera_rel_layout->addWidget(m_actions_wrapper);
    m_mainwindow_layout_wrapper->addLayout(m_camera_rel_layout);
    m_mainwindow_layout_wrapper->addWidget(m_camera_cropped_layout);

    m_mainwindow_layout->setLayout(m_mainwindow_layout_wrapper);
    setCentralWidget(m_mainwindow_layout);
}

mainwindow::~mainwindow() {
       
}