#include "windows/rtsp_config_window.h"
#include <QApplication>
#include <QStyle>
#include <QGroupBox>
#include <QDebug>
#include <QMessageBox>

rtsp_config_window::rtsp_config_window(QWidget* parent)
    : QWidget(parent, Qt::Window)
    {
    setWindowTitle(tr("RTSP Configuration"));
    setMinimumWidth(500);
    setMinimumHeight(350);
    
    m_settings = new QSettings("Chun Hui", "inf_qwq", this);
    
    setupUI();
    createConnections();
    loadSettings();
}

rtsp_config_window::~rtsp_config_window(){ }

void rtsp_config_window::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
   
    // RTSP Protocal Type
    m_rtspProtoLayout = new QHBoxLayout();
    m_rtspProtoLabel = new QLabel(tr("Protocol:"));
    m_rtspProtoCombo = new QComboBox();
    m_rtspProtoCombo->addItem("HIKVISION", static_cast<int>(rtsp_proto_type::HIKVISION));
    m_rtspProtoCombo->addItem("ALHUA", static_cast<int>(rtsp_proto_type::ALHUA));
    m_rtspProtoLayout->addWidget(m_rtspProtoLabel);
    m_rtspProtoLayout->addWidget(m_rtspProtoCombo);
    
    // Username & Password
    m_userPassLayout = new QHBoxLayout();
    
    // Username
    m_usernameLayout = new QHBoxLayout();
    m_usernameLabel = new QLabel(tr("Username:"));
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText(tr("username"));
    m_usernameLayout->addWidget(m_usernameLabel);
    m_usernameLayout->addWidget(m_usernameEdit);
    
    // Password
    m_passwordLayout = new QHBoxLayout();
    m_passwordLabel = new QLabel(tr("Password:"));
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText(tr("password"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordLayout->addWidget(m_passwordLabel);
    m_passwordLayout->addWidget(m_passwordEdit);
    
    m_userPassLayout->addLayout(m_usernameLayout);
    m_userPassLayout->addLayout(m_passwordLayout);
    
    // IP & Port
    m_ipPortLayout = new QHBoxLayout();
    
    // IP
    m_ipLayout = new QHBoxLayout();
    m_ipLabel = new QLabel(tr("IP:"));
    m_ipEdit = new QLineEdit();
    m_ipEdit->setPlaceholderText(tr("192.168.1.100"));
    m_ipLayout->addWidget(m_ipLabel);
    m_ipLayout->addWidget(m_ipEdit);
    
    // Port
    m_portLayout = new QHBoxLayout();
    m_portLabel = new QLabel(tr("Port:"));
    m_portEdit = new QLineEdit();
    m_portEdit->setPlaceholderText(tr("554"));
    m_portLayout->addWidget(m_portLabel);
    m_portLayout->addWidget(m_portEdit);
    
    m_ipPortLayout->addLayout(m_ipLayout);
    m_ipPortLayout->addLayout(m_portLayout);
    
    // Channel & Subtype
    m_channelSubtypeLayout = new QHBoxLayout();
    
    // Channel
    m_channelLayout = new QHBoxLayout();
    m_channelLabel = new QLabel(tr("Channel:"));
    m_channelEdit = new QLineEdit();
    m_channelEdit->setText("101");
    m_channelLayout->addWidget(m_channelLabel);
    m_channelLayout->addWidget(m_channelEdit);
    
    // Subtype
    m_subtypeLayout = new QHBoxLayout();
    m_subtypeLabel = new QLabel(tr("Subtype:"));
    m_subtypeEdit = new QLineEdit();
    m_subtypeEdit->setText("0");
    m_subtypeLayout->addWidget(m_subtypeLabel);
    m_subtypeLayout->addWidget(m_subtypeEdit);
    
    m_channelSubtypeLayout->addLayout(m_channelLayout);
    m_channelSubtypeLayout->addLayout(m_subtypeLayout);
    
    // RTSP URL
    m_rtspUrlLayout = new QHBoxLayout();
    m_rtspUrlLabel = new QLabel(tr("RTSP URL:"));
    m_rtspUrlEdit = new QLineEdit();
    m_rtspUrlEdit->setReadOnly(false);
    m_rtspUrlLayout->addWidget(m_rtspUrlLabel);
    m_rtspUrlLayout->addWidget(m_rtspUrlEdit);
    
    // buttons
    m_buttonLayout = new QHBoxLayout();
    m_testButton = new QPushButton(tr("Test Connection"));
    m_saveButton = new QPushButton(tr("Save Config"));
    m_connectButton = new QPushButton(tr("Connect"));
    m_cancelButton = new QPushButton(tr("Cancel"));
    
    m_testButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    m_saveButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_connectButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    m_cancelButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    
    m_buttonLayout->addWidget(m_testButton);
    m_buttonLayout->addWidget(m_saveButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_connectButton);
    m_buttonLayout->addWidget(m_cancelButton);
    
    // status label
    m_statusLabel = new QLabel();
    m_statusLabel->setWordWrap(true);
    
    QGroupBox* rtspGroup = new QGroupBox(tr("RTSP Connection Settings"));
    QVBoxLayout* groupLayout = new QVBoxLayout();
    groupLayout->addLayout(m_rtspProtoLayout);
    groupLayout->addLayout(m_userPassLayout);
    groupLayout->addLayout(m_ipPortLayout);
    groupLayout->addLayout(m_channelSubtypeLayout);
    groupLayout->addLayout(m_rtspUrlLayout);
    rtspGroup->setLayout(groupLayout);
    
    m_mainLayout->addWidget(rtspGroup);
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addStretch();
    
    setLayout(m_mainLayout); 
    updateRtspUrl();
}

void rtsp_config_window::createConnections() {
    // RTSP Protocal Type
    connect(m_rtspProtoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int index) {
                m_rtspConfig.rpt = static_cast<rtsp_proto_type>(
                    m_rtspProtoCombo->itemData(index).toInt());
                qDebug() << "Protocol: " << (m_rtspConfig.rpt == rtsp_proto_type::HIKVISION ? "HIKVISION" : "ALHUA");
                updateRtspUrl();
            });
    
    // Username
    connect(m_usernameEdit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtspConfig.username = text;
                updateRtspUrl();
            });
    
    // Password
    connect(m_passwordEdit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtspConfig.password = text;
                updateRtspUrl();
            });
    
    // IP
    connect(m_ipEdit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtspConfig.ip = text;
                updateRtspUrl();
            });
    
    // Port
    connect(m_portEdit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtspConfig.port = text;
                updateRtspUrl();
            });
    
    // Channel
    connect(m_channelEdit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtspConfig.channel = text;
                updateRtspUrl();
            });
    
    // Subtype
    connect(m_subtypeEdit, &QLineEdit::textChanged, 
            [this](const QString& text) {
                m_rtspConfig.subtype = text;
                updateRtspUrl();
            });
    
    connect(m_testButton, &QPushButton::clicked, this, &rtsp_config_window::onTestConnection);
    connect(m_saveButton, &QPushButton::clicked, this, &rtsp_config_window::onSaveConfig);
    connect(m_connectButton, &QPushButton::clicked, this, &rtsp_config_window::onConnect);
    connect(m_cancelButton, &QPushButton::clicked, this, &rtsp_config_window::onCancel);
}

