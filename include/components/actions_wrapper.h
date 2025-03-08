#pragma once

#include <array>
#include <qabstractspinbox.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qendian.h>
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
#include <QLineEdit>

enum class rtsp_protocal_type {
    HIKVISION,
    ALHUA
};


class actions_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit actions_wrapper(QWidget* parent = nullptr);
    ~actions_wrapper() = default;

signals:
    void keywords_changed(const QString& keywords);
    void username_changed(const QString& username);
    void password_changed(const QString& password);

    void ip_changed(const QString& ip);
    void port_changed(const QString& port);
    void scale_factor_changed(double factor);
    void rtsp_protocal_type_changed(rtsp_protocal_type rpt);

    void start_inf();
    void connect_cam();
    void exit_app();
private:
    std::unique_ptr<ort_inferer> m_ort_inferer;

    QGroupBox* m_actions_layout;
    QHBoxLayout* m_actions_layout_wrapper;

    // col 1
    QVBoxLayout* m_col_1_layout_wrapper;
    // start inf
    QHBoxLayout* m_start_inf_wrapper;
    QPushButton* m_start_inf; 
    // placeholder
    QHBoxLayout* m_connector_wrapper;
    QPushButton* m_connector;
    // exit app
    QHBoxLayout* m_exit_app_wrapper;
    QPushButton* m_exit_app;
    
    // col 2
    QVBoxLayout* m_col_2_layout_wrapper;
    
    // row_1
    QHBoxLayout* m_col_2_row_1_layout_wrapper;
    // edit username
    QHBoxLayout* m_edit_username_wrapper;
    QLabel*      m_edit_username_label;
    QLineEdit*   m_edit_username;
    // edit password
    QHBoxLayout* m_edit_password_wrapper;
    QLabel*      m_edit_password_label;
    QLineEdit*   m_edit_password;
    
    // row 2
    QHBoxLayout* m_col_2_row_2_layout_wrapper;
    // edit port
    QHBoxLayout* m_edit_port_wrapper;
    QLabel*      m_edit_port_label;
    QLineEdit*   m_edit_port;
    // edit ip 
    QHBoxLayout* m_edit_ip_wrapper;
    QLabel*      m_edit_ip_label;
    QLineEdit*   m_edit_ip;
    // change camera scale
    QHBoxLayout* m_adjust_camera_scale_wrapper;
    QLabel*      m_camera_scale_label;
    QComboBox*   m_adjust_camera_scale; 
    // change rtsp protocal
    QHBoxLayout* m_switch_rtsp_wrapper;
    QLabel*      m_swtich_rtsp_label;
    QComboBox*   m_switch_rtsp;


    // row 3
    QHBoxLayout* m_col_2_row_3_layout_wrapper;
    // edit keywords
    QHBoxLayout* m_edit_keywords_wrapper;
    QLabel*      m_edit_keywords_label;
    QLineEdit*   m_edit_keywords;

};
