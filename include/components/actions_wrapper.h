#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

class actions_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit actions_wrapper(QWidget* parent = nullptr);
    
private:
    QGroupBox* m_actions_layout;
    QHBoxLayout* m_actions_layout_wrapper;


    QHBoxLayout* m_start_inf_wrapper;
    QPushButton* m_start_inf_button;

    QHBoxLayout* m_switch_camera_stream_wrapper;
    QPushButton* m_switch_camera_stream_button;

    QHBoxLayout* m_switch_auto_crop_wrapper;
    QPushButton* m_switch_auto_crop_button;

    QHBoxLayout* m_adjust_inf_interval_wrapper;
    QLabel* m_adjust_inf_interval_label;
    QComboBox* m_adjust_inf_interval_combobox;

    QHBoxLayout* m_select_store_path_wrapper;
    QPushButton* m_select_store_path_button;

    QHBoxLayout* m_select_cropped_count_wrapper;
    QLabel* m_select_cropped_count_label;
    QComboBox* m_select_cropped_count_combobox;
    
};