#include "windows/rtsp_config_window.h"
#include "utils/db_manager_mini.h"
#include "utils/http_server.h"
#include <QApplication>
#include <QStyle>
#include <QGroupBox>
#include <QDebug>
#include <QMessageBox>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qendian.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qoverload.h>
#include <qpushbutton.h>


rtsp_config_window::rtsp_config_window(QWidget* parent)
    : QWidget(parent, Qt::Window)
    {
    setWindowTitle(tr("RTSP配置"));
    setMinimumWidth(500);
    setMinimumHeight(350);
    m_settings = new QSettings("Chun Hui", "inf_qwq", this);
    setup_UI();
    create_connections();
    load_settings();
}

rtsp_config_window::~rtsp_config_window(){ }

void rtsp_config_window::setup_UI() {
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setSpacing(10);
    m_main_layout->setContentsMargins(10, 10, 10, 10);
   
    // RTSP Protocal Type & RTSP Stream Name
    m_rtsp_protocal__name_layout = new QHBoxLayout();

    // RTSP Protocal Type
    m_rtsp_proto_layout = new QHBoxLayout();
    m_rtsp_proto_label = new QLabel(tr("协议类型:"));
    m_rtsp_proto_combo = new QComboBox();
    m_rtsp_proto_combo->addItem("HIKVISION", static_cast<int>(rtsp_proto_type::HIKVISION));
    m_rtsp_proto_combo->addItem("DAHUA", static_cast<int>(rtsp_proto_type::ALHUA));
    m_rtsp_proto_layout->addWidget(m_rtsp_proto_label);
    m_rtsp_proto_layout->addWidget(m_rtsp_proto_combo);
     
    // RTSP stream name
    m_rtsp_stream_name_layout = new QHBoxLayout();
    m_rtsp_stream_name_label = new QLabel(tr("RTSP信息"));
    m_rtsp_stream_name_edit = new QLineEdit();
    m_rtsp_stream_name_edit->setPlaceholderText(tr("RTSP信息（可选）"));
    m_rtsp_stream_name_layout->addWidget(m_rtsp_stream_name_label);
    m_rtsp_stream_name_layout->addWidget(m_rtsp_stream_name_edit);
    
    // devide average
    m_rtsp_protocal__name_layout->addLayout(m_rtsp_proto_layout);
    m_rtsp_protocal__name_layout->addLayout(m_rtsp_stream_name_layout);

    // Username & Password
    m_user_pass_layout = new QHBoxLayout();
    
    // Username
    m_username_layout = new QHBoxLayout();
    m_username_label = new QLabel(tr("用户名:"));
    m_username_edit = new QLineEdit();
    m_username_edit->setPlaceholderText(tr("username"));
    m_username_layout->addWidget(m_username_label);
    m_username_layout->addWidget(m_username_edit);
    
    // Password
    m_password_layout = new QHBoxLayout();
    m_password_label = new QLabel(tr("密码:"));
    m_password_edit = new QLineEdit();
    m_password_edit->setPlaceholderText(tr("password"));
    m_password_edit->setEchoMode(QLineEdit::Password);
    m_password_layout->addWidget(m_password_label);
    m_password_layout->addWidget(m_password_edit);
    
    m_user_pass_layout->addLayout(m_username_layout);
    m_user_pass_layout->addLayout(m_password_layout);
    
    // IP & Port
    m_ip_port_layout = new QHBoxLayout();
    
    // IP
    m_ip_layout = new QHBoxLayout();
    m_ip_label = new QLabel(tr("IP地址:"));
    m_ip_edit = new QLineEdit();
    m_ip_edit->setPlaceholderText(tr("192.168.1.100"));
    m_ip_layout->addWidget(m_ip_label);
    m_ip_layout->addWidget(m_ip_edit);
    
    // Port
    m_port_layout = new QHBoxLayout();
    m_port_label = new QLabel(tr("端口:"));
    m_port_edit = new QLineEdit();
    m_port_edit->setPlaceholderText(tr("554"));
    m_port_layout->addWidget(m_port_label);
    m_port_layout->addWidget(m_port_edit);
    
    m_ip_port_layout->addLayout(m_ip_layout);
    m_ip_port_layout->addLayout(m_port_layout);
    
    // Channel & Subtype
    m_channel_subtype_layout = new QHBoxLayout();
    
    // Channel
    m_channel_layout = new QHBoxLayout();
    m_channel_label = new QLabel(tr("频道:"));
    m_channel_edit = new QLineEdit();
    m_channel_edit->setText("101");
    m_channel_layout->addWidget(m_channel_label);
    m_channel_layout->addWidget(m_channel_edit);
    
    // Subtype
    m_subtype_layout = new QHBoxLayout();
    m_subtype_label = new QLabel(tr("码流类型:"));
    m_subtype_edit = new QLineEdit();
    m_subtype_edit->setText("0");
    m_subtype_layout->addWidget(m_subtype_label);
    m_subtype_layout->addWidget(m_subtype_edit);
    
    m_channel_subtype_layout->addLayout(m_channel_layout);
    m_channel_subtype_layout->addLayout(m_subtype_layout);
    
    // RTSP URL
    m_rtsp_url_layout = new QHBoxLayout();
    m_enable_custom_rtsp_url = new QCheckBox();
    m_rtsp_rrl_label = new QLabel(tr("RTSP链接:"));
    m_rtsp_url_edit = new QLineEdit();
    m_rtsp_url_edit->setReadOnly(true);
    m_rtsp_url_layout->addWidget(m_enable_custom_rtsp_url);
    m_rtsp_url_layout->addWidget(m_rtsp_rrl_label);
    m_rtsp_url_layout->addWidget(m_rtsp_url_edit);
   
    // RTSP URL combobox
    m_rtsp_url_combo_layout = new QHBoxLayout();
    m_rtsp_url_combo_lable = new QLabel("RTSP链接: ");
    m_rtsp_url_combo = new QComboBox();
    m_refresh_rtsp_url_list = new QPushButton("刷新");

    m_rtsp_url_combo_layout->addWidget(m_refresh_rtsp_url_list, 1);
    m_rtsp_url_combo_layout->addWidget(m_rtsp_url_combo_lable, 1);
    m_rtsp_url_combo_layout->addWidget(m_rtsp_url_combo, 3);
    
    // buttons
    m_button_layout = new QHBoxLayout();
    m_test_button = new QPushButton(tr("测试连接"));
    m_save_button = new QPushButton(tr("保存配置"));
    m_close_conn_button = new QPushButton(tr("断开连接"));
    m_connect_button = new QPushButton(tr("连接"));
    m_cancel_button = new QPushButton(tr("取消"));
   
    m_close_conn_button->setEnabled(false);

    m_test_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    m_save_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_close_conn_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton)); 
    m_connect_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    m_cancel_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    
    m_button_layout->addWidget(m_test_button);
    m_button_layout->addWidget(m_save_button);
    m_button_layout->addWidget(m_close_conn_button);
    m_button_layout->addWidget(m_connect_button);
    m_button_layout->addWidget(m_cancel_button);
    
    // status label
    m_status_label = new QLabel();
    m_status_label->setWordWrap(true);
    
    QGroupBox* rtspGroup = new QGroupBox(tr("RTSP连接设置"));
    QVBoxLayout* groupLayout = new QVBoxLayout();
    groupLayout->addLayout(m_rtsp_protocal__name_layout);
    groupLayout->addLayout(m_user_pass_layout);
    groupLayout->addLayout(m_ip_port_layout);
    groupLayout->addLayout(m_channel_subtype_layout);
    groupLayout->addLayout(m_rtsp_url_layout);
    groupLayout->addLayout(m_rtsp_url_combo_layout);
    rtspGroup->setLayout(groupLayout);
    
    m_main_layout->addWidget(rtspGroup);
    m_main_layout->addLayout(m_button_layout);
    m_main_layout->addWidget(m_status_label);
    m_main_layout->addStretch();
    
    setLayout(m_main_layout); 
    update_rtsp_url();
}

