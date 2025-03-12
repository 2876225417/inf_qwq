#pragma once

#include "components/grouping_sidebar.h"
#include "utils/chars_ort_inferer.h"
#include "windows/rtsp_config_window.h"
#include <GL/gl.h>
#include <QMainWindow>
#include <QCamera>
#include <QVideoWidget>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

#include <algorithm>
#include <components/camera_wrapper.h>
#include <components/camera_config_wrapper.h>
#include <components/tool_bar.h>
#include <components/status_bar.h>
#include <components/cropped_img_wrapper.h>
#include <components/actions_wrapper.h>

#include <qevent.h>
#include <qgridlayout.h>
#include <qnamespace.h>
#include <qopenglshaderprogram.h>
#include <qpushbutton.h>
#include <qstackedwidget.h>
#include <utils/ort_inf.h>

#include <utils/ort_inf.hpp>

#include <components/cropped_wrapper.h>


#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>

#include <QHBoxLayout>

#include <QObject>

#include <windows/expanded_camera_window.h>

class mainwindow: public QMainWindow {
    Q_OBJECT
public:
    explicit mainwindow(QWidget* parent = nullptr);
    ~mainwindow();
signals:
    void keywords_changed(const QString& keywords);
    void conn_cnt_changed(int cnt); 
private:
    // mainwindow layout
    QWidget*                m_mainwindow_layout;
    QHBoxLayout*            m_mainwindow_layout_wrapper;
    // camera layout
    QVBoxLayout*            m_camera_rel_layout;
    QGridLayout*            m_camera_rel_layout_grid;
    QGroupBox*              m_mainwindow_camera_layout;
    QHBoxLayout*            m_mainwindow_camera_layout_wrapper;
    // actions relatives 
    actions_wrapper*        m_actions_wrapper;
    // camera croppeds
    QGroupBox*              m_camera_cropped_layout;
    QVBoxLayout*            m_camera_cropped_layout_wrapper;
    // camera capturer
    camera_wrapper*         m_camera;
    camera_config_wrapper*  m_camera_config;

    // croppeds wrapper
    QHBoxLayout*            m_cropped_img_wrapper;
    // camera croppeds layout
    cropped_wrapper<4>*     m_4_croppeds_img;
    cropped_wrapper<1>*     m_1_cropped_img;

    QImage tmp;
    QPushButton* test_inf = new QPushButton("test_inf");

    QPushButton* test_class = new QPushButton("test_class");

    cv::Mat qimage2mat(QImage& qimage);

    struct {
        QString username;
        QString password;
        QString ip;
        QString port;
        
        rtsp_protocal_type rtp;
        
        QString comb_rtsp_url() {
            switch(rtp) {
                case rtsp_protocal_type::HIKVISION: return comb_hk();
                case rtsp_protocal_type::ALHUA:     return comb_ah();
                default: return "";
            }
        }
    private:
        QString comb_hk() {
            return "rtsp://" + username + ":" + password + "@" + ip + ":" + port + "/Streaming/Channels/101";
        }
        QString comb_ah() {
            return "rtsp://" + username + ":" + password + "@" + ip + ":" + port + "/cam/realmonitor?channel=1&subtype=0";
        }
    } m_config;

   

    //ort_inferer* m_inferer;
    cv::Mat for2{};
    det_inferer* m_chars_det_inferer;
    rec_inferer* m_chars_rec_inferer;
    tool_bar*               m_tool_bar;
    status_bar*             m_status_bar;

    // sidebar
    grouping_sidebar* m_sidebar; 

    // rtsp stream group
    QStackedWidget* m_stream_group;  

    static int cam_nums;
   
    chars_ort_inferer* m_chars_ort_inferer;
    
    // todo struct expanded map data 
    // @ rstp_config
    // @ inf result
    // @ inf cropped
    // @ keywords
    QMap<int, expanded_camera_window*> m_expanded_windows;
    QMap<int, rtsp_config> m_expands_window2rtsp_config; 
    QMap<int, QString> m_expanded_window2_inf_res;
    QMap<int, QImage> m_expanded_window2_inf_cropped;
    QMap<int, QString> m_expanded_window2_keywords;
};

inline int mainwindow::cam_nums = 0;

