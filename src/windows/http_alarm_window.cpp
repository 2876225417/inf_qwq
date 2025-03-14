

#include "utils/ort_inf.h"
#include <qapplication.h>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qobject.h>
#include <qoverload.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qstringview.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <stdatomic.h>
#include <windows/http_alarm_window.h>




#include <QStyle>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>


http_alarm_window::http_alarm_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    {
    setWindowTitle("HTTP配置");
    setup_UI();
    setup_connections();

    load_config();

    m_network_mgr = new QNetworkAccessManager(this);

}

http_alarm_window::~http_alarm_window() { }

void http_alarm_window::setup_UI() {
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setSpacing(10);
    m_main_layout->setContentsMargins(10, 10, 10, 10);

    m_server_group = new QGroupBox(tr("服务器配置"), this);
    m_server_layout = new QGridLayout();

    m_host_label = new QLabel(tr("域名/IP"), this);
    m_host_edit = new QLineEdit(this);
    m_host_edit->setPlaceholderText(tr("target.com or xxx.xxx.xxx.xxx"));

    m_port_label = new QLabel(tr("端口"), this);
    m_port_spinbox = new QSpinBox(this);
    m_port_spinbox->setRange(1, 65535);
    m_port_spinbox->setValue(80);
    m_port_spinbox->setSpecialValueText(tr("默认：80"));

    m_path_label = new QLabel(tr("Path: "), this);
    m_path_edit = new QLineEdit(this);
    m_path_edit->setPlaceholderText("/api/alarms");

    m_method_label = new QLabel(tr("请求方法"), this);
    m_method_combo = new QComboBox(this);
    m_method_combo->addItems({"GET", "POST", "PUT", "DELETE"});
    m_method_combo->setCurrentText("POST");

    m_auth_label = new QLabel(tr("验证令牌"), this);
    m_auth_edit = new QLineEdit(this);
    m_auth_edit->setPlaceholderText("可选");
    m_auth_edit->setEchoMode(QLineEdit::Password);

    m_interval_label = new QLabel(tr("测试请求间隔(s): "), this);
    m_interval_spinbox = new QSpinBox(this);
    m_interval_spinbox->setRange(1, 3600);
    m_interval_spinbox->setValue(30);

    m_enabled_check = new QCheckBox(tr("启用HTTP警报"), this);
    m_auto_start_check = new QCheckBox(tr("自动启动"), this);
    m_radiated_for_all_check = new QCheckBox(tr("保持Http警报开启"), this);

    m_server_layout->addWidget(m_host_label, 0, 0);
    m_server_layout->addWidget(m_host_edit, 0, 1, 1, 3);
    m_server_layout->addWidget(m_port_label, 1, 0);
    m_server_layout->addWidget(m_port_spinbox, 1, 1);
    m_server_layout->addWidget(m_path_label, 1, 2);
    m_server_layout->addWidget(m_path_edit, 1, 3);
    m_server_layout->addWidget(m_method_label, 2, 0);
    m_server_layout->addWidget(m_method_combo, 2, 1);
    m_server_layout->addWidget(m_auth_label, 2, 2);
    m_server_layout->addWidget(m_auth_edit, 2, 3);
    m_server_layout->addWidget(m_interval_label, 3, 0);
    m_server_layout->addWidget(m_interval_spinbox, 3, 1);
    m_server_layout->addWidget(m_enabled_check, 4, 0, 1, 2);
    m_server_layout->addWidget(m_auto_start_check, 4, 2, 1, 1);
    m_server_layout->addWidget(m_radiated_for_all_check, 4, 3, 1, 1);

    m_server_group->setLayout(m_server_layout);

    m_json_group = new QGroupBox(tr("Json配置"), this);
    m_json_layout = new QVBoxLayout();

    m_json_edit = new QTextEdit(this);
    m_json_edit->setAcceptRichText(false);
    m_json_edit->setPlaceholderText("{\n  \"key\": \"value\",\n  \"array\": [1, 2, 3]\n}");
    m_json_edit->setFont(QFont("Courier New", 10));

    m_validate_json_button = new QPushButton(tr("JSON验证"), this);
    m_validate_json_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogHelpButton));

    m_json_layout->addWidget(m_json_edit);
    m_json_layout->addWidget(m_validate_json_button);

    m_json_group->setLayout(m_json_layout);

    m_control_layout = new QHBoxLayout();

    m_test_button = new QPushButton(tr("连接测试"), this);
    m_test_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));

    m_save_button = new QPushButton(tr("保存配置"), this);
    m_save_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));

    m_control_layout->addWidget(m_test_button);
    m_control_layout->addStretch();
    m_control_layout->addWidget(m_save_button);


    m_log_group = new QGroupBox(tr("测试日志"), this);
    m_log_layout = new QVBoxLayout();

    m_log_edit = new QTextEdit(this);
    m_log_edit->setReadOnly(true);
    m_log_edit->setFont(QFont("Courier New", 9));


    m_clear_log_button = new QPushButton(tr("清除日志"), this);
    m_clear_log_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogResetButton));

    m_log_layout->addWidget(m_log_edit);
    m_log_layout->addWidget(m_clear_log_button);

    m_log_group->setLayout(m_log_layout);

    m_main_layout->addWidget(m_server_group);
    m_main_layout->addWidget(m_json_group);
    m_main_layout->addLayout(m_control_layout);
    m_main_layout->addWidget(m_log_group, 1);
    
    setLayout(m_main_layout);
}


