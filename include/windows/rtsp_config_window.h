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
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <utils/common.h>
#include <utils/http_server.h>

class rtsp_config_window: public QWidget {
    Q_OBJECT
public:
    explicit rtsp_config_window(QWidget* parent = nullptr);
    ~rtsp_config_window();

    rtsp_config get_config() const;
    QString get_rtsp_url() const;
    
    bool has_saved_config() const;
    
    void load_saved_config();
signals:
    void send_rtsp_url(const QString& rtsp_url, const rtsp_config& rtsp_cfg);
    void rtsp_config_saved(const rtsp_config& config);

    void on_close_conn(); 

private slots:
    void on_test_connection();
    void on_save_config();
    void on_connect();
    void on_cancel();
    void update_rtsp_url();
public:
    void setup_UI();
    void create_connections();
    void save_settings();
    void load_settings();

    // rtsp_config_layout
    QVBoxLayout* m_main_layout;
   
    // RTSP Protocal Type & RTSP Stream Name
    QHBoxLayout* m_rtsp_protocal__name_layout;

    // RTSP protocal type
    QHBoxLayout* m_rtsp_proto_layout;
    QLabel* m_rtsp_proto_label;
    QComboBox* m_rtsp_proto_combo;
   
    QHBoxLayout* m_rtsp_stream_name_layout;
    QLabel* m_rtsp_stream_name_label;
    QLineEdit* m_rtsp_stream_name_edit;


    // Username & Password
    QHBoxLayout* m_user_pass_layout;
    
    // Username
    QHBoxLayout* m_username_layout;
    QLabel* m_username_label;
    QLineEdit* m_username_edit;
    
    // Password
    QHBoxLayout* m_password_layout;
    QLabel* m_password_label;
    QLineEdit* m_password_edit;
    
    // IP & Port
    QHBoxLayout* m_ip_port_layout;
    
    // IP
    QHBoxLayout* m_ip_layout;
    QLabel* m_ip_label;
    QLineEdit* m_ip_edit;
    
    // Port
    QHBoxLayout* m_port_layout;
    QLabel* m_port_label;
    QLineEdit* m_port_edit;
    
    // Channel and Subtype
    QHBoxLayout* m_channel_subtype_layout;
    
    // Channel
    QHBoxLayout* m_channel_layout;
    QLabel* m_channel_label;
    QLineEdit* m_channel_edit;
    
    // Subtype
    QHBoxLayout* m_subtype_layout;
    QLabel* m_subtype_label;
    QLineEdit* m_subtype_edit;
    
    // RTSP URL LineEdit
    QHBoxLayout* m_rtsp_url_layout;
    QCheckBox* m_enable_custom_rtsp_url;
    QLabel* m_rtsp_rrl_label;
    QLineEdit* m_rtsp_url_edit;
    
    // RTSP URL Combobox
    QHBoxLayout* m_rtsp_url_combo_layout;
    QLabel* m_rtsp_url_combo_lable;
    QComboBox* m_rtsp_url_combo;
    QPushButton* m_refresh_rtsp_url_list;

    // buttons layout
    QHBoxLayout* m_button_layout;
    QPushButton* m_test_button;
    QPushButton* m_save_button;
    QPushButton* m_connect_button;
    QPushButton* m_cancel_button;
    QPushButton* m_close_conn_button;

    // Status label
    QLabel* m_status_label;
    

    QString m_rtsp_url4conn;
    rtsp_config m_rtsp_config;
    QSettings* m_settings;

    http_server* m_http_server;
    QVector<rtsp_config> m_rtsp_configs;
};

#endif // RTSP_CONFIG_WINDOW_H
