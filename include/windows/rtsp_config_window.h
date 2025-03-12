#ifndef RTSP_CONFIG_WINDOW_H
#define RTSP_CONFIG_WINDOW_H

#include "components/actions_wrapper.h"
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QSettings>

enum class rtsp_proto_type {
    HIKVISION,
    ALHUA
};

struct rtsp_config {
    rtsp_proto_type rpt = rtsp_proto_type::HIKVISION;
    QString username;
    QString password;
    QString ip;
    QString port;
    QString channel = "101";
    QString subtype = "0";

    QString config2url() const {
        switch(rpt) {
            case rtsp_proto_type::HIKVISION: return rtsp_hk();
            case rtsp_proto_type::ALHUA:     return rtsp_ah();
            default: return "";
        }
    } 
private:
    QString rtsp_hk() const { 
        return "rtsp://" + username 
             + ":"       + password
             + "@"       + ip       
             + ":"       + port     
             + "/Streaming/Channels/101";
    }

    QString rtsp_ah() const {
        return "rtsp://" + username
             + ":"       + password
             + "@"       + ip
             + ":"       + port
             + "cam/realmonitor?channel=1@subtype=0";
    }
};

class rtsp_config_window: public QWidget {
    Q_OBJECT
public:
    explicit rtsp_config_window(QWidget* parent = nullptr);
    ~rtsp_config_window();

    rtsp_config getConfig() const;
    QString getRtspUrl() const;
    
    bool hasSavedConfig() const;
    
    void loadSavedConfig();

signals:
    void send_rtsp_url(const QString& rtsp_url, const rtsp_config& rtsp_cfg);
    void rtspConfigSaved(const rtsp_config& config);

private slots:
    void onTestConnection();
    void onSaveConfig();
    void onConnect();
    void onCancel();
    void updateRtspUrl();

private:
    void setupUI();
    void createConnections();
    void saveSettings();
    void loadSettings();

    // rtsp_config_layout
    QVBoxLayout* m_mainLayout;
    
    // RTSP protocal type
    QHBoxLayout* m_rtspProtoLayout;
    QLabel* m_rtspProtoLabel;
    QComboBox* m_rtspProtoCombo;
    
    // Username & Password
    QHBoxLayout* m_userPassLayout;
    
    // Username
    QHBoxLayout* m_usernameLayout;
    QLabel* m_usernameLabel;
    QLineEdit* m_usernameEdit;
    
    // Password
    QHBoxLayout* m_passwordLayout;
    QLabel* m_passwordLabel;
    QLineEdit* m_passwordEdit;
    
    // IP & Port
    QHBoxLayout* m_ipPortLayout;
    
    // IP
    QHBoxLayout* m_ipLayout;
    QLabel* m_ipLabel;
    QLineEdit* m_ipEdit;
    
    // Port
    QHBoxLayout* m_portLayout;
    QLabel* m_portLabel;
    QLineEdit* m_portEdit;
    
    // Channel and Subtype
    QHBoxLayout* m_channelSubtypeLayout;
    
    // Channel
    QHBoxLayout* m_channelLayout;
    QLabel* m_channelLabel;
    QLineEdit* m_channelEdit;
    
    // Subtype
    QHBoxLayout* m_subtypeLayout;
    QLabel* m_subtypeLabel;
    QLineEdit* m_subtypeEdit;
    
    // RTSP URL
    QHBoxLayout* m_rtspUrlLayout;
    QLabel* m_rtspUrlLabel;
    QLineEdit* m_rtspUrlEdit;
    
    // buttons layout
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_testButton;
    QPushButton* m_saveButton;
    QPushButton* m_connectButton;
    QPushButton* m_cancelButton;
    
    // Status label
    QLabel* m_statusLabel;
    
    rtsp_config m_rtspConfig;
    QSettings* m_settings;
};

#endif // RTSP_CONFIG_WINDOW_H
