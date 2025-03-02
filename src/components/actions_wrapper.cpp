#include <components/actions_wrapper.h>


actions_wrapper::actions_wrapper(QWidget* parent)
    : QWidget{parent}
    , m_ort_inferer{ std::make_unique<ort_inferer>() }
    , m_actions_layout{new QGroupBox(this)}
    , m_actions_layout_wrapper{new QHBoxLayout(m_actions_layout)}
    , m_col_1_layout_wrapper{new QVBoxLayout()}
    , m_col_2_layout_wrapper{new QVBoxLayout()}
    , m_col_3_layout_wrapper{new QVBoxLayout()}
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
    // select_camera
    , m_select_camera_index_wrapper{new QHBoxLayout()}
    // switch_multi_cameras
    , m_switch_multi_cameras_wrapper{new QHBoxLayout()}
    // clear cropped
    , m_clear_all_cropped_wrapper{new QHBoxLayout()}
    {
    m_start_inf_button = new QPushButton("Inf");
    m_start_inf_wrapper->addWidget(m_start_inf_button);

    connect( m_start_inf_button
           , &QPushButton::clicked
           , this, []() {
                qDebug() << "Button clicked!\n"; 
           });


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

    m_select_camera_index_label = new QLabel("Camera");
    m_select_camera_index_combobox = new QComboBox();
    m_select_camera_index_wrapper->addWidget(m_select_camera_index_label);
    m_select_camera_index_wrapper->addWidget(m_select_camera_index_combobox);

    m_switch_multi_cameras_label = new QLabel("Multi Cameras");
    m_switch_multi_cameras_checkbox = new QCheckBox();
    m_switch_multi_cameras_wrapper->addWidget(m_switch_multi_cameras_label);
    m_switch_multi_cameras_wrapper->addWidget(m_switch_multi_cameras_checkbox);

    m_clear_all_cropped_button = new QPushButton("Clear Crops");
    m_clear_all_cropped_wrapper->addWidget(m_clear_all_cropped_button);

    m_col_1_layout_wrapper->addLayout(m_start_inf_wrapper);
    m_col_1_layout_wrapper->addLayout(m_switch_camera_stream_wrapper);
    m_col_1_layout_wrapper->addLayout(m_select_cropped_count_wrapper);

    m_col_2_layout_wrapper->addLayout(m_adjust_inf_interval_wrapper);
    m_col_2_layout_wrapper->addLayout(m_select_store_path_wrapper);
    m_col_2_layout_wrapper->addLayout(m_switch_auto_crop_wrapper);

    m_col_3_layout_wrapper->addLayout(m_select_camera_index_wrapper);
    m_col_3_layout_wrapper->addLayout(m_switch_multi_cameras_wrapper);
    m_col_3_layout_wrapper->addLayout(m_clear_all_cropped_wrapper);


    m_actions_layout_wrapper->addLayout(m_col_1_layout_wrapper);
    m_actions_layout_wrapper->addLayout(m_col_2_layout_wrapper);
    m_actions_layout_wrapper->addLayout(m_col_3_layout_wrapper);

}
