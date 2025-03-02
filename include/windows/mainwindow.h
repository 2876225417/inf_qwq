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
#include <utils/ort_inf.h>

#include <components/cropped_wrapper.h>


#include <QHBoxLayout>

class mainwindow: public QMainWindow {
    Q_OBJECT
public:
    explicit mainwindow(QWidget* parent = nullptr);
    ~mainwindow();
    QWidget*                m_mainwindow_layout;
    QHBoxLayout*            m_mainwindow_layout_wrapper;

    QVBoxLayout*            m_camera_rel_layout;
    QGroupBox*              m_mainwindow_camera_layout;
    QHBoxLayout*            m_mainwindow_camera_layout_wrapper;

    actions_wrapper*        m_actions_wrapper;

    QGroupBox*              m_camera_cropped_layout;
    QVBoxLayout*            m_camera_cropped_layout_wrapper;
private:

    camera_wrapper*         m_camera;
    camera_config_wrapper*  m_camera_config;

    QHBoxLayout*            m_cropped_img_1_and_2_wrapper;
    cropped_img_wrapper*    m_cropped_img_1;
    cropped_img_wrapper*    m_cropped_img_2;
    QHBoxLayout*            m_cropped_img_3_and_4_wrapper;
    cropped_img_wrapper*    m_cropped_img_3;
    cropped_img_wrapper*    m_cropped_img_4;

    cropped_wrapper<4>*     m_4_croppeds_img;

    QImage tmp;
    QPushButton* test_inf = new QPushButton("test_inf");
    
    cv::Mat qimage2mat(QImage& qimage);

    ort_inferer* inferer = new ort_inferer();

    tool_bar*               m_tool_bar;
    status_bar*             m_status_bar;
};