void http_alarm_window::setup_connections() {
    connect (m_save_button, &QPushButton::clicked, this, &http_alarm_window::on_save_config);
    connect (m_test_button, &QPushButton::clicked, this, &http_alarm_window::on_test_connection);
    connect (m_clear_log_button, &QPushButton::clicked, this, &http_alarm_window::on_clear_log);

    connect (m_method_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &http_alarm_window::on_request_method_changed);
   
    connect (m_radiated_for_all_check, &QCheckBox::toggled, this, [this](bool checked) {
        m_radiated_for_all = checked;
        emit config_changed(get_full_url(), m_radiated_for_all);
    });

    connect (m_enabled_check, &QCheckBox::toggled, this, [this](bool checked) {
        m_host_edit->setEnabled(checked);
        m_port_spinbox->setEnabled(checked);
        m_path_edit->setEnabled(checked);
        m_method_combo->setEnabled(checked);
        m_auth_edit->setEnabled(checked);
        m_interval_spinbox->setEnabled(checked);
        m_auto_start_check->setEnabled(checked);
        m_json_edit->setEnabled(checked);
        m_validate_json_button->setEnabled(checked);
        m_test_button->setEnabled(checked);
        m_radiated_for_all_check->setEnabled(checked);
    });
}

void http_alarm_window::load_config() {
    QSettings settings;

    m_host_edit->setText(settings.value("HttpAlarm/Host", "").toString());
    m_port_spinbox->setValue(settings.value("HttpAlarm/Port", 80).toInt());
    m_path_edit->setText(settings.value("HttpAlarm/Path", "").toString());
    m_method_combo->setCurrentText(settings.value("HttpAlarm/Method", "POST").toString());
    m_auth_edit->setText(settings.value("HttpAlarm/AuthToken", "").toString());
    m_interval_spinbox->setValue(settings.value("HttpAlarm/Interval", 30).toInt());
    m_enabled_check->setChecked(settings.value("HttpAlarm/Enabled", false).toBool());
    m_auto_start_check->setChecked(settings.value("HttpAlarm/AutoStart", false).toBool());
    m_json_edit->setText(settings.value("HttpAlarm/JsonParams", "{}").toString());
    m_radiated_for_all_check->setChecked(settings.value("HttpAlarm/Radiated").toBool());

    bool enabled = m_enabled_check->isChecked();
    m_host_edit->setEnabled(enabled);
    m_port_spinbox->setEnabled(enabled);
    m_path_edit->setEnabled(enabled);
    m_method_combo->setEnabled(enabled);
    m_auth_edit->setEnabled(enabled);
    m_interval_spinbox->setEnabled(enabled);
    m_auto_start_check->setEnabled(enabled);
    m_json_edit->setEnabled(enabled);
    m_validate_json_button->setEnabled(enabled);
    m_test_button->setEnabled(enabled);

    on_request_method_changed(m_method_combo->currentIndex());
    log_message(tr("配置已加载"));
}

void http_alarm_window::save_config() {
    QSettings settings;

    settings.setValue("HttpAlarm/Host", m_host_edit->text());
    settings.setValue("HttpAlarm/Port", m_port_spinbox->value());
    settings.setValue("HttpAlarm/Path", m_path_edit->text());
    settings.setValue("HttpAlarm/Method",  m_method_combo->currentText());
    settings.setValue("HttpAlarm/AuthToken", m_auth_edit->text());
    settings.setValue("HttpAlarm/Interval", m_interval_spinbox->value());
    settings.setValue("HttpAlarm/Enabled", m_enabled_check->isChecked());
    settings.setValue("HttpAlarm/AutoStart", m_auto_start_check->isChecked());
    settings.setValue("HttpAlarm/JsonParams", m_json_edit->toPlainText());
    settings.setValue("HttpAlarm/Radiated", m_radiated_for_all_check->isChecked());

    settings.sync();
    emit config_changed(get_full_url(), m_radiated_for_all);
}

void http_alarm_window::on_save_config() {
    if (!m_json_edit->toPlainText().isEmpty()) {
        QJsonParseError error;
        QJsonDocument::fromJson(m_json_edit->toPlainText().toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            QMessageBox::warning( this
                                , tr("无效Json格式")
                                , tr("无效Json参数, 请修正:\n%1")
                                            .arg(error.errorString())
                                );
            return;
        }
    }   
    save_config();
    log_message(tr("配置已保存"));

    QMessageBox::warning(this, tr("配置已保存"), tr("HTTP配置已保存"));
}


