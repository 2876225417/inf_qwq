#include <components/actions_wrapper.h>


actions_wrapper::actions_wrapper(QWidget* parent)
    : QWidget{parent}
    , m_actions_layout{new QGroupBox(this)}
    , m_actions_layout_wrapper{new QHBoxLayout(m_actions_layout)}
    // start_inf
    , m_start_inf_wrapper{new QHBoxLayout()}
    // switch_camera_stream
    , m_switch_camera_stream_wrapper{new QHBoxLayout()}
    // select_cropped_count
    , m_select_cropped_count_wrapper{new QHBoxLayout()}
    // adjust_inf_interval
    , m_adjust_inf_interval_wrapper{new QHBoxLayout()}
    // select_store_path
    , m_select_store_path_wrapper{new QHBoxLayout()}
    // auto_cropped
    , m_switch_auto_crop_wrapper{new QHBoxLayout()}
    {
    m_start_inf_button = new QPushButton("Inf");
    m_start_inf_wrapper->addWidget(m_start_inf_button);

    m_switch_camera_stream_button = new QPushButton("Pause");
    m_switch_camera_stream_wrapper->addWidget(m_switch_camera_stream_button);

    m_select_cropped_count_label = new QLabel("Cropped Count");
    m_select_cropped_count_combobox = new QComboBox();
    m_select_cropped_count_combobox->addItem("2");
    m_select_cropped_count_combobox->addItem("4");
    m_select_cropped_count_combobox->addItem("6");
    m_select_cropped_count_wrapper->addWidget(m_select_cropped_count_label);
    m_select_cropped_count_wrapper->addWidget(m_select_cropped_count_combobox);

    m_adjust_inf_interval_label = new QLabel("Inf Interval");
    m_adjust_inf_interval_combobox = new QComboBox();
    m_adjust_inf_interval_combobox->addItem("5");
    m_adjust_inf_interval_combobox->addItem("15");
    m_adjust_inf_interval_combobox->addItem("25");
    m_adjust_inf_interval_wrapper->addWidget(m_adjust_inf_interval_label);
    m_adjust_inf_interval_wrapper->addWidget(m_adjust_inf_interval_combobox);

    m_select_store_path_button = new QPushButton("Store Path");
    m_select_store_path_wrapper->addWidget(m_select_store_path_button);

    m_switch_auto_crop_button = new QPushButton("Auto Crop");
    m_switch_auto_crop_wrapper->addWidget(m_switch_auto_crop_button);

    m_actions_layout_wrapper->addLayout(m_start_inf_wrapper);
    m_actions_layout_wrapper->addLayout(m_switch_camera_stream_wrapper);
    m_actions_layout_wrapper->addLayout(m_select_cropped_count_wrapper);
    m_actions_layout_wrapper->addLayout(m_adjust_inf_interval_wrapper);
    m_actions_layout_wrapper->addLayout(m_select_store_path_wrapper);
    m_actions_layout_wrapper->addLayout(m_switch_auto_crop_wrapper);

}