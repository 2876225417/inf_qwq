#include "components/camera_wrapper.h"
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
    , m_camera_cropped_layout_wrapper{new QVBoxLayout()}
    , m_cropped_img_1_and_2_wrapper{new QHBoxLayout()}
    , m_cropped_img_3_and_4_wrapper{new QHBoxLayout()}
    { 
    m_camera = new camera_wrapper();

    m_tool_bar = new tool_bar();
    addToolBar(Qt::TopToolBarArea, m_tool_bar);

    m_status_bar = new status_bar();
    setStatusBar(m_status_bar);

    m_4_croppeds_img = new cropped_wrapper<4>(this); 

    m_cropped_img_1 = new cropped_img_wrapper();
    m_cropped_img_2 = new cropped_img_wrapper();
    m_cropped_img_3 = new cropped_img_wrapper();
    m_cropped_img_4 = new cropped_img_wrapper();
    m_cropped_img_1_and_2_wrapper->addWidget(m_4_croppeds_img);
    m_actions_wrapper = new actions_wrapper();

    connect ( m_camera
            , &camera_wrapper::img_cropped
            , this
            , [this](QVector<cropped_image>& cropped_images) {
                qDebug() << "count of images: " << cropped_images.size();
                for (auto& cropped: cropped_images)
                    m_4_croppeds_img->set_image(cropped.number - 1, cropped.image);
            });

    m_camera_cropped_layout_wrapper->addLayout(m_cropped_img_1_and_2_wrapper);
    m_camera_cropped_layout_wrapper->addLayout(m_cropped_img_3_and_4_wrapper);
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
    resize(1400, 700);
    setCentralWidget(m_mainwindow_layout);
}

mainwindow::~mainwindow() {
       
}