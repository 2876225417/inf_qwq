

#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <windows/stream_settings_window.h>


stream_settings_window::stream_settings_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    // stream setting options layout
    , m_stream_settings_main_layout{new QVBoxLayout()}
    // scale factor
    , m_adjust_scale_factor_wrapper{new QHBoxLayout()}
    // auto crop
    , m_check_auto_crop_wrapper{new QHBoxLayout()}
    {

    m_adjust_scale_factor_label = new QLabel("Scale Factor");
    m_adjust_scale_factor = new QComboBox();

    m_adjust_scale_factor->addItem("10%", 0.1f); 
    m_adjust_scale_factor->addItem("25%", 0.25f); 
    m_adjust_scale_factor->addItem("50%", 0.5f); 
    m_adjust_scale_factor->addItem("75%", 0.75f); 
    m_adjust_scale_factor->addItem("100%", 1.f); 

    m_adjust_scale_factor->setCurrentIndex(2);

    connect ( m_adjust_scale_factor
            , QOverload<int>::of(&QComboBox::currentIndexChanged)
            , this, [this](int index) {
                float scale_factor = m_adjust_scale_factor->itemData(index).toFloat();
                emit scale_factor_changed(scale_factor);
            });

    m_adjust_scale_factor_wrapper->addWidget(m_adjust_scale_factor_label);
    m_adjust_scale_factor_wrapper->addWidget(m_adjust_scale_factor);

    m_check_auto_crop_label = new QLabel("Auto Crop");
    m_check_auto_crop = new QCheckBox();
    m_check_auto_crop_wrapper->addWidget(m_check_auto_crop_label);
    m_check_auto_crop_wrapper->addWidget(m_check_auto_crop);
    
    

    m_stream_settings_main_layout->addLayout(m_adjust_scale_factor_wrapper);
    m_stream_settings_main_layout->addLayout(m_check_auto_crop_wrapper);

    setLayout(m_stream_settings_main_layout);
}
