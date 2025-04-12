#pragma once

#include "utils/common.h"
#include <components/grouping_sidebar.h>
#include <utils/chars_ort_inferer.h>
#include <windows/rtsp_config_window.h>
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
#include <windows/expanded_camera_window.h>

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>

#include <QHBoxLayout>

#include <QObject>

#include <utils/http_server.h>

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
    
    tool_bar*               m_tool_bar;
    status_bar*             m_status_bar;

    // sidebar
    grouping_sidebar* m_sidebar; 

    // rtsp stream group
    QStackedWidget* m_stream_group;  

    static inline int cam_nums = 0;
   
    chars_ort_inferer* m_chars_ort_inferer;
    
    std::tuple<camera_wrapper*, bool>
    add_rtsp_stream(const QString& rtsp_url, const rtsp_config& config);

    void setup_camera_connections(camera_wrapper* cam, const rtsp_config& config);

    void update_sidebar();
    void connect_saved_rtsp_streams();


    // todo struct expanded map data 
    QMap<int, rtsp_config> m_expands_window2rtsp_config; 
    QMap<int, QString> m_expanded_window2_inf_res;
    QMap<int, QImage> m_expanded_window2_inf_cropped;
    QMap<int, QString> m_expanded_window2_keywords;
    
    
    void setup_UI();
    void setup_connections();
    QVector<rtsp_config> m_rtsp_configs;

    bool m_is_first_conn_when_startup = false;

};


