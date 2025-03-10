

#ifndef RTSP_CONFIG_WINDOW_H
#define RTSP_CONFIG_WINDOW_H

#include "components/actions_wrapper.h"
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <qboxlayout.h>
#include <qcombobox.h>

#include <QLineEdit>
#include <qlineedit.h>
#include <QVBoxLayout>
#include <qpushbutton.h>

enum class rtsp_proto_type {
    HIKVISION,
    ALHUA
};

class rtsp_config_window: public QWidget {
    Q_OBJECT
public:
    explicit rtsp_config_window(QWidget* parent = nullptr);

signals:
    


private:
    QVBoxLayout* m_rtsp_config_main_layout; 
    
    // rtsp protocals
    QHBoxLayout* m_rtsp_protocal_type_wrapper;
    QLabel* m_rtsp_protocal_type_label;
    QComboBox* m_rtsp_protocal_type_select;
    
    // username & password wrapper
    QHBoxLayout* m_username__password_wrapper;
    // username
    QHBoxLayout* m_rtsp_username_wrapper;
    QLabel* m_rtsp_username_label;
    QLineEdit* m_rtsp_username_edit;

    // password
    QHBoxLayout* m_rtsp_password_wrapper;
    QLabel* m_rtsp_password_label;
    QLineEdit* m_rtsp_password_edit;
    
    // ip & port wrapper
    QHBoxLayout* m_ip__port_wrapper;
    // ip 
    QHBoxLayout* m_rtsp_ip_wrapper;
    QLabel* m_rtsp_ip_label;
    QLineEdit* m_rtsp_ip_edit;
    
    // port
    QHBoxLayout* m_rtsp_port_wrapper;
    QLabel* m_rtsp_port_label;
    QLineEdit* m_rtsp_port_edit;
    
    // channel & subtype wrapper
    QHBoxLayout* m_channel__subtype_wrapper;
    // channel 
    QHBoxLayout* m_rtsp_channel_wrapper;
    QLabel* m_rtsp_channel_label;
    QLineEdit* m_rtsp_channel_edit;
    
    // subtype
    QHBoxLayout* m_rtsp_subtype_wrapper;
    QLabel* m_rtsp_subtype_label;
    QLineEdit* m_rtsp_subtype_edit;

    // customized rtsp info
    QHBoxLayout* m_customized_rtsp_info_wrapper;
    QLabel* m_customized_rtsp_info_label;
    QLineEdit* m_customized_rtsp_info_edit;

    // test & connect
    QHBoxLayout* m_test__connect_wrapper;
    // test connect
    QPushButton* m_test_button;
    // connect
    QPushButton* m_connect_button;

    struct {
        rtsp_proto_type rpt = rtsp_proto_type::HIKVISION;
        QString username;
        QString password;
        QString ip;
        QString port;
        QString channel;
        QString subtype;

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
    } m_rtsp_config;
};
#endif