void rtsp_config_window::create_connections() {
    // RTSP Protocal Type
    connect(m_rtsp_proto_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int index) {
                m_rtsp_config.rpt = static_cast<rtsp_proto_type>(
                    m_rtsp_proto_combo->itemData(index).toInt());
                qDebug() << "Protocol: " << (m_rtsp_config.rpt == rtsp_proto_type::HIKVISION ? "HIKVISION" : "ALHUA");
                update_rtsp_url();
            });

    // RTSP Stream Name
    connect ( m_rtsp_stream_name_edit
            , &QLineEdit::textChanged
            , this, [this](const QString& stream_name) {
                m_rtsp_config.rtsp_name = stream_name;
                update_rtsp_url();
            });
    
    // Username
    connect(m_username_edit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtsp_config.username = text;
                update_rtsp_url();
            });
    
    // Password
    connect(m_password_edit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtsp_config.password = text;
                update_rtsp_url();
            });
    
    // IP
    connect(m_ip_edit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtsp_config.ip = text;
                update_rtsp_url();
            });
    
    // Port
    connect(m_port_edit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtsp_config.port = text;
                update_rtsp_url();
            });
    
    // Channel
    connect(m_channel_edit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtsp_config.channel = text;
                update_rtsp_url();
            });
    
    // Subtype
    connect(m_subtype_edit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtsp_config.subtype = text;
                update_rtsp_url();
            });
    
    // Enable customized RTSP url
    connect ( m_enable_custom_rtsp_url
            , &QCheckBox::toggled
            , this, [this](bool enable_customized){ 
                if (enable_customized) m_rtsp_url_edit->setReadOnly(false);
                else m_rtsp_url_edit->setReadOnly(true);
            });
    
    // RTSP URL
    connect ( m_rtsp_url_edit
            , &QLineEdit::textChanged
            , this, [this]() {
                m_rtsp_url4conn = m_rtsp_url_edit->text();
            });

    // RTSP URL Combo
    connect ( m_rtsp_url_combo
            , QOverload<int>::of(&QComboBox::currentIndexChanged)
            , this, [this](int idx) {
                if (idx > 0) {
                    QString url = m_rtsp_url_combo->itemText(idx);
                    // update the corresponding info
                    qDebug() << "Current rtsp url: " << url; 
                    m_rtsp_url4conn = url;
                    m_rtsp_url_edit->setText(url);
                }
            });

    // Refresh RTSP URL List
    connect ( m_refresh_rtsp_url_list
            , &QPushButton::clicked
            , this, [this]() {
                m_rtsp_url_combo->clear();
                
             http_server::instance().fetch_all_rtsp_stream_info();


            });

    connect ( &http_server::instance()
            , &http_server::send_all_rtsp_stream_info
            , [this](const QVector<rtsp_config>& configs){
                m_rtsp_configs = configs;
                for (const auto& rtsp_url: m_rtsp_configs) {
                    QString url = rtsp_url.rtsp_url;
                    if (!url.isEmpty()) m_rtsp_url_combo->addItem(url);
                    qDebug() << "Existing rtsp url: " << url;
                }
            });

    connect(m_test_button, &QPushButton::clicked, this, &rtsp_config_window::on_test_connection);
    connect(m_save_button, &QPushButton::clicked, this, &rtsp_config_window::on_save_config);
    connect(m_close_conn_button, &QPushButton::clicked, this, [this](){ emit on_close_conn(); });
    connect(m_connect_button, &QPushButton::clicked, this, &rtsp_config_window::on_connect);
    connect(m_cancel_button, &QPushButton::clicked, this, &rtsp_config_window::on_cancel);
}



