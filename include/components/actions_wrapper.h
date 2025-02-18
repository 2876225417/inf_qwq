#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>


class actions_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit actions_wrapper(QWidget* parent = nullptr);
    
private:
    QGroupBox* m_actions_layout;
    QHBoxLayout* m_actions_layout_wrapper;

    QVBoxLayout* m_col_1_layout_wrapper;
    QHBoxLayout* m_start_inf_wrapper;
    QPushButton* m_start_inf_button;

    QHBoxLayout* m_switch_camera_stream_wrapper;
    QPushButton* m_switch_camera_stream_button;

    QHBoxLayout* m_switch_auto_crop_wrapper;
    QPushButton* m_switch_auto_crop_button;

    QVBoxLayout* m_col_2_layout_wrapper;
    QHBoxLayout* m_adjust_inf_interval_wrapper;
    QLabel* m_adjust_inf_interval_label;
    QComboBox* m_adjust_inf_interval_combobox;

    QHBoxLayout* m_select_store_path_wrapper;
    QPushButton* m_select_store_path_button;

    QHBoxLayout* m_select_cropped_count_wrapper;
    QLabel* m_select_cropped_count_label;
    QComboBox* m_select_cropped_count_combobox;

    QVBoxLayout* m_col_3_layout_wrapper;
    QHBoxLayout* m_select_camera_index_wrapper;
    QLabel* m_select_camera_index_label;
    QComboBox* m_select_camera_index_combobox;

    QHBoxLayout* m_switch_multi_cameras_wrapper;
    QLabel* m_switch_multi_cameras_label;
    QCheckBox* m_switch_multi_cameras_checkbox;   
    
    QHBoxLayout* m_clear_all_cropped_wrapper;
    QPushButton* m_clear_all_cropped_button;
};