
#ifndef EXPANDED_CAMERA_WINDOW_H
#define EXPANDED_CAMERA_WINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <components/camera_wrapper.h>
#include <QPushButton>

class expanded_camera_window: public QWidget {
    Q_OBJECT
public:
    explicit expanded_camera_window( camera_wrapper* cam
                                   , QWidget* parent = nullptr);
    ~expanded_camera_window() = default;
private:
    QVBoxLayout* m_expanded_camera_window_layout;
    camera_wrapper* m_camera;
    QPushButton* m_close_window_button;
    QLabel* m_title_label;

    camera_wrapper* m_org_camera;
};

#endif  // EXPANDED_CAMERA_WINDOW_H
