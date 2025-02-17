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
#include <QHBoxLayout>

class mainwindow: public QMainWindow {
    Q_OBJECT
public:
    explicit mainwindow(QWidget* parent = nullptr);
    ~mainwindow();
    QWidget* m_mainwindow_layout;
    QHBoxLayout* m_mainwindow_layout_wrapper;
    QGroupBox* m_mainwindow_camera_layout;
    QHBoxLayout* m_mainwindow_camera_layout_wrapper;
private:

    camera_wrapper* m_camera;
};