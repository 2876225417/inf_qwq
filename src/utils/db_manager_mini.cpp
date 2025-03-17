

#include <qsqlquery.h>
#include <utils/db_manager_mini.h>
#include <QSettings>


// db_manager.cpp
#include <QDebug>
#include <QSqlRecord>
#include <QDateTime>

db_manager::db_manager(QObject* parent) 
    : QObject(parent), m_connected(false) {
    m_db = QSqlDatabase::addDatabase("QPSQL");
}

db_manager::~db_manager() {
    disconnect();
}

db_manager& db_manager::instance() {
    static db_manager instance;
    return instance;
}

bool db_manager::database_initialize() {
    QSettings settings("Chun Hui", "inf_qwq");
    QString host = settings.value("Database/Host", "localhost").toString();
    QString db_name = settings.value("Database/Database", "").toString();
    QString user = settings.value("Database/Username", "postgres").toString();
    QString password = settings.value("Database/Password").toString();
    int port = settings.value("Database/Port", 5432).toInt();

    bool success =  db_manager::instance().connect(host, db_name, user, password, port);

    if (success) qDebug() << "Database connection intialized successfully!";
    else qDebug() << "Failed to initialize database connection";
    
    emit database_conn_status(success);
    return success;
}

bool db_manager::connect( const QString& host
                        , const QString& db_name
                        , const QString& user
                        , const QString& password
                        , int port
                        ) { 
    if (m_connected) {
        qDebug() << "Database already connected";
        return true;
    }
    
    m_db.setHostName(host);
    m_db.setDatabaseName(db_name);
    m_db.setUserName(user);
    m_db.setPassword(password);
    m_db.setPort(port);
    
    if (!m_db.open()) {
        emit database_error("Failed to connect to database: " + m_db.lastError().text());
        return false;
        emit database_conn_status(false);
    }
     
    m_connected = true;
    emit database_conn_status(m_connected);
    //bool tablesCreated = ensureTablesExist();
    bool tablesCreated = true;
    if (tablesCreated) {
        qDebug() << "Database connected successfully";
    } else {
        disconnect();
        return false;
    }
    
    return m_connected;
}

void db_manager::disconnect() {
    QMutexLocker locker(&m_mutex);
    
    if (m_connected) {
        m_db.close();
        m_connected = false;
        qDebug() << "Database disconnected";
    }
}

bool db_manager::is_connected() const {
    
    return m_connected && m_db.isOpen();
}

QSqlQuery db_manager::execute_query(const QString& query, const QMap<QString, QVariant>& params) {
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery sqlQuery(m_db);
    sqlQuery.prepare(query);
    
    // 绑定参数
    QMapIterator<QString, QVariant> i(params);
    while (i.hasNext()) {
        i.next();
        sqlQuery.bindValue(i.key(), i.value());
    }
    
    if (!sqlQuery.exec()) {
        qWarning() << "Query failed:" << sqlQuery.lastError().text();
        qWarning() << "Query was:" << query;
        emit database_error("Query failed: " + sqlQuery.lastError().text());
    }
    
    return sqlQuery;
}

bool db_manager::execute_non_query(const QString& query, const QMap<QString, QVariant>& params) {
    QSqlQuery sqlQuery = execute_query(query, params);
    return !sqlQuery.lastError().isValid();
}

bool db_manager::ensure_tables_exist() {
    return create_rtsp_config_table() && create_recognition_results_table();
}

bool db_manager::create_rtsp_config_table() {
    return execute_non_query(
        "CREATE TABLE IF NOT EXISTS rtsp_config ("
        "id SERIAL PRIMARY KEY, "
        "username VARCHAR(50) NOT NULL, "
        "ip VARCHAR(50) NOT NULL, "
        "port VARCHAR(10) NOT NULL, "
        "channel VARCHAR(50) NOT NULL, "
        "subtype VARCHAR(50), "
        "rtsp_url VARCHAR(255) NOT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")"
    );
}

bool db_manager::create_recognition_results_table() {
    return execute_non_query(
        "CREATE TABLE IF NOT EXISTS recognition_results ("
        "id SERIAL PRIMARY KEY, "
        "camera_id INTEGER, "
        "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "text_content TEXT[], "
        "CONSTRAINT fk_camera FOREIGN KEY (camera_id) REFERENCES rtsp_config(id) ON DELETE CASCADE"
        ")"
    );
}