void rtsp_config_window::update_rtsp_url() {
    m_rtsp_url_edit->setText(m_rtsp_config.config2url_mask());
    
    
    m_rtsp_url4conn = m_rtsp_config.config2url();
}

void rtsp_config_window::on_test_connection() {
    m_status_label->setText(tr("RTSP连接测试中..."));
    QApplication::processEvents();
    
    bool success = true;
    
    if (success) {
        m_status_label->setText(tr("连接成功!"));
        m_status_label->setStyleSheet("color: green;");
    } else {
        m_status_label->setText(tr("连接失败!"));
        m_status_label->setStyleSheet("color: red;");
    }
   
    // proceed connecting
    emit send_rtsp_url(m_rtsp_url4conn, m_rtsp_config);
}

void rtsp_config_window::on_save_config() {
    save_settings();
    m_status_label->setText(tr("配置已保存。"));
    m_status_label->setStyleSheet("color: black;");
    
    emit rtsp_config_saved(m_rtsp_config);
}

void rtsp_config_window::on_connect() {
    m_status_label->setText(tr("连接至RTSP流中..."));
    QApplication::processEvents();
    
    save_settings();
    
    emit send_rtsp_url(m_rtsp_url4conn, m_rtsp_config);
    
    close();
}

bool rtsp_config_window::connect_when_startup(const QString& _rtsp_url4conn, const rtsp_config& _rtsp_config) {
    emit send_rtsp_url(_rtsp_url4conn, _rtsp_config); 
    return true;
}


