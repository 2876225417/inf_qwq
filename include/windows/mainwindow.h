#pragma once

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
#include <qnamespace.h>
#include <qopenglshaderprogram.h>
#include <qpushbutton.h>
#include <utils/ort_inf.h>

#include <components/cropped_wrapper.h>


#include <QHBoxLayout>

class mainwindow: public QMainWindow {
    Q_OBJECT
public:
    explicit mainwindow(QWidget* parent = nullptr);
    ~mainwindow();
signals:
    void keywords_changed(const QString& keywords);

private:
    // mainwindow layout
    QWidget*                m_mainwindow_layout;
    QHBoxLayout*            m_mainwindow_layout_wrapper;
    // camera layout
    QVBoxLayout*            m_camera_rel_layout;
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

    ort_inferer* m_inferer;
    chars_det_inferer* m_chars_inferer;
    cv::Mat for2{};
    chars_inferer* m_chars_rec_inferer;
    tool_bar*               m_tool_bar;
    status_bar*             m_status_bar;
};
