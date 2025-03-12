


#ifndef DB_CONFIG_WINDOW_H
#define DB_CONFIG_WINDOW_H


#include <QWidget>
#include <QtSql/qsqldatabase.h>
#include <qgridlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qwidget.h>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <utils/db_manager.h>
#include <utils/db_manager_mini.h>


class db_config_window: public QWidget {
    Q_OBJECT
public:
    explicit db_config_window(QWidget* parent = nullptr);
    ~db_config_window();

    QString get_host()     const;
    QString get_database() const;
    QString get_username() const;
    QString get_password() const;
    int     get_port()     const;


    bool has_saved_config() const;
    
    void load_saved_config();

signals:
    void connection_established();
    void connection_failed(const QString& err_msg);

private slots:
    void on_test_connection();
    void on_save_config();
    void on_connect();
    void on_cancel();
private:
    void setup_UI();
    void create_connections();
    void save_settings();
    void load_settings();
    bool test_connection();


    QLineEdit* m_host_edit;
    QLineEdit* m_database_edit;
    QLineEdit* m_username_edit;
    QLineEdit* m_password_edit;
    QSpinBox*  m_port_spinbox;

    QPushButton* m_test_btn;
    QPushButton* m_save_btn;
    QPushButton* m_connenction_btn;
    QPushButton* m_cancle_btn;

    QLabel* m_status_label;
    QGridLayout* m_db_config_layout;

    QSettings* m_settings;
    QSqlDatabase m_db;
};
#endif
