#pragma once


#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>

class camera_config_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit camera_config_wrapper(QWidget* parent = nullptr);
    ~camera_config_wrapper() = default;
private:
    QGroupBox* m_camera_config_layout;
    QVBoxLayout* m_camera_config_layout_wrapper;
};