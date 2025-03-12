

#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QMutex>
#include <QVector>
#include <QString>
#include <QVariant>
#include <QSharedPointer>
#include <QThreadStorage>
#include <qdatetime.h>
#include <qmutex.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qvariant.h>

class database_connection {
public:
    database_connection();
    ~database_connection();

    bool connect( const QString& driver
                , const QString& host
                , const QString& db_name
                , const QString& user
                , const QString& password
                , int port
                );

    bool is_connected() const;
    void close();

    QSqlQuery exec_query( const QString& query
                        , const QMap<QString, QVariant>& params 
                            = QMap<QString, QVariant>()
                        );
    QSqlQuery prepare_query(const QString& query);
    bool execute_non_query( const QString& query
                          , const QMap<QString, QVariant>& params 
                                = QMap<QString, QVariant>());

    QSqlDatabase& data_base() { return m_db;}
private:
    QSqlDatabase m_db;
    QString m_connection_name;
    bool m_connected;

};


class db_manager: public QObject {
    Q_OBJECT
public:
    static db_manager& instance();

    bool initialize( const QString& driver
                   , const QString& host
                   , const QString& db_name
                   , const QString& user
                   , const QString& password
                   , int port = 5432
                   );

    bool is_initialized() const;

    QSharedPointer<database_connection> get_connection();

    void release_connection(QSharedPointer<database_connection> connection) { }
    
    void close_all() { }

    bool add_rtsp_config( const QString& username
                        , const QString& ip
                        , int port
                        , const QString& channel
                        , const QString& subtypem
                        , const QString& rtsp_url
                        ) { return false; }

    bool update_rtsp_config( int id
                           , const QString& username
                           , const QString& ip
                           , int port
                           , const QString& channel
                           , const QString& subtype
                           , const QString& rtsp_url
                           ) { return false; }

    bool delete_rtsp_config(int id) { return false; }

    QVector<QMap<QString, QVariant>> get_all_rtso_configs() { return {}; }
    QMap<QString, QVariant> get_rstp_config_by_id(int id) { return {};  }

    bool add_inf_result( int camera_id
                       , const QVector<QString>& text_content
                       , const QVector<float>& confidence = {}
                       );
    
    QVector<QMap<QString, QVariant>> 
    get_inf_results( int camera_id
                   , const QDateTime& start_time = QDateTime()
                   , const QDateTime& end_time = QDateTime()
                   , int limit = 100
                   ) { return {}; }

signals:
    void database_error(const QString& error);
    void operation_completed(const QString& operation, bool success);
private:
    explicit db_manager(QObject* parent = nullptr);
    ~db_manager();

    db_manager(const db_manager&) = delete;
    db_manager& operator=(const db_manager&) = delete;

    bool ensure_tables_exist();
    bool create_rtsp_config_table();
    bool create_inf_reuslts_table();


    QVector<QSharedPointer<database_connection>> m_connection_pool;
    QMutex m_pool_mutex;
    
    
    QString m_driver;
    QString m_host;
    QString m_db_name;
    QString m_user;
    QString m_password;
    int m_port;

    bool m_initialized;
    int m_max_poolsize;
};

#endif // DB_MANAGER_H