void rtsp_config_window::on_cancel() {
    close();
}

void rtsp_config_window::save_settings() {
    m_settings->setValue("RTSP/Protocol", static_cast<int>(m_rtsp_config.rpt));
    m_settings->setValue("RTSP/Username", m_rtsp_config.username);
    m_settings->setValue("RTSP/Password", m_rtsp_config.password);
    m_settings->setValue("RTSP/IP", m_rtsp_config.ip);
    m_settings->setValue("RTSP/Port", m_rtsp_config.port);
    m_settings->setValue("RTSP/Channel", m_rtsp_config.channel);
    m_settings->setValue("RTSP/Subtype", m_rtsp_config.subtype);
    m_settings->setValue("RTSP/URL", m_rtsp_url4conn);
    m_settings->sync();
}

void rtsp_config_window::load_settings() {
    // load config
    int protocol = m_settings->value("RTSP/Protocol", static_cast<int>(rtsp_proto_type::HIKVISION)).toInt();
    m_rtsp_config.rpt = static_cast<rtsp_proto_type>(protocol);
    m_rtsp_config.username = m_settings->value("RTSP/Username", "").toString();
    m_rtsp_config.password = m_settings->value("RTSP/Password", "").toString();
    m_rtsp_config.ip = m_settings->value("RTSP/IP", "").toString();
    m_rtsp_config.port = m_settings->value("RTSP/Port", "554").toString();
    m_rtsp_config.channel = m_settings->value("RTSP/Channel", "101").toString();
    m_rtsp_config.subtype = m_settings->value("RTSP/Subtype", "0").toString();
     
    
    m_rtsp_proto_combo->setCurrentIndex(m_rtsp_proto_combo->findData(protocol));
    m_username_edit->setText(m_rtsp_config.username);
    m_password_edit->setText(m_rtsp_config.password);
    m_ip_edit->setText(m_rtsp_config.ip);
    m_port_edit->setText(m_rtsp_config.port);
    m_channel_edit->setText(m_rtsp_config.channel);
    m_subtype_edit->setText(m_rtsp_config.subtype);
     
    update_rtsp_url();
}

rtsp_config rtsp_config_window::get_config() const {
    return m_rtsp_config;
}

QString rtsp_config_window::get_rtsp_url() const {
    return m_rtsp_url4conn;
}

bool rtsp_config_window::has_saved_config() const {
    return m_settings->contains("RTSP/IP") && 
           m_settings->contains("RTSP/Username");
}

void rtsp_config_window::load_saved_config() {
    load_settings();
}
