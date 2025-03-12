#include "components/camera_wrapper.h"
#include "windows/rtsp_config_window.h"
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qformlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qtextbrowser.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <windows/expanded_camera_window.h>
#include <QDebug>

expanded_camera_window::expanded_camera_window(camera_wrapper* org_cam, QWidget* parent)
    : QWidget{parent, Qt::Window}
    , m_org_camera{org_cam}
{
    setup_UI();
    connect_signals();

    setWindowTitle("Expanded Camera View - Camera " + QString::number(org_cam->get_cam_id()));
    resize(1200, 800);
    setAttribute(Qt::WA_DeleteOnClose);
}

void expanded_camera_window::set_cropped_image(const QImage& image) {
    QSize label_size = m_image_display->size();
    QSize scaled_size = image.size();

    scaled_size.scale( label_size, Qt::KeepAspectRatio);

    QImage scaled_image = image.scaled( scaled_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QImage background(label_size, QImage::Format_RGB888);
    background.fill(Qt::black);

    QPainter painter(&background);
    int x = (label_size.width() - scaled_size.width()) / 2;
    int y = (label_size.height() - scaled_size.height()) / 2;
    painter.drawImage(x, y, scaled_image);

    m_image_display->setPixmap(QPixmap::fromImage(background));
}


// void expanded_camera_window::set_inf_result(const QString& inf_res) {
//     m_text_display->setText(inf_res);
// }

void expanded_camera_window::set_rstp_info(const rtsp_config& rtsp_cfg) {
    m_rtsp_config = rtsp_cfg;    
     
    m_rtsp_proto_type->setText(parse_rtsp_protocal_type());
    m_rtsp_username->setText(m_rtsp_config.username);
    m_rtsp_ip->setText(m_rtsp_config.ip);
    m_rtsp_port->setText(m_rtsp_config.port);
    m_rtsp_channel->setText(m_rtsp_config.channel);
    m_rtsp_subtype->setText(m_rtsp_config.subtype);

}

QString expanded_camera_window::parse_rtsp_protocal_type() {
    switch(m_rtsp_config.rpt) {
        case rtsp_proto_type::HIKVISION: return "HIKVISION";
        case rtsp_proto_type::ALHUA:     return "ALHUA";
        default:                         return "";
    }
}

void expanded_camera_window::setup_UI() {
    m_expanded_camera_window_layout = new QVBoxLayout(this);
    m_expanded_camera_window_layout->setContentsMargins(10, 10, 10, 10);
    m_expanded_camera_window_layout->setSpacing(10);

    m_rtsp_proto_type = new QLabel();
    m_rtsp_username = new QLabel();
    m_rtsp_ip = new QLabel();
    m_rtsp_port = new QLabel();
    m_rtsp_channel = new QLabel();
    m_rtsp_subtype = new QLabel();

    m_content_splitter = new QSplitter(Qt::Horizontal, this);
    m_expanded_camera_window_layout->addWidget(m_content_splitter);
    
    create_left_panel();
    create_right_panel();

    QList<int> main_sizes;
    main_sizes << width() * 0.6 << width() * 0.4;
    m_content_splitter->setSizes(main_sizes);
    setLayout(m_expanded_camera_window_layout);
}


void expanded_camera_window::create_left_panel() {
    m_left_widget = new QWidget(m_content_splitter);
    m_left_layout = new QVBoxLayout(m_left_widget);
    m_left_layout->setContentsMargins(0, 0, 0, 0);

    m_camera = new camera_wrapper(m_org_camera->get_cam_id(), m_left_widget);
    
    m_camera->set_rtsp_stream(m_org_camera->get_rtsp_url());
    
    m_left_layout->addWidget(m_camera);    
    m_left_widget->setLayout(m_left_layout);
    m_content_splitter->addWidget(m_left_widget);
}

void expanded_camera_window::create_right_panel() {
    m_right_widget = new QWidget(m_content_splitter);
    m_right_layout = new QVBoxLayout(m_right_widget);
    m_right_layout->setContentsMargins(0, 0, 0, 0);

    m_right_splitter = new QSplitter(Qt::Vertical, m_right_widget);
    m_right_layout->addWidget(m_right_splitter);

    create_right_top_panel();
    create_right_bottom_panel();

    QList<int> right_sizes;
    right_sizes << m_right_widget->height() * 0.7 << m_right_widget->height() * 0.3;
    m_right_splitter->setSizes(right_sizes);
    
    m_right_widget->setLayout(m_right_layout);
    m_content_splitter->addWidget(m_right_widget);
}

void expanded_camera_window::create_right_top_panel() {
    m_right_top_widget = new QWidget(m_right_splitter);
    m_right_top_layout = new QVBoxLayout(m_right_top_widget);
    m_right_top_layout->setContentsMargins(0, 0, 0, 0);

    m_right_top_splitter = new QSplitter(Qt::Vertical, m_right_top_widget);
    m_right_top_layout->addWidget(m_right_top_splitter);

    // cropped image
    m_image_widget = new QWidget(m_right_top_splitter);
    m_image_layout = new QVBoxLayout(m_image_widget);
    
    m_image_display = new QLabel(m_image_widget);
    m_image_display->setAlignment(Qt::AlignCenter);
    m_image_display->setMinimumSize(200, 200);
    m_image_display->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    m_image_display->setText("Cropped Image");

    m_image_layout->addWidget(m_image_display);
    m_image_widget->setLayout(m_image_layout);
    

    // inf result
    m_text_widget = new QWidget(m_right_top_splitter);
    m_text_layout = new QVBoxLayout(m_text_widget);
   
    // m_text_display = new QTextEdit(m_text_widget);
    // m_text_display->setReadOnly(true);
    // m_text_display->setText("Inference result");
    m_inf_result = new QTextBrowser();
    

    // relative keywords
    m_keywords_wrapper = new QHBoxLayout();
    m_keywords_label = new QLabel();
    m_keywords_layout = new QGroupBox("Keywords");
    m_keywords_wrapper = new QHBoxLayout();
    m_keywords_wrapper->addWidget(m_keywords_label);
    m_keywords_layout->setLayout(m_keywords_wrapper);

    m_text_layout->addWidget(m_keywords_layout);
    //m_text_layout->addWidget(m_text_display);
    m_text_layout->addWidget(m_inf_result);
    m_text_widget->setLayout(m_text_layout);
    
    m_right_top_splitter->addWidget(m_image_widget);
    m_right_top_splitter->addWidget(m_text_widget);

    QList<int> top_sizes;
    top_sizes << m_right_top_widget->width() * 0.5 
              << m_right_top_widget->width() * 0.5;
    m_right_top_splitter->setSizes(top_sizes);

    m_right_top_widget->setLayout(m_right_top_layout);
    m_right_splitter->addWidget(m_right_top_widget);
}

void expanded_camera_window::create_right_bottom_panel() {
    m_settings_group = new QGroupBox("Detection Settings", m_right_splitter);
    m_settings_layout = new QVBoxLayout(m_settings_group);
    m_rtsp__inf_layout = new QHBoxLayout(); 
    m_rtsp_info_layout = new QFormLayout();
    m_inf_config_layout = new QFormLayout();
    m_edit_layout = new QVBoxLayout();

    m_rtsp_info_layout->addRow("RTSP Protocal Type", m_rtsp_proto_type);
    m_rtsp_info_layout->addRow("RTSP Username", m_rtsp_username);
    m_rtsp_info_layout->addRow("RTSP IP", m_rtsp_ip);
    m_rtsp_info_layout->addRow("RTSP Port", m_rtsp_port);
    m_rtsp_info_layout->addRow("RTSP Channel", m_rtsp_channel);
    m_rtsp_info_layout->addRow("RTSP Subtype", m_rtsp_subtype);

    m_edit_keywords_wrapper = new QHBoxLayout();
    m_edit_keywords_label = new QLabel("Keyword: ");
    m_edit_keywords = new QLineEdit();
    
    m_edit_keywords_wrapper->addWidget(m_edit_keywords_label);
    m_edit_keywords_wrapper->addWidget(m_edit_keywords);

    m_comfirm_config_btn = new QPushButton("Comfirm"); 
   
    m_edit_layout->addLayout(m_edit_keywords_wrapper);
    m_edit_layout->addWidget(m_comfirm_config_btn);

    m_rtsp__inf_layout->addLayout(m_rtsp_info_layout);
    m_rtsp__inf_layout->addLayout(m_inf_config_layout);
    
    m_settings_layout->addLayout(m_rtsp__inf_layout);
    m_settings_layout->addLayout(m_edit_layout);

    m_settings_group->setLayout(m_settings_layout);
    m_right_splitter->addWidget(m_settings_group);
}

void expanded_camera_window::connect_signals() {    
    connect ( m_edit_keywords   // -> update keywords
            , &QLineEdit::textChanged
            , this, [this](){
                QString keywords = m_edit_keywords->text();
                qDebug() << "Keywords edit: " << keywords;
                set_keywords(keywords);
                emit keywords_changed(keywords);
            });
}