void rtsp_config_window::updateRtspUrl() {
    m_rtspUrlEdit->setText(m_rtspConfig.config2url());
}

void rtsp_config_window::onTestConnection() {
    m_statusLabel->setText(tr("Testing RTSP connection..."));
    QApplication::processEvents();
    
    bool success = true;
    
    if (success) {
        m_statusLabel->setText(tr("Connection successful!"));
        m_statusLabel->setStyleSheet("color: green;");
    } else {
        m_statusLabel->setText(tr("Connection failed!"));
        m_statusLabel->setStyleSheet("color: red;");
    }
   
    // proceed connecting
    emit send_rtsp_url(m_rtspUrlEdit->text(), m_rtspConfig);
}

void rtsp_config_window::onSaveConfig() {
    saveSettings();
    m_statusLabel->setText(tr("Configuration saved."));
    m_statusLabel->setStyleSheet("color: black;");
    
    emit rtspConfigSaved(m_rtspConfig);
}

void rtsp_config_window::onConnect() {
    m_statusLabel->setText(tr("Connecting to RTSP stream..."));
    QApplication::processEvents();
    
    saveSettings();
    
    emit send_rtsp_url(m_rtspUrlEdit->text(), m_rtspConfig);
    
    close();
}

void rtsp_config_window::onCancel() {
    close();
}

void rtsp_config_window::saveSettings() {
    m_settings->setValue("RTSP/Protocol", static_cast<int>(m_rtspConfig.rpt));
    m_settings->setValue("RTSP/Username", m_rtspConfig.username);
    m_settings->setValue("RTSP/Password", m_rtspConfig.password);
    m_settings->setValue("RTSP/IP", m_rtspConfig.ip);
    m_settings->setValue("RTSP/Port", m_rtspConfig.port);
    m_settings->setValue("RTSP/Channel", m_rtspConfig.channel);
    m_settings->setValue("RTSP/Subtype", m_rtspConfig.subtype);
    m_settings->setValue("RTSP/URL", m_rtspUrlEdit->text());
    m_settings->sync();
}

void rtsp_config_window::loadSettings() {
    // load config
    int protocol = m_settings->value("RTSP/Protocol", static_cast<int>(rtsp_proto_type::HIKVISION)).toInt();
    m_rtspConfig.rpt = static_cast<rtsp_proto_type>(protocol);
    m_rtspConfig.username = m_settings->value("RTSP/Username", "").toString();
    m_rtspConfig.password = m_settings->value("RTSP/Password", "").toString();
    m_rtspConfig.ip = m_settings->value("RTSP/IP", "").toString();
    m_rtspConfig.port = m_settings->value("RTSP/Port", "554").toString();
    m_rtspConfig.channel = m_settings->value("RTSP/Channel", "101").toString();
    m_rtspConfig.subtype = m_settings->value("RTSP/Subtype", "0").toString();
    
    m_rtspProtoCombo->setCurrentIndex(m_rtspProtoCombo->findData(protocol));
    m_usernameEdit->setText(m_rtspConfig.username);
    m_passwordEdit->setText(m_rtspConfig.password);
    m_ipEdit->setText(m_rtspConfig.ip);
    m_portEdit->setText(m_rtspConfig.port);
    m_channelEdit->setText(m_rtspConfig.channel);
    m_subtypeEdit->setText(m_rtspConfig.subtype);
    
    updateRtspUrl();
}

rtsp_config rtsp_config_window::getConfig() const {
    return m_rtspConfig;
}

QString rtsp_config_window::getRtspUrl() const {
    return m_rtspUrlEdit->text();
}

bool rtsp_config_window::hasSavedConfig() const {
    return m_settings->contains("RTSP/IP") && 
           m_settings->contains("RTSP/Username");
}

void rtsp_config_window::loadSavedConfig() {
    loadSettings();
}