void http_alarm_window::on_test_connection() {
    if (m_host_edit->text().isEmpty()) {
        QMessageBox::warning(this, tr("连接测试"), tr("请输入一个有效的域名/IP"));
        return;
    }
    
    QString url = get_full_url();
    log_message(tr("连接至 %1").arg(url));

    QNetworkRequest request = create_request();
    QByteArray request_body = get_request_body();

    QNetworkReply* reply = nullptr;
    QString method = m_method_combo->currentText();

    if (method == "GET")        reply = m_network_mgr->get(request);
    else if (method == "POST")  reply = m_network_mgr->post(request, request_body);
    else if (method == "PUT")   reply = m_network_mgr->put(request, request_body);
    else if (method == "DELETE")reply = m_network_mgr->deleteResource(request);

    if (!reply) {
        log_message(tr("请求创建失败"), true);
        return;
    }

    log_message(tr("请求方法: %1").arg(method));
    if (!request_body.isEmpty()) log_message(tr("请求体: %1").arg(QString(request_body)));

    connect (reply, &QNetworkReply::finished, [reply, this]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response_data = reply->readAll();
            log_message(tr("连接成功!"));
            log_message(tr("状态码: %1")
                            .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()));

            log_message(tr("响应体: %1").arg(QString(response_data)));

            QMessageBox::information(this, tr("连接测试"), tr("连接成功!"));
        } else {
            QString error_msg = tr("连接至: %1失败").arg(reply->errorString());
            log_message(error_msg, true);
            log_message(tr("状体码: %1").arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()));
            QMessageBox::critical(this, tr("连接测试"), error_msg);
        }
        reply->deleteLater();
    });
}

void http_alarm_window::on_clear_log() {
    m_log_edit->clear();
    log_message(tr("日志已清除"));
}

void http_alarm_window::on_validate_json() {
    QString json_text = m_json_edit->toPlainText().trimmed();
    if (json_text.isEmpty()) {
        QMessageBox::information(this, tr("Json验证"), tr("Json内容为空"));
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json_text.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        QMessageBox::critical( this
                             , tr("无效Json格式")
                             , tr("JSON格式无效: %1 位置: %2")
                                .arg(error.errorString())
                                .arg(error.offset));
    } else {
        QString formatted_json = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
        m_json_edit->setPlainText(formatted_json);
        QMessageBox::information(this, tr("Json验证"), tr("验证Json格式"));
    }
}

void http_alarm_window::on_request_method_changed(int index) {
    QString method = m_method_combo->itemText(index);

    bool show_json_editor = (method == "POST" || method == "PUT");

    m_json_group->setVisible(show_json_editor);
    m_json_edit->setEnabled(show_json_editor && m_enabled_check->isChecked());
    m_validate_json_button->setEnabled(show_json_editor && m_enabled_check->isChecked());

    adjustSize();
}


void http_alarm_window::log_message(const QString& messsage, bool is_error) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString log_entry = QString("[%1] %2").arg(timestamp).arg(messsage);

    QTextCursor cursor = m_log_edit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_log_edit->setTextCursor(cursor);

    if (is_error) m_log_edit->setTextColor(Qt::red);
    else m_log_edit->setTextColor(Qt::black);

    m_log_edit->insertPlainText(log_entry + "\n");
    m_log_edit->ensureCursorVisible();
}

QString http_alarm_window::get_full_url() const {
    QString host = m_host_edit->text();
    int port = m_port_spinbox->value();
    QString path = m_path_edit->text();

    if (host.startsWith("http://") || host.startsWith("https://")) {
        QUrl url(host);
        host = url.host();
    }

    QString url = "http://";
    url += host;

    if (port != 80) url += ":" + QString::number(port);

    if (!path.isEmpty()) {
        if (!path.startsWith("/")) url += "/";

        url += path;
    }
    return url;
}


QByteArray http_alarm_window::get_request_body() const {
    QString method = m_method_combo->currentText();
    if (method == "GET" || method == "DELETE")  return QByteArray(); 

    QString json_text = m_json_edit->toPlainText().trimmed();
    if (json_text.isEmpty()) return "{}";

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json_text.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) return json_text.toUtf8();

    QJsonObject obj = doc.object();
    obj["test"] = true;
    obj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument new_doc(obj);
    return new_doc.toJson();
}

QNetworkRequest http_alarm_window::create_request() const {
    QUrl url(get_full_url());
    QNetworkRequest request(url);

    QString method = m_method_combo->currentText();

    if (method == "POST" || method == "PUT") 
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_auth_edit->text().isEmpty()) 
        request.setRawHeader("Authorization", "Bearer " + m_auth_edit->text().toUtf8());

    request.setHeader(QNetworkRequest::UserAgentHeader, "HttpAlarmClient/1.0");
    return request;
}