bool db_manager::check_rtsp_url_exists(const QString& rtsp_url) {
    if (!is_connected()) {
        emit database_error("Database not connected");
        return false;
    }

    QMap<QString, QVariant> params;
    params[":rtsp_url"] = rtsp_url;

    QSqlQuery query = execute_query(
        "SELECT COUNT(*) FROM rtsp_sources WHERE rtsp_url = :rtsp_url",
        params
    );

    if (query.next()) {
        int count = query.value(0).toInt();
        qDebug() << "count: " << count;
        return count > 0;
    }
    return false;
}

bool db_manager::add_rtsp_config( const QString& username
                                , const QString& ip
                                , const QString& port
                                , const QString& channel
                                , const QString& subtype
                                , const QString& rtsp_url
                                ) {
    if (!is_connected()) {
        emit database_error("Database not connected");
        return false;
    }
    
    if (check_rtsp_url_exists(rtsp_url)) {
        emit operation_completed("RTSP URL already exists", false);
        return false;
    }

    QMap<QString, QVariant> params;
    params[":username"] = username;
    params[":ip"] = ip;
    params[":port"] = port;
    params[":channel"] = channel;
    params[":subtype"] = subtype;
    params[":rtsp_url"] = rtsp_url;
    
    QSqlQuery query = execute_query(
        "INSERT INTO rtsp_sources (username, ip, port, channel, subtype, rtsp_url) "
        "VALUES (:username, :ip, :port, :channel, :subtype, :rtsp_url) RETURNING id",
        params
    );
    
    bool success = query.next();
    
    if (success) {
        emit operation_completed("Add RTSP Config", true);
    } else {
        emit database_error("Failed to add RTSP configuration");
    }
    
    return success;
}

bool db_manager::update_rtsp_config( int id
                                   , const QString& username
                                   , const QString& ip
                                   , const QString& port
                                   , const QString& channel
                                   , const QString& subtype
                                   , const QString& rtsp_url
                                   ) {
    if (!is_connected()) {
        emit database_error("Database not connected");
        return false;
    }
    
    QMap<QString, QVariant> params;
    params[":id"] = id;
    params[":username"] = username;
    params[":ip"] = ip;
    params[":port"] = port;
    params[":channel"] = channel;
    params[":subtype"] = subtype;
    params[":rtsp_url"] = rtsp_url;
    
    bool success = execute_non_query(
        "UPDATE rtsp_config SET "
        "username = :username, "
        "ip = :ip, "
        "port = :port, "
        "channel = :channel, "
        "subtype = :subtype, "
        "rtsp_url = :rtsp_url, "
        "updated_at = CURRENT_TIMESTAMP "
        "WHERE id = :id",
        params
    );
    
    if (success) {
        emit operation_completed("Update RTSP Config", true);
    } else {
        emit database_error("Failed to update RTSP configuration");
    }
    
    return success;
}

bool db_manager::delete_rtsp_config(int id) {
    if (!is_connected()) {
        emit database_error("Database not connected");
        return false;
    }
    
    QMap<QString, QVariant> params;
    params[":id"] = id;
    
    bool success = execute_non_query("DELETE FROM rtsp_config WHERE id = :id", params);
    
    if (success) {
        emit operation_completed("Delete RTSP Config", true);
    } else {
        emit database_error("Failed to delete RTSP configuration");
    }
    
    return success;
}

QVector<QMap<QString, QVariant>> db_manager::get_all_rtsp_configs() {
    QVector<QMap<QString, QVariant>> result;
    
    if (!is_connected()) {
        emit database_error("Database not connected");
        qDebug() << "Database not connected";
        return result;
    }
 
    QSqlQuery query = execute_query("SELECT * FROM rtsp_sources ORDER BY id");
    
    while (query.next()) {
        QMap<QString, QVariant> row;
        for (int i = 0; i < query.record().count(); i++) {
            row[query.record().fieldName(i)] = query.value(i);
        }
        result.append(row);
    }
    
    return result;
}

