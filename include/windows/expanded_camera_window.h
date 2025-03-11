
#ifndef EXPANDED_CAMERA_WINDOW_H
#define EXPANDED_CAMERA_WINDOW_H

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
    QFormLayout* m_settings_layout;
    QComboBox* m_detection_mode;
    QCheckBox* m_enable_tracking;
    QSpinBox* m_confidence_threshold;
    QPushButton* m_apply_settings;
    QPushButton* m_close_window_button;

    camera_wrapper* m_org_camera;

    void setup_UI();
    void create_left_panel();
    void create_right_panel();
    void create_right_top_panel();
    void create_right_bottom_panel();
    void connect_signals();
};

#endif  // EXPANDED_CAMERA_WINDOW_H
