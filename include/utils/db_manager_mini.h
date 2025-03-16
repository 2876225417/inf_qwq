

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
#include <qcontainerfwd.h>
#include <qdatetime.h>

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
   

    bool database_initialize();

    void disconnect();
    
    bool is_connected() const;
    
    QSqlQuery execute_query( const QString& query
                           , const QMap<QString, QVariant>& params 
                             = QMap<QString, QVariant>()
                           );    
    
    bool execute_non_query( const QString& query
                          , const QMap<QString, QVariant>& params 
                            = QMap<QString, QVariant>()
                          );
   
    bool check_rtsp_url_exists(const QString& rtsp_url);

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

    int add_warning_record( int cam_id
                           , const QString& cam_name
                           , const QString& inf_res
                           , bool status
                           , const QString& keywords
                           , const QString& rtsp_name
                           , const QString& rtsp_url
                           );
       
    bool update_warning_record_push_status( int id
                                           , bool status
                                           , const QString& message = QString()
                                           );


    bool delete_rtsp_config(int id);

    QVector<QMap<QString, QVariant>> get_all_rtsp_configs();
    QMap<QString, QVariant> get_rtsp_config_by_id(int id);
    
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

    QVector<QMap<QString, QVariant>>
    get_warning_records( int cam_id = -1
                       , bool only_abnormal = false
                       , const QDateTime& start_time = QDateTime()
                       , const QDateTime& end_time = QDateTime()
                       , int limit = 100
                       , int offset = 0
                       , const QString& keyword = QString()
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
    bool create_warning_records_table();

    QSqlDatabase m_db;
    QMutex m_mutex;
    bool m_connected;
};

#endif
