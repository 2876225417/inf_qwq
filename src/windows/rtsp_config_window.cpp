

#include "opencv2/core.hpp"
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <windows/rtsp_config_window.h>

rtsp_config_window::rtsp_config_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    // rtsp config main layout
    , m_rtsp_config_main_layout{new QVBoxLayout()}
    // rtsp protocal
    , m_rtsp_protocal_type_wrapper{new QHBoxLayout()}
    // username & passowrd
    , m_username__password_wrapper{new QHBoxLayout()}
    // username
    , m_rtsp_username_wrapper{new QHBoxLayout()}
    // password
    , m_rtsp_password_wrapper{new QHBoxLayout()}
    // ip & port
    , m_ip__port_wrapper{new QHBoxLayout()}
    // ip
    , m_rtsp_ip_wrapper{new QHBoxLayout()}
    // port
    , m_rtsp_port_wrapper{new QHBoxLayout()}
    // channel & subtype
    , m_channel__subtype_wrapper{new QHBoxLayout()}
    // channel
    , m_rtsp_channel_wrapper{new QHBoxLayout()}
    // subtype
    , m_rtsp_subtype_wrapper{new QHBoxLayout()}
    // customized rtsp info
    , m_customized_rtsp_info_wrapper{new QHBoxLayout()}
    // test & connect
    , m_test__connect_wrapper{new QHBoxLayout()}
    {
    // rtsp protocal
    m_rtsp_protocal_type_label = new QLabel("Protocal");
    m_rtsp_protocal_type_select = new QComboBox();
    m_rtsp_protocal_type_wrapper->addWidget(m_rtsp_protocal_type_label);
    m_rtsp_protocal_type_wrapper->addWidget(m_rtsp_protocal_type_select);

    m_rtsp_protocal_type_select->addItem("HIKVISION", static_cast<int>(rtsp_proto_type::HIKVISION));
    m_rtsp_protocal_type_select->addItem("ALHUA", static_cast<int>(rtsp_proto_type::ALHUA));

    m_rtsp_protocal_type_select->setCurrentIndex(0);

    // username
    m_rtsp_username_label = new QLabel("Username");
    m_rtsp_username_edit = new QLineEdit();
    m_rtsp_username_wrapper->addWidget(m_rtsp_username_label);
    m_rtsp_username_wrapper->addWidget(m_rtsp_username_edit);
    
    // password
    m_rtsp_password_label = new QLabel("Password");
    m_rtsp_password_edit = new QLineEdit();
    m_rtsp_password_wrapper->addWidget(m_rtsp_password_label);
    m_rtsp_password_wrapper->addWidget(m_rtsp_password_edit);

    // username & password
    m_username__password_wrapper->addLayout(m_rtsp_username_wrapper);
    m_username__password_wrapper->addLayout(m_rtsp_password_wrapper);

    // ip
    m_rtsp_ip_label = new QLabel("IP");
    m_rtsp_ip_edit = new QLineEdit();
    m_rtsp_ip_wrapper->addWidget(m_rtsp_ip_label);
    m_rtsp_ip_wrapper->addWidget(m_rtsp_ip_edit);

    // port
    m_rtsp_port_label = new QLabel("Port");
    m_rtsp_port_edit = new QLineEdit();
    m_rtsp_port_wrapper->addWidget(m_rtsp_port_label);
    m_rtsp_port_wrapper->addWidget(m_rtsp_port_edit);

    // ip & port
    m_ip__port_wrapper->addLayout(m_rtsp_ip_wrapper);
    m_ip__port_wrapper->addLayout(m_rtsp_port_wrapper);

    // channel
    m_rtsp_channel_label = new QLabel("Channel");
    m_rtsp_channel_edit = new QLineEdit();
    m_rtsp_channel_wrapper->addWidget(m_rtsp_channel_label);
    m_rtsp_channel_wrapper->addWidget(m_rtsp_channel_edit);

    // subtype
    m_rtsp_subtype_label = new QLabel("Subtype");
    m_rtsp_subtype_edit = new QLineEdit();
    m_rtsp_subtype_wrapper->addWidget(m_rtsp_subtype_label);
    m_rtsp_subtype_wrapper->addWidget(m_rtsp_subtype_edit);

    // channel & subtype
    m_channel__subtype_wrapper->addLayout(m_rtsp_channel_wrapper);
    m_channel__subtype_wrapper->addLayout(m_rtsp_subtype_wrapper);

    // customized rtsp info
    m_customized_rtsp_info_label = new QLabel("RTSP");
    m_customized_rtsp_info_edit = new QLineEdit();
    m_customized_rtsp_info_wrapper->addWidget(m_customized_rtsp_info_label);
    m_customized_rtsp_info_wrapper->addWidget(m_customized_rtsp_info_edit);
        
    // test
    m_test_button = new QPushButton("Test");

    connect ( m_test_button
            , &QPushButton::clicked
            , this, [this]() {
                emit send_rtsp_url(m_customized_rtsp_info_edit->text());
            });

    // connect
    m_connect_button = new QPushButton("Connect");
    
    // test & connect
    m_test__connect_wrapper->addWidget(m_test_button);
    m_test__connect_wrapper->addWidget(m_connect_button);
    
    // initialize customized rtsp info
    m_customized_rtsp_info_edit->setText(m_rtsp_config.config2url());

    // rtsp protocal
    connect ( m_rtsp_protocal_type_select
            , QOverload<int>::of(&QComboBox::currentIndexChanged)
            , this, [this](int index){    
                rtsp_proto_type rpt 
                    = static_cast<rtsp_proto_type>(m_rtsp_protocal_type_select->itemData(index).toInt());
                qDebug() << "protocal: " << (rpt == rtsp_proto_type::HIKVISION ? "HIKVISION" : "ALHUA"); 
                m_rtsp_config.rpt = rpt;
                m_customized_rtsp_info_edit->setText(m_rtsp_config.config2url());
            });
        
    // username
    connect ( m_rtsp_username_edit
            , &QLineEdit::textEdited
            , this, [this](const QString& username){
                qDebug() << "username: " << username;
                m_rtsp_config.username = username; 
                m_customized_rtsp_info_edit->setText(m_rtsp_config.config2url()); 
            });

    // password
    connect ( m_rtsp_password_edit
            , &QLineEdit::textEdited
            , this, [this](const QString& password){ 
                qDebug() << "password: " << password;
                m_rtsp_config.password = password;
                m_customized_rtsp_info_edit->setText(m_rtsp_config.config2url());
            });

    // ip
    connect ( m_rtsp_ip_edit
            , &QLineEdit::textEdited
            , this, [this](const QString& ip) {
                qDebug() << "ip: " << ip;
                m_rtsp_config.ip = ip;
                m_customized_rtsp_info_edit->setText(m_rtsp_config.config2url());
            });

    // port
    connect ( m_rtsp_port_edit
            , &QLineEdit::textEdited
            , this, [this](const QString& port){
                qDebug() << "port: " << port;
                m_rtsp_config.port = port;
                m_customized_rtsp_info_edit->setText(m_rtsp_config.config2url());
            });

    // channel
    connect ( m_rtsp_channel_edit
            , &QLineEdit::textEdited
            , this, [this](const QString& channel){
                qDebug() << "channel: " << channel;
                m_rtsp_config.channel = channel;
                m_customized_rtsp_info_edit->setText(m_rtsp_config.config2url());
            });

    // subtype
    connect ( m_rtsp_subtype_edit
            , &QLineEdit::textEdited
            , this, [this](const QString& subtype){
                qDebug() << "subtype: " << subtype;    
                m_rtsp_config.subtype = subtype;
                m_customized_rtsp_info_edit->setText(m_rtsp_config.config2url());
            });
   
    // customized rtsp info
    connect ( m_customized_rtsp_info_edit
            , &QLineEdit::textEdited
            , this, [](const QString& customized_rtsp_info){
                qDebug() << "customized rtsp info: " << customized_rtsp_info;
                 
            });

    // test
    connect ( m_test_button
            , &QPushButton::clicked
            , this, []() {
                qDebug() << "try to test";
            });
    
    // connect
    connect ( m_connect_button
            , &QPushButton::clicked
            , this, [](){
                qDebug() << "try to connect";
            });

    m_rtsp_config_main_layout->addLayout(m_rtsp_protocal_type_wrapper);
    m_rtsp_config_main_layout->addLayout(m_username__password_wrapper);
    m_rtsp_config_main_layout->addLayout(m_ip__port_wrapper);
    m_rtsp_config_main_layout->addLayout(m_channel__subtype_wrapper);
    m_rtsp_config_main_layout->addLayout(m_customized_rtsp_info_wrapper);
    m_rtsp_config_main_layout->addLayout(m_test__connect_wrapper);

    setLayout(m_rtsp_config_main_layout);
}
