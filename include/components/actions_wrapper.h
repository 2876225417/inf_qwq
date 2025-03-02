#pragma once

#include <array>
#include <stdexcept>
#include <string_view>

enum class control_type { 
    button,
    combobox,
    checkbox
};

enum class button_id {
    start_inference,
    switch_camera_stream,
    switch_auto_crop,
    select_store_path,
    clear_all_croppeds
};

enum class combobox_id {
    adjust_interval,
    cropped_count,
    camera_index
};

enum class checkbox_id {
    multi_cameras
};

template <typename E>
struct control_traits;

template <>
struct control_traits<button_id> {
    using id = button_id;
    static constexpr std::string_view type_name = "button";
    static constexpr std::array entries {
        std::pair{button_id::start_inference,       "start_inf"},
        std::pair{button_id::switch_camera_stream,  "switch_camera_stream"},
        std::pair{button_id::switch_auto_crop,      "switch_auto_crop"},
        std::pair{button_id::select_store_path,     "select_store_path"},
        std::pair{button_id::clear_all_croppeds,    "clear_all_croppeds"}
    };
};

template <>
struct control_traits<combobox_id> {
    using id = combobox_id;
    static constexpr std::string_view type_name = "combobox";
    static constexpr std::array entries {
        std::pair{combobox_id::adjust_interval, "adjust_inf_interval"},
        std::pair{combobox_id::cropped_count,   "select_cropped_count"},
        std::pair{combobox_id::camera_index,    "select_camera_index"}
    };
};

template <>
struct control_traits<checkbox_id> {
    using id = checkbox_id;
    static constexpr std::string_view type_name = "checkbox";
    static constexpr std::array entries {
        std::pair{checkbox_id::multi_cameras, "switch_multi_cameras"}
    };
};

template <typename E>
constexpr std::string_view enum2string(E id) {
    const auto& entries = control_traits<E>::entries;
    for (const auto& [key, value]: entries) 
        if (key == id) return value;
    return "";
}

template <typename E>
constexpr E string2enum(std::string_view str) {
    const auto& entries = control_traits<E>::entries;
    for (const auto& [key, value]: entries)
        if (value == str) return key;
    throw std::invalid_argument("Invalid enum string");
}

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <functional>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

template <control_type Type>
class control_factory;

template <>
class control_factory<control_type::button> {
public:
    using id = button_id;
    using widget_type = QPushButton;
    using callback = std::function<void()>;
    
    static QPushButton* create(id id_, QWidget* parent_, callback cb_) {
        auto* btn = new QPushButton(parent_);
        btn->setObjectName(QString(enum2string(id_).data()));
        configure(id_, btn);
        QObject::connect(btn, &QPushButton::clicked, cb_);
        return btn;
    }
private:
    static void configure(id id_, QPushButton* btn_) {
        switch (id_) {
            case button_id::start_inference: 
                btn_->setText("Start Inference");
                break;
            case button_id::switch_camera_stream:
                break;
            case button_id::switch_auto_crop:
                break;
            case button_id::select_store_path:
                break;
            case button_id::clear_all_croppeds:
                break;
        }
    }
};


#include <utils/ort_inf.h>

class actions_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit actions_wrapper(QWidget* parent = nullptr);
    ~actions_wrapper() = default;
private:
    std::unique_ptr<ort_inferer> m_ort_inferer;

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
