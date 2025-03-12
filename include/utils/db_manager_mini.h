

#ifndef DB_MANAGER_MINI_H
#define DB_MANAGER_MINI_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QMutex>
#include <QVector>
#include <QMap>
#include <QString>
#include <QVariant>

#include <QDateTime>

class db_manager: public QObject {
    Q_OBJECT
    
public:
    static db_manager& instance();
    
    bool connect( const QString& host
                , const QString& db_name
                , const QString& user
                , const QString& password
                , int port = 5432
                );
    
    void disconnect();
    
    bool is_connected() const;
    
    QSqlQuery execute_query( const QString& query
                           , const QMap<QString, QVariant>& params 
                             = QMap<QString, QVariant>()
                           );    
    
    // 执行非查询操作（插入、更新、删除）
    bool execute_non_query( const QString& query
                          , const QMap<QString, QVariant>& params 
                            = QMap<QString, QVariant>()
                          );
    
    // RTSP相关操作
    bool add_rtsp_config( const QString& username
                        , const QString& ip
                        , const QString& port
                        , const QString& channel
                        , const QString& subtype
                        , const QString& rtsp_url
                        );

    bool update_rtsp_config( int id
                           , const QString& username
                           , const QString& ip
                           , const QString& port
                           , const QString& channel
                           , const QString& subtype
                           , const QString& rtsp_url
                           );

    bool delete_rtsp_config(int id);

    QVector<QMap<QString, QVariant>> get_all_rtsp_configs();
    QMap<QString, QVariant> get_rtsp_config_by_id(int id);
    
    // 识别结果相关操作
    bool add_recognition_result( int camera_id
                               , const QVector<QString>& text_content
                               ) ;
    QVector<QMap<QString, QVariant>> 
    get_recognition_results( int camera_id
                         , const QDateTime& startTime = QDateTime()
                         , const QDateTime& endTime = QDateTime()
                         , int limit = 100
                         );

    bool add_inf_result( int cam_id
                       , const QString& keywords
                       , const QString& inf_result
                       );
    QVector<QMap<QString, QVariant>> 
    get_inf_results( int cam_id
                   , int limit = 100
                   );
    
signals:
    void database_error(const QString& error);
    void operation_completed( const QString& operation
                            , bool success
                            );
    
private:
    explicit db_manager(QObject* parent = nullptr);
    ~db_manager();
    
    db_manager(const db_manager&) = delete;
    db_manager& operator=(const db_manager&) = delete;
    
    bool ensure_tables_exist();
    bool create_rtsp_config_table();
    bool create_recognition_results_table();
    bool create_recognition_result_table();

    QSqlDatabase m_db;
    QMutex m_mutex;
    bool m_connected;
};

#endif
