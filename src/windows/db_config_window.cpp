

#include "HCNetSDK.h"
#include "utils/db_manager.h"
#include "utils/db_manager_mini.h"
#include <QtSql/qsqldatabase.h>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qdebug.h>
#include <qformlayout.h>
#include <qgroupbox.h>
#include <qline.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qwidget.h>
#include <windows/db_config_window.h>


#include <QStyle>

db_config_window::db_config_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    {
    setWindowTitle(tr("Database Connection"));

    m_settings = new QSettings("Chun Hui", "inf_qwq", this);
    
    setup_UI();
    create_connections();
    load_settings();
}


void db_config_window::setup_UI() {
    QFormLayout* db_config_form_layout = new QFormLayout();

    m_host_edit = new QLineEdit(this);
    m_host_edit->setPlaceholderText(tr("localhost"));

    m_database_edit = new QLineEdit(this);
    m_database_edit->setPlaceholderText(tr("database name"));
 
    m_username_edit = new QLineEdit(this);
    m_username_edit->setPlaceholderText(tr("database name"));

    m_password_edit = new QLineEdit(this);
    m_password_edit->setPlaceholderText(tr("password"));
    m_password_edit->setEchoMode(QLineEdit::Password);

    m_port_spinbox = new QSpinBox(this);
    m_port_spinbox->setRange(1, 65535);
    m_port_spinbox->setValue(5432);

    db_config_form_layout->addRow(tr("Host:"), m_host_edit);
    db_config_form_layout->addRow(tr("Database: "), m_database_edit);
    db_config_form_layout->addRow(tr("Username: "), m_username_edit);
    db_config_form_layout->addRow(tr("Password: "), m_password_edit);
    db_config_form_layout->addRow(tr("Port: "), m_port_spinbox);


    m_test_btn = new QPushButton(tr("Test Connection"), this);
    m_save_btn = new QPushButton(tr("Save Config"), this);
    m_connenction_btn = new QPushButton(tr("Connect"), this);
    m_cancle_btn = new QPushButton(tr("Cancel"), this);

    m_test_btn->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
    m_save_btn->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_connenction_btn->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    m_cancle_btn->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addWidget(m_test_btn);
    button_layout->addWidget(m_save_btn);
    button_layout->addStretch();
    button_layout->addWidget(m_connenction_btn);
    button_layout->addWidget(m_cancle_btn);

    m_status_label = new QLabel(this);
    m_status_label->setWordWrap(true);

    QVBoxLayout* main_layout = new QVBoxLayout(this);

    QGroupBox* connection_group = new QGroupBox(tr("Connction Settings"), this);
    connection_group->setLayout(db_config_form_layout);

    main_layout->addWidget(connection_group);
    main_layout->addLayout(button_layout);
    main_layout->addWidget(m_status_label);


    setLayout(main_layout);
}


db_config_window::~db_config_window() {
    if (m_db.isOpen()) m_db.close();
}

void db_config_window::create_connections() {
    connect (m_test_btn, &QPushButton::clicked, this, &db_config_window::on_test_connection);
    connect (m_save_btn, &QPushButton::clicked, this, &db_config_window::on_save_config);
    connect (m_connenction_btn, &QPushButton::clicked, this, &db_config_window::on_connect);
    connect (m_cancle_btn, &QPushButton::clicked, this, &db_config_window::on_cancel);
    
}

void db_config_window::on_test_connection() {
    m_status_label->setText(tr("Testing connection..."));
    QApplication::processEvents();

    if (test_connection()) {
        m_status_label->setText(tr("Connection successful!"));
        m_status_label->setStyleSheet("color: green");
    } else {
        m_status_label->setStyleSheet("color: red;");
    }
}

void db_config_window::on_save_config() {
    save_settings();
    m_status_label->setText(tr("Configuration saved."));
    m_status_label->setStyleSheet("color: black;");

}

void db_config_window::on_connect() {
    m_status_label->setText(tr("Connecting to database..."));
    QApplication::processEvents();

    qDebug() << "Try to connect";
    qDebug() << "host: " << m_host_edit->text()
             << "databse: " << m_database_edit->text()
             << "username: " << m_username_edit->text()
             << "password: " << m_password_edit->text()
             << "port: " << m_port_spinbox->value();
    
    bool success = db_manager::instance().connect(
        m_host_edit->text(),
        m_database_edit->text(),
        m_username_edit->text(), 
        m_password_edit->text(),
        m_port_spinbox->value()); 
    

    if (success) {
        save_settings();
        m_status_label->setText(tr("Connection successful!"));
        m_status_label->setStyleSheet("color: green;");
        emit connection_established();
        
    } else {
        m_status_label->setText(tr("Connection failed"));
        m_status_label->setStyleSheet("color: red;");
    }
}

void db_config_window::on_cancel() {
    close();
}

void db_config_window::save_settings() {
    m_settings->setValue("Database/Host", m_host_edit->text());
    m_settings->setValue("Database/Database", m_database_edit->text());
    m_settings->setValue("Database/Username", m_username_edit->text());
    m_settings->setValue("Database/Password", m_password_edit->text());
    m_settings->setValue("Database/Port", m_port_spinbox->value());
    m_settings->sync();
}

void db_config_window::load_settings() {
    m_host_edit->setText(m_settings->value("Database/Host").toString());
    m_database_edit->setText(m_settings->value("Database/Database").toString());
    m_username_edit->setText(m_settings->value("Database/Username").toString());
    m_password_edit->setText(m_settings->value("Database/Password").toString());
    int port = m_settings->value("Database/Port", 5432).toInt();
    m_port_spinbox->setValue(port);
}

bool db_config_window::test_connection() {
    if (m_db.isOpen()) m_db.close();

    if (QSqlDatabase::contains("test_connection")) {
        QSqlDatabase::removeDatabase("test_connection");
    }
    
    m_db = QSqlDatabase::addDatabase("QPSQL", "test_connection");
    m_db.setHostName(m_host_edit->text());
    m_db.setDatabaseName(m_database_edit->text());
    m_db.setUserName(m_username_edit->text());
    m_db.setPassword(m_password_edit->text());
    m_db.setPort(m_port_spinbox->value());
    

    if (!m_db.open()) {
        QString error_msg = tr("Connction failed: ") + m_db.lastError().text();
        m_status_label->setText(error_msg);
        emit connection_failed(error_msg);
        return false;
    }
    return true;
}



QString db_config_window::get_host() const { return m_host_edit->text(); }
QString db_config_window::get_database() const { return m_database_edit->text(); }
QString db_config_window::get_username() const { return m_username_edit->text(); }
QString db_config_window::get_password() const { return m_password_edit->text(); }
int db_config_window::get_port() const { return m_port_spinbox->value(); }

bool db_config_window::has_saved_config() const {
    return m_settings->contains("Database/Host")    && 
           m_settings->contains("Database/Databse") &&
           m_settings->contains("Database/Username");
}

void db_config_window::load_saved_config() {
    load_settings();
}
