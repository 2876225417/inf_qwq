


#include "windows/stream_settings_window.h"
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qoverload.h>
#include <qwidget.h>
#include <windows/settings_window.h>


settings_window::settings_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    {
    setWindowTitle("Settings");
    setup_UI();
    setup_connections();
    setLayout(m_settings_main_layout);
}

settings_window::~settings_window() {}

void settings_window::setup_UI() {
    m_settings_main_layout = new QVBoxLayout(this);
    m_settings_main_layout->setSpacing(10);
    m_settings_main_layout->setContentsMargins(10, 10, 10, 10);


    QGroupBox* m_settings_main_box = new QGroupBox("全局设置");
    QVBoxLayout* m_settings_main_groupbox = new QVBoxLayout(); 

    m_adjust_rtsp_stream_count_layout = new QHBoxLayout();     
    m_rtsp_stream_count_label = new QLabel("单组RSTP流数量");
    m_adjust_rtsp_stream_count_combo = new QComboBox();

    m_adjust_rtsp_stream_count_combo->addItems({"4", "9", "25", "36", "49", "64"});

    m_adjust_rtsp_stream_count_layout->addWidget(m_rtsp_stream_count_label);
    m_adjust_rtsp_stream_count_layout->addWidget(m_adjust_rtsp_stream_count_combo);



    m_settings_main_groupbox->addLayout(m_adjust_rtsp_stream_count_layout);

    m_settings_main_box->setLayout(m_settings_main_groupbox);
    

    m_settings_main_layout->addWidget(m_settings_main_box);
    setWindowTitle("设置");
    setMinimumWidth(500);
    setMinimumHeight(350);
}



void settings_window::setup_connections() {
    connect ( m_adjust_rtsp_stream_count_combo
            , QOverload<int>::of(&QComboBox::currentIndexChanged)
            , this, [this](int idx) {
                int current_count =  m_adjust_rtsp_stream_count_combo->itemText(idx).toInt();
                emit send_rtsp_stream_count_changed(current_count); 
            });    
}