QMap<QString, QVariant> db_manager::get_rtsp_config_by_id(int id) {
    QMap<QString, QVariant> result;
    
    if (!is_connected()) {
        emit database_error("Database not connected");
        return result;
    }
    
    QMap<QString, QVariant> params;
    params[":id"] = id;
    
    QSqlQuery query = execute_query("SELECT * FROM rtsp_config WHERE id = :id", params);
    
    if (query.next()) {
        for (int i = 0; i < query.record().count(); i++) {
            result[query.record().fieldName(i)] = query.value(i);
        }
    }
    
    return result;
}

bool db_manager::add_recognition_result(int camera_id, const QVector<QString>& text_content) {
    if (!is_connected()) {
        emit database_error("Database not connected");
        return false;
    }
    
    QString textArray = "{";
    for (int i = 0; i < text_content.size(); ++i) {
        if (i > 0) textArray += ",";
        //textArray += "\"" + text_content[i].replace("\"", "\"\"") + "\"";
    }
    textArray += "}";
    
    QMap<QString, QVariant> params;
    params[":camera_id"] = camera_id;
    params[":text_content"] = textArray;
    
    bool success = execute_non_query(
        "INSERT INTO recognition_results (camera_id, text_content) "
        "VALUES (:camera_id, :text_content::text[])",
        params
    );
    
    if (!success) {
        emit database_error("Failed to add recognition result");
    }
    
    return success;
}

QVector<QMap<QString, QVariant>> db_manager::get_recognition_results(int camera_id,
                                                               const QDateTime& startTime,
                                                               const QDateTime& endTime,
                                                               int limit) {
    QVector<QMap<QString, QVariant>> result;
    
    if (!is_connected()) {
        emit database_error("Database not connected");
        return result;
    }
    
    QString queryStr = "SELECT * FROM recognition_results WHERE camera_id = :camera_id";
    QMap<QString, QVariant> params;
    params[":camera_id"] = camera_id;
    
    if (startTime.isValid()) {
        queryStr += " AND timestamp >= :start_time";
        params[":start_time"] = startTime;
    }
    
    if (endTime.isValid()) {
        queryStr += " AND timestamp <= :end_time";
        params[":end_time"] = endTime;
    }
    
    queryStr += " ORDER BY timestamp DESC LIMIT :limit";
    params[":limit"] = limit;
    
    QSqlQuery query = execute_query(queryStr, params);
    
    while (query.next()) {
        QMap<QString, QVariant> row;
        for (int i = 0; i < query.record().count(); i++) {
            row[query.record().fieldName(i)] = query.value(i);
        }
        result.append(row);
    }
    
    return result;
}


bool db_manager::add_inf_result(int cam_id, const QString& keywords, const QString& inf_result) {
    if (!is_connected()) {
        emit database_error("Database not connected");
        return false;
    }
    
    QMap<QString, QVariant> params;
    params[":cam_id"] = cam_id;
    params[":keywords"] = keywords;
    params[":inf_result"] = inf_result;
    
    bool success = execute_non_query(
        "INSERT INTO inf_qwq_result (cam_id, keywords, inf_result) "
        "VALUES (:cam_id, :keywords, :inf_result)",
        params
    );
    
    if (!success) {
        emit database_error("Failed to add inference result");
    } else {
        emit operation_completed("Add Inference Result", true);
    }
    
    return success;
}

QVector<QMap<QString, QVariant>> db_manager::get_inf_results(int cam_id, int limit) {
    QVector<QMap<QString, QVariant>> result;
    
    if (!is_connected()) {
        emit database_error("Database not connected");
        return result;
    }
    
    QMap<QString, QVariant> params;
    params[":cam_id"] = cam_id;
    params[":limit"] = limit;
    
    QSqlQuery query = execute_query(
        "SELECT * FROM inf_qwq_result "
        "WHERE cam_id = :cam_id "
        "ORDER BY created_at DESC "
        "LIMIT :limit",
        params
    );
    
    while (query.next()) {
        QMap<QString, QVariant> row;
        for (int i = 0; i < query.record().count(); i++) {
            row[query.record().fieldName(i)] = query.value(i);
        }
        result.append(row);
    }
    
    return result;
}

