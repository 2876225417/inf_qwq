#include <components/actions_wrapper.h>
#include <qabstractspinbox.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>


actions_wrapper::actions_wrapper(QWidget* parent)
    : QWidget{parent}
    , m_ort_inferer{ std::make_unique<ort_inferer>() }
    , m_actions_layout{new QGroupBox(this)}
    , m_actions_layout_wrapper{new QHBoxLayout(m_actions_layout)}
    // col 1
    , m_col_1_layout_wrapper{new QVBoxLayout()}
    // start inf
    , m_start_inf_wrapper{new QHBoxLayout()}
    // connector
    , m_connector_wrapper{new QHBoxLayout()}
    // exit app
    , m_exit_app_wrapper{new QHBoxLayout()}

    // col_2
    , m_col_2_layout_wrapper{new QVBoxLayout()}
    // col_2_row_1
    , m_col_2_row_1_layout_wrapper{new QHBoxLayout()}
    // edit username
    , m_edit_username_wrapper{new QHBoxLayout()}
    // edit password
    , m_edit_password_wrapper{new QHBoxLayout()}
    
    // col_2_row_2
    , m_col_2_row_2_layout_wrapper{new QHBoxLayout()}
    // edit ip
    , m_edit_ip_wrapper{new QHBoxLayout()}
    // edit port
    , m_edit_port_wrapper{new QHBoxLayout} 
    // adjust_scale 
    , m_adjust_camera_scale_wrapper{new QHBoxLayout()}
    // col_2_row_3
    , m_col_2_row_3_layout_wrapper{new QHBoxLayout()}
    // edit keywords
    , m_edit_keywords_wrapper{new QHBoxLayout()}
    {
    // col_1
    // start inf
    m_start_inf = new QPushButton("开始识别");
    m_start_inf_wrapper->addWidget(m_start_inf);
    
    connect (m_start_inf, &QPushButton::clicked, this, [this]() {
        emit start_inf();
    });

    // connector
    m_connector = new QPushButton("连接");
    m_connector_wrapper->addWidget(m_connector);

    connect (m_connector, &QPushButton::clicked, this, [this]() {
        emit connect_cam();
    });

    // exit app
    m_exit_app = new QPushButton("退出");
    m_exit_app_wrapper->addWidget(m_exit_app);
    
    connect (m_exit_app, &QPushButton::clicked, this, [this]() {
        emit exit_app();
    });

    m_col_1_layout_wrapper->addLayout(m_start_inf_wrapper);     // start inf
    m_col_1_layout_wrapper->addLayout(m_connector_wrapper);   // placeholder
    m_col_1_layout_wrapper->addLayout(m_exit_app_wrapper);      // exit app
    
    // col 2
    // col_2_row_1
    // edit username
    m_edit_username_label = new QLabel("username(用户名)");
    m_edit_username = new QLineEdit();
    m_edit_username->setFixedWidth(160);
    m_edit_username_wrapper->addWidget(m_edit_username_label);
    m_edit_username_wrapper->addWidget(m_edit_username);

    connect (m_edit_username, &QLineEdit::textChanged, this, [this]() {
        emit username_changed(m_edit_username->text());
    });

    // edit password
    m_edit_password_label = new QLabel("password(密码)");
    m_edit_password = new QLineEdit();
    m_edit_password->setFixedWidth(160);
    m_edit_password_wrapper->addWidget(m_edit_password_label);
    m_edit_password_wrapper->addWidget(m_edit_password);

    connect (m_edit_password, &QLineEdit::textChanged, this, [this]() {
        emit password_changed(m_edit_password->text());
    });

    m_col_2_row_1_layout_wrapper->addLayout(m_edit_username_wrapper);   // username
    m_col_2_row_1_layout_wrapper->addLayout(m_edit_password_wrapper);   // password

    // col_2_row_2
    // edit ip
    m_edit_ip_label = new QLabel("ip");
    m_edit_ip = new QLineEdit();
    m_edit_ip->setFixedWidth(130);
    m_edit_ip_wrapper->addWidget(m_edit_ip_label);
    m_edit_ip_wrapper->addWidget(m_edit_ip);
    
    connect (m_edit_ip, &QLineEdit::textChanged, this, [this]() {
        emit ip_changed(m_edit_ip->text());
    });

    // edit port
    m_edit_port_label = new QLabel("port(端口)");
    m_edit_port = new QLineEdit();
    m_edit_port->setFixedWidth(80);
    m_edit_port_wrapper->addWidget(m_edit_port_label);
    m_edit_port_wrapper->addWidget(m_edit_port);
    
    connect (m_edit_port, &QLineEdit::textChanged, this, [this]() {
        emit port_changed(m_edit_port->text());
    });

    // adjust camera scale
    m_camera_scale_label = new QLabel("缩放倍率");
    m_adjust_camera_scale = new QComboBox();

    m_adjust_camera_scale->addItem("50%", 0.5);
    m_adjust_camera_scale->addItem("75%", 0.75);
    m_adjust_camera_scale->addItem("100%", 1.0);
    m_adjust_camera_scale->addItem("125%", 1.25);
    m_adjust_camera_scale->addItem("150%", 1.5);
    m_adjust_camera_scale->addItem("175%", 1.75);
    m_adjust_camera_scale->addItem("200%", 2.0);
    m_adjust_camera_scale->addItem("250%", 2.5);
    m_adjust_camera_scale->addItem("300%", 3.0);
    
    m_adjust_camera_scale->setCurrentIndex(2);

    connect ( m_adjust_camera_scale
            , QOverload<int>::of(&QComboBox::currentIndexChanged)
            , this, [this](int index) {
                double scale_factor = m_adjust_camera_scale->itemData(index).toDouble();
                //qDebug() << "Scale factor: " << scale_factor;
                emit scale_factor_changed(scale_factor);
            });

    m_adjust_camera_scale_wrapper->addWidget(m_camera_scale_label);
    m_adjust_camera_scale_wrapper->addWidget(m_adjust_camera_scale);
    

    m_col_2_row_2_layout_wrapper->addLayout(m_edit_ip_wrapper);         // ip
    m_col_2_row_2_layout_wrapper->addLayout(m_edit_port_wrapper);       // port
    m_col_2_row_2_layout_wrapper->addLayout(m_adjust_camera_scale_wrapper);     // scale

    // col_2_row_3
    // edit keywords
    m_edit_keywords_label = new QLabel("关键词");
    m_edit_keywords = new QLineEdit();
    m_edit_keywords_wrapper->addWidget(m_edit_keywords_label);
    m_edit_keywords_wrapper->addWidget(m_edit_keywords);
    connect(m_edit_keywords, &QLineEdit::textChanged, this, [this]() {
        // qDebug() << "keywords: " << m_edit_keywords->text();
        emit keywords_changed(m_edit_keywords->text());
    });

    m_col_2_row_3_layout_wrapper->addLayout(m_edit_keywords_wrapper);   // keywords

    m_col_2_layout_wrapper->addLayout(m_col_2_row_1_layout_wrapper);
    m_col_2_layout_wrapper->addLayout(m_col_2_row_2_layout_wrapper);
    m_col_2_layout_wrapper->addLayout(m_col_2_row_3_layout_wrapper);

    m_actions_layout_wrapper->addLayout(m_col_1_layout_wrapper, 1);
    m_actions_layout_wrapper->addLayout(m_col_2_layout_wrapper, 4);
    m_actions_layout->setLayout(m_actions_layout_wrapper);    
}
