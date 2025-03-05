#include <components/actions_wrapper.h>
#include <qabstractspinbox.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>


actions_wrapper::actions_wrapper(QWidget* parent)
    : QWidget{parent}
    , m_ort_inferer{ std::make_unique<ort_inferer>() }
    , m_actions_layout{new QGroupBox(this)}
    , m_actions_layout_wrapper{new QHBoxLayout(m_actions_layout)}
    // col 1
    , m_col_1_layout_wrapper{new QVBoxLayout()}
    // start inf
    , m_start_inf_wrapper{new QHBoxLayout()}
    // edit keywords
    , m_edit_keywords_wrapper{new QHBoxLayout()}
    // exit app
    , m_exit_app_wrapper{new QHBoxLayout()}
    // col 2
    , m_col_2_layout_wrapper{new QVBoxLayout()}
    // edit username
    , m_edit_username_wrapper{new QHBoxLayout()}
    // edit password
    , m_edit_password_wrapper{new QHBoxLayout()}
    // edit port
    , m_edit_port_wrapper{new QHBoxLayout}
    // col 3
    , m_col_3_layout_wrapper{new QVBoxLayout()}
    // edit ip
    , m_edit_ip_wrapper{new QHBoxLayout()}
    // edit channels
    , m_edit_channels_wrapper{new QHBoxLayout()}
    // edit sub_channels
    , m_edit_sub_channels_wrapper{new QHBoxLayout()}
    {
    // col_1
    // start inf
    m_start_inf = new QPushButton("Start");
    m_start_inf_wrapper->addWidget(m_start_inf);
    
    connect (m_start_inf, &QPushButton::clicked, this, [this]() {
        emit start_inf();
    });

    // edit keywords
    m_edit_keywords_label = new QLabel("Keywords");
    m_edit_keywords = new QLineEdit();
    m_edit_keywords_wrapper->addWidget(m_edit_keywords_label);
    m_edit_keywords_wrapper->addWidget(m_edit_keywords);
    connect(m_edit_keywords, &QLineEdit::textChanged, this, [this]() {
        // qDebug() << "keywords: " << m_edit_keywords->text();
        emit keywords_changed(m_edit_keywords->text());
    });

    // exit app
    m_exit_app = new QPushButton("Exit");
    m_exit_app_wrapper->addWidget(m_exit_app);

    m_col_1_layout_wrapper->addLayout(m_start_inf_wrapper);     // start inf
    m_col_1_layout_wrapper->addLayout(m_edit_keywords_wrapper); // edit keywords
    m_col_1_layout_wrapper->addLayout(m_exit_app_wrapper);      // exit app
    
    // col 2
    // edit username
    m_edit_username_label = new QLabel("Username");
    m_edit_username = new QLineEdit();
    m_edit_username_wrapper->addWidget(m_edit_username_label);
    m_edit_username_wrapper->addWidget(m_edit_username);

    // edit password
    m_edit_password_label = new QLabel("Password");
    m_edit_password = new QLineEdit();
    m_edit_password_wrapper->addWidget(m_edit_password_label);
    m_edit_password_wrapper->addWidget(m_edit_password);

    // edit port
    m_edit_port_label = new QLabel("Port");
    m_edit_port = new QLineEdit();
    m_edit_port_wrapper->addWidget(m_edit_port_label);
    m_edit_port_wrapper->addWidget(m_edit_port);

    m_col_2_layout_wrapper->addLayout(m_edit_username_wrapper);     // edit username
    m_col_2_layout_wrapper->addLayout(m_edit_password_wrapper);     // edit password
    m_col_2_layout_wrapper->addLayout(m_edit_port_wrapper);         // edit port

    // col 3
    // edit ip
    m_edit_ip_label = new QLabel("IP");
    m_edit_ip = new QLineEdit();
    m_edit_ip_wrapper->addWidget(m_edit_ip_label);
    m_edit_ip_wrapper->addWidget(m_edit_ip);

    // edit channels
    m_edit_channels_label = new QLabel("Channels");
    m_edit_channels = new QLineEdit();
    m_edit_channels_wrapper->addWidget(m_edit_channels_label);
    m_edit_channels_wrapper->addWidget(m_edit_channels);

    // edit sub_channels
    m_edit_sub_channels_label = new QLabel("Sub Channels");
    m_edit_sub_channels = new QLineEdit();
    m_edit_sub_channels_wrapper->addWidget(m_edit_sub_channels_label);
    m_edit_sub_channels_wrapper->addWidget(m_edit_sub_channels);

    m_col_3_layout_wrapper->addLayout(m_edit_ip_wrapper);
    m_col_3_layout_wrapper->addLayout(m_edit_channels_wrapper);
    m_col_3_layout_wrapper->addLayout(m_edit_sub_channels_wrapper);

    m_actions_layout_wrapper->addLayout(m_col_1_layout_wrapper);
    m_actions_layout_wrapper->addLayout(m_col_2_layout_wrapper);
    m_actions_layout_wrapper->addLayout(m_col_3_layout_wrapper);
}