bool db_manager::create_warning_records_table() {
    return execute_non_query(
        "CREATE TABLE IF NOT EXISTS warning_records ("
        "id SERIAL PRIMARY KEY, "
        "cam_id INTEGER NOT NULL, "
        "cam_name VARCHAR(100), "
        "inf_res TEXT NOT NULL, "
        "status BOOLEAN NOT NULL, "
        "keywords TEXT, "
        "rtsp_name VARCHAR(100), "
        "rtsp_url TEXT NOT NULL, "
        "record_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")"
    );
}

int db_manager::add_warning_record( int cam_id
                                   , const QString& cam_name
                                   , const QString& inf_res
                                   , bool status
                                   , const QString& keywords
                                   , const QString& rtsp_name
                                   , const QString& rtsp_url) 
                                   {
    if (!is_connected()) {
        emit database_error("Database not connected");
        return false;
    }

    QMap<QString, QVariant> params;
    params[":cam_id"] = cam_id;
    params[":cam_name"] = cam_name;
    params[":inf_res"] = inf_res;
    params[":status"] = status;
    params[":keywords"] = keywords;
    params[":rtsp_name"] = rtsp_name;
    params[":rtsp_url"] = rtsp_url;
    params[":push_status"] = false;

    QSqlQuery query = execute_query(
        "INSERT INTO warning_records (cam_id, cam_name, inf_res, status, keywords, rtsp_name, rtsp_url, push_status) "
        "VALUES (:cam_id, :cam_name, :inf_res, :status, :keywords, :rtsp_name, :rtsp_url, :push_status)"
        "RETURNING id",
        params
    );

    int record_id = -1;

    if (query.next()){ record_id = query.value(0).toInt();  emit database_error("Failed to add warning record"); }
    else operation_completed("Add Warning Record", true);
    
    return record_id;
}


QVector<QMap<QString, QVariant>> db_manager::get_warning_records( int cam_id
                                                                , bool onlyAbnormal
                                                                , const QDateTime& startTime
                                                                , const QDateTime& endTime
                                                                , int limit
                                                                , int offset
                                                                , const QString& keyword
                                                                ) {
    QVector<QMap<QString, QVariant>> result;
    
    if (!is_connected()) {
        emit database_error("Database not connected");
        return result;
    }
    
    QString queryStr = "SELECT * FROM warning_records WHERE 1=1";
    QMap<QString, QVariant> params;
    
    if (cam_id >= 0) {
        queryStr += " AND cam_id = :cam_id";
        params[":cam_id"] = cam_id;
    }
    
    if (onlyAbnormal) {
        queryStr += " AND status = false";
    }
    
    if (startTime.isValid()) {
        queryStr += " AND record_time >= :start_time";
        params[":start_time"] = startTime;
    }
    
    if (endTime.isValid()) {
        queryStr += " AND record_time <= :end_time";
        params[":end_time"] = endTime;
    }
    
    if (!keyword.isEmpty()) {
        queryStr += " AND (keywords LIKE :keyword OR inf_res LIKE :keyword)";
        params[":keyword"] = "%" + keyword + "%";
    }
    
    queryStr += " ORDER BY record_time DESC LIMIT :limit OFFSET :offset";
    params[":limit"] = limit;
    params[":offset"] = offset;
    
    QSqlQuery query = execute_query(queryStr, params);
    
    while (query.next()) {
        QMap<QString, QVariant> row;
        for (int i = 0; i < query.record().count(); i++) {
            row[query.record().fieldName(i)] = query.value(i);
        }
        result.append(row);
    }
    
    return result;
}



bool db_manager::update_warning_record_push_status(int id, bool status, const QString& message) {
    if (!is_connected()) {
        emit database_error("Database not connected");
        return false;
    }
    
    QMap<QString, QVariant> params;
    params[":id"] = id;
    params[":status"] = status;
    params[":message"] = message;
    
    bool success = execute_non_query(
        "UPDATE warning_records SET push_status = :status, push_message = :message WHERE id = :id",
        params
    );
    
    if (!success) {
        emit database_error("Failed to update warning record push status");
    }
    
    return success;
}
