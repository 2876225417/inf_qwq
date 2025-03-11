
#ifndef EXPANDED_CAMERA_WINDOW_H
#define EXPANDED_CAMERA_WINDOW_H

#include "windows/rtsp_config_window.h"
#include <QWidget>
#include <QVBoxLayout>
#include <components/camera_wrapper.h>
#include <QPushButton>
#include <qboxlayout.h>
#include <QSplitter>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qformlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qwidget.h>

#include <QTextEdit>

class expanded_camera_window: public QWidget {
    Q_OBJECT
public:
    explicit expanded_camera_window( camera_wrapper* cam
                                   , QWidget* parent = nullptr);
    ~expanded_camera_window() = default;

    void set_cropped_image(const QImage& image);
    void set_rstp_info(const rtsp_config& rstp_cfg);
    void set_inf_result(const QString& inf_res);
    QString parse_rtsp_protocal_type();
private:
    QVBoxLayout* m_expanded_camera_window_layout;
    QHBoxLayout* m_content_layout;
    QSplitter*   m_content_splitter;

    // left -> camera area
    QWidget* m_left_widget;
    QVBoxLayout* m_left_layout;
    camera_wrapper* m_camera;
    QLabel* m_title_label;

    // right -> exhibition area
    QWidget* m_right_widget;
    QVBoxLayout* m_right_layout;
    QSplitter* m_right_splitter;

    // right -> top 
    QWidget* m_right_top_widget;
    QHBoxLayout* m_right_top_layout;
    QSplitter* m_right_top_splitter;

    // right -> top -> right -> rec image
    QWidget* m_image_widget;
    QVBoxLayout* m_image_layout;
    QLabel* m_image_label;
    QLabel* m_image_display;

    // right -> top -> left  -> rec text
    QWidget* m_text_widget;
    QVBoxLayout* m_text_layout;
    QLabel* m_text_label;
    QTextEdit* m_text_display;

    // right -> bottom
    QWidget* m_settings_group;
    QHBoxLayout* m_settings_layout;
    QFormLayout* m_rtsp_info_layout;
    QFormLayout* m_inf_config_layout;
    
    // right -> bottom -> rtsp connect info
    QLabel* m_rtsp_proto_type;
    QLabel* m_rtsp_username;
    QLabel* m_rtsp_ip;
    QLabel* m_rtsp_port;
    QLabel* m_rtsp_channel;
    QLabel* m_rtsp_subtype;

    camera_wrapper* m_org_camera;
    rtsp_config m_rtsp_config;


    void setup_UI();
    void create_left_panel();
    void create_right_panel();
    void create_right_top_panel();
    void create_right_bottom_panel();
    void connect_signals();
};

#endif  // EXPANDED_CAMERA_WINDOW_H
