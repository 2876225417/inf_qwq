#include "components/camera_wrapper.h"
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qformlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <windows/expanded_camera_window.h>
#include <QDebug>

expanded_camera_window::expanded_camera_window(camera_wrapper* org_cam, QWidget* parent)
    : QWidget{parent, Qt::Window}
    , m_org_camera{org_cam}
{
    setup_UI();
    connect_signals();

    setWindowTitle("Expanded Camera View - Camera " + QString::number(org_cam->get_cam_id()));
    resize(1200, 800);
    setAttribute(Qt::WA_DeleteOnClose);
}

void expanded_camera_window::setup_UI() {
    m_expanded_camera_window_layout = new QVBoxLayout(this);
    m_expanded_camera_window_layout->setContentsMargins(10, 10, 10, 10);
    m_expanded_camera_window_layout->setSpacing(10);

    m_title_label = new QLabel("Camera " 
                              + QString::number(m_org_camera->get_cam_id()) 
                              + " Expanded View");

    QFont title_font = m_title_label->font();
    title_font.setPointSize(12);
    title_font.setBold(true);
    m_title_label->setFont(title_font);
    //m_expanded_camera_window_layout->addWidget(m_title_label);

    m_content_splitter = new QSplitter(Qt::Horizontal, this);
    m_expanded_camera_window_layout->addWidget(m_content_splitter);
    
    create_left_panel();
    create_right_panel();

    QList<int> main_sizes;
    main_sizes << width() * 0.6 << width() * 0.4;
    m_content_splitter->setSizes(main_sizes);

    m_close_window_button = new QPushButton("Close Window");
    //m_expanded_camera_window_layout->addWidget(m_close_window_button, 0, Qt::AlignRight);

    setLayout(m_expanded_camera_window_layout);
}

void expanded_camera_window::create_left_panel() {
    m_left_widget = new QWidget(m_content_splitter);
    m_left_layout = new QVBoxLayout(m_left_widget);
    m_left_layout->setContentsMargins(0, 0, 0, 0);

    // 创建新的camera_wrapper实例
    m_camera = new camera_wrapper(m_org_camera->get_cam_id(), m_left_widget);
    
    // 设置RTSP流
    m_camera->set_rtsp_stream(m_org_camera->get_rtsp_url());
    
    // 将摄像头添加到布局
    m_left_layout->addWidget(m_camera);
    
    m_left_widget->setLayout(m_left_layout);
    m_content_splitter->addWidget(m_left_widget);
}

void expanded_camera_window::create_right_panel() {
    m_right_widget = new QWidget(m_content_splitter);
    m_right_layout = new QVBoxLayout(m_right_widget);
    m_right_layout->setContentsMargins(0, 0, 0, 0);

    m_right_splitter = new QSplitter(Qt::Vertical, m_right_widget);
    m_right_layout->addWidget(m_right_splitter);

    create_right_top_panel();
    create_right_bottom_panel();

    QList<int> right_sizes;
    right_sizes << m_right_widget->height() * 0.7 << m_right_widget->height() * 0.3;
    m_right_splitter->setSizes(right_sizes);
    
    m_right_widget->setLayout(m_right_layout);
    m_content_splitter->addWidget(m_right_widget);
}

void expanded_camera_window::create_right_top_panel() {
    m_right_top_widget = new QWidget(m_right_splitter);
    m_right_top_layout = new QHBoxLayout(m_right_top_widget);
    m_right_top_layout->setContentsMargins(0, 0, 0, 0);

    m_right_top_splitter = new QSplitter(Qt::Horizontal, m_right_top_widget);
    m_right_top_layout->addWidget(m_right_top_splitter);

    m_image_widget = new QWidget(m_right_top_splitter);
    m_image_layout = new QVBoxLayout(m_image_widget);
    //m_image_label = new QLabel("Captured Image", m_image_widget);
    //m_image_label->setAlignment(Qt::AlignCenter);
    //QFont image_font = m_image_label->font();
    //image_font.setBold(true);
    //m_image_label->setFont(image_font);

    m_image_display = new QLabel(m_image_widget);
    m_image_display->setAlignment(Qt::AlignCenter);
    m_image_display->setMinimumSize(200, 200);
    m_image_display->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    m_image_display->setText("Image will be displayed here");

    //m_image_layout->addWidget(m_image_label);
    m_image_layout->addWidget(m_image_display);
    m_image_widget->setLayout(m_image_layout);

    m_text_widget = new QWidget(m_right_top_splitter);
    m_text_layout = new QVBoxLayout(m_text_widget);
    //m_text_label = new QLabel("Recognition Results", m_text_widget);
    //m_text_label->setAlignment(Qt::AlignCenter);
    //QFont text_font = m_text_label->font();
    //text_font.setBold(true);
    //m_text_label->setFont(text_font);
    
    m_text_display = new QTextEdit(m_text_widget);
    m_text_display->setReadOnly(true);
    m_text_display->setText("Recognition results will be displayed here");

    //m_text_layout->addWidget(m_text_label);
    m_text_layout->addWidget(m_text_display);
    m_text_widget->setLayout(m_text_layout);

    m_right_top_splitter->addWidget(m_image_widget);
    m_right_top_splitter->addWidget(m_text_widget);

    QList<int> top_sizes;
    top_sizes << m_right_top_widget->width() * 0.5 
              << m_right_top_widget->width() * 0.5;
    m_right_top_splitter->setSizes(top_sizes);

    m_right_top_widget->setLayout(m_right_top_layout);
    m_right_splitter->addWidget(m_right_top_widget);
}

void expanded_camera_window::create_right_bottom_panel() {
    m_settings_group = new QGroupBox("Detection Settings", m_right_splitter);
    m_settings_layout = new QFormLayout(m_settings_group);

    m_detection_mode = new QComboBox(m_settings_group);
    m_detection_mode->addItems({"Face Detection", "Object Detection", "Text Recognition"});

    m_enable_tracking = new QCheckBox("Enable Tracking", m_settings_group);
    m_enable_tracking->setChecked(true);

    m_confidence_threshold = new QSpinBox(m_settings_group);
    m_confidence_threshold->setRange(1, 100);
    m_confidence_threshold->setValue(50);
    m_confidence_threshold->setSuffix("%");

    m_apply_settings = new QPushButton("Apply Settings", m_settings_group);

    m_settings_layout->addRow("Detection Mode:", m_detection_mode);
    m_settings_layout->addRow("Confidence Threshold:", m_confidence_threshold);
    m_settings_layout->addRow("", m_enable_tracking);
    m_settings_layout->addRow("", m_apply_settings);

    m_settings_group->setLayout(m_settings_layout);
    m_right_splitter->addWidget(m_settings_group);
}

void expanded_camera_window::connect_signals() {
    // 连接关闭按钮
    connect(m_close_window_button, &QPushButton::clicked, this, &expanded_camera_window::close);
    
    // 连接应用设置按钮
    connect(m_apply_settings, &QPushButton::clicked, this, [this]() {
        QString message = QString("Settings applied:\n")
                         + "- Mode: " + m_detection_mode->currentText() + "\n"
                         + "- Confidence: " + QString::number(m_confidence_threshold->value()) + "%\n"
                         + "- Tracking: " + (m_enable_tracking->isChecked() ? "Enabled" : "Disabled");
        
        m_text_display->setText(message);
        qDebug() << "Applied settings:" << message;
    });
}
