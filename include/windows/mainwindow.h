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

    QHBoxLayout* mainwindow_layout;
private:

    camera_wrapper* m_camera;
};