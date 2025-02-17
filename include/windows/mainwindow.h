#pragma once

#include <QMainWindow>
#include <QCamera>
#include <QVideoWidget>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

#include <components/camera_wrapper.h>
#include <components/camera_config_wrapper.h>
#include <components/tool_bar.h>
#include <components/status_bar.h>
#include <components/cropped_img_wrapper.h>
#include <components/actions_wrapper.h>

#include <QHBoxLayout>

class mainwindow: public QMainWindow {
    Q_OBJECT
public:
    explicit mainwindow(QWidget* parent = nullptr);
    ~mainwindow();
    QWidget* m_mainwindow_layout;
    QHBoxLayout* m_mainwindow_layout_wrapper;

    QVBoxLayout* m_camera_rel_layout;
    QGroupBox* m_mainwindow_camera_layout;
    QHBoxLayout* m_mainwindow_camera_layout_wrapper;

    actions_wrapper* m_actions_wrapper;

    QGroupBox* m_camera_cropped_layout;
    QHBoxLayout* m_camera_cropped_layout_wrapper;
private:

    camera_wrapper* m_camera;
    camera_config_wrapper* m_camera_config;

    cropped_img_wrapper* m_cropped_img;

    tool_bar* m_tool_bar;
    status_bar* m_status_bar;
};