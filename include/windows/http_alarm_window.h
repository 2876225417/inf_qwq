#ifndef HTTP_ALARM_WINDOW_H
#define HTTP_ALARM_WINDOW_H


#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkrequest.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstringview.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QSpinBox>
#include <QComboBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTextEdit>
#include <QJsonDocument>





class http_alarm_window: public QWidget {
    Q_OBJECT
public:
    explicit http_alarm_window(QWidget* parent = nullptr);
    ~http_alarm_window();

signals:
    void config_changed(const QString& url, bool radiated_all);

private slots:
    void on_save_config();
    void on_test_connection();
    void on_clear_log();
    void on_validate_json();
    void on_request_method_changed(int index);
private:
    QVBoxLayout* m_main_layout;

    // config
    QGroupBox* m_server_group;
    QGridLayout* m_server_layout;
    
    QLabel* m_host_label;
    QLineEdit* m_host_edit;

    QLabel* m_port_label;
    QSpinBox* m_port_spinbox;

    QLabel* m_path_label;
    QLineEdit* m_path_edit;

    QLabel* m_method_label;
    QComboBox* m_method_combo;

    QLabel* m_auth_label;
    QLineEdit* m_auth_edit;

    QLabel* m_interval_label;
    QSpinBox* m_interval_spinbox;

    QCheckBox* m_enabled_check;
    QCheckBox* m_auto_start_check;
    QCheckBox* m_radiated_for_all_check;
    bool m_radiated_for_all = false;


    // json
    QGroupBox* m_json_group;
    QVBoxLayout* m_json_layout;

    QTextEdit* m_json_edit;
    QPushButton* m_validate_json_button;

    // control
    QHBoxLayout* m_control_layout;
    QPushButton* m_test_button;
    QPushButton* m_save_button;

    // log
    QGroupBox* m_log_group;
    QVBoxLayout* m_log_layout;
    QTextEdit* m_log_edit;
    QPushButton* m_clear_log_button;

    QNetworkAccessManager* m_network_mgr;

    void setup_UI();
    void setup_connections();
    void load_config();
    void save_config();
    void log_message(const QString& msg, bool is_error = false);
    QString get_full_url() const;
    QByteArray get_request_body() const;
    QNetworkRequest create_request() const;
};


#endif // HTTP_ALARM_WINDOW_H
