

#include <utils/db_manager_mini.h>


// db_manager.cpp
#include <QDebug>
#include <QSqlRecord>
#include <QDateTime>

DBManager::DBManager(QObject* parent) 
    : QObject(parent), m_connected(false) {
    // 创建数据库连接
    m_db = QSqlDatabase::addDatabase("QPSQL");
}

DBManager::~DBManager() {
    disconnect();
}

DBManager& DBManager::instance() {
    static DBManager instance;
    return instance;
}

bool DBManager::connect(const QString& host, const QString& dbName,
                      const QString& user, const QString& password, int port) {
    // QMutexLocker locker(&m_mutex);
    
    if (m_connected) {
        qDebug() << "Database already connected";
        return true;
    }
    
    // 设置连接参数
    m_db.setHostName(host);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    m_db.setPort(port);
    
    // 尝试连接
    if (!m_db.open()) {
        emit databaseError("Failed to connect to database: " + m_db.lastError().text());
        return false;
    }
    
    m_connected = true;
    
    // 确保必要的表存在
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

void DBManager::disconnect() {
    QMutexLocker locker(&m_mutex);
    
    if (m_connected) {
        m_db.close();
        m_connected = false;
        qDebug() << "Database disconnected";
    }
}

bool DBManager::isConnected() const {
    return m_connected && m_db.isOpen();
}

QSqlQuery DBManager::executeQuery(const QString& query, const QMap<QString, QVariant>& params) {
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
        emit databaseError("Query failed: " + sqlQuery.lastError().text());
    }
    
    return sqlQuery;
}

bool DBManager::executeNonQuery(const QString& query, const QMap<QString, QVariant>& params) {
    QSqlQuery sqlQuery = executeQuery(query, params);
    return !sqlQuery.lastError().isValid();
}

bool DBManager::ensureTablesExist() {
    return createRtspConfigTable() && createRecognitionResultsTable();
}

bool DBManager::createRtspConfigTable() {
    return executeNonQuery(
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

bool DBManager::createRecognitionResultsTable() {
    return executeNonQuery(
        "CREATE TABLE IF NOT EXISTS recognition_results ("
        "id SERIAL PRIMARY KEY, "
        "camera_id INTEGER, "
        "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "text_content TEXT[], "
        "CONSTRAINT fk_camera FOREIGN KEY (camera_id) REFERENCES rtsp_config(id) ON DELETE CASCADE"
        ")"
    );
}

bool DBManager::addRtspConfig(const QString& username, const QString& ip, const QString& port,
                            const QString& channel, const QString& subtype, const QString& rtsp_url) {
    if (!isConnected()) {
        emit databaseError("Database not connected");
        return false;
    }
    
    QMap<QString, QVariant> params;
    params[":username"] = username;
    params[":ip"] = ip;
    params[":port"] = port;
    params[":channel"] = channel;
    params[":subtype"] = subtype;
    params[":rtsp_url"] = rtsp_url;
    
    QSqlQuery query = executeQuery(
        "INSERT INTO rtsp_config (username, ip, port, channel, subtype, rtsp_url) "
        "VALUES (:username, :ip, :port, :channel, :subtype, :rtsp_url) RETURNING id",
        params
    );
    
    bool success = query.next();
    
    if (success) {
        emit operationCompleted("Add RTSP Config", true);
    } else {
        emit databaseError("Failed to add RTSP configuration");
    }
    
    return success;
}

bool DBManager::updateRtspConfig(int id, const QString& username, const QString& ip, const QString& port,
                               const QString& channel, const QString& subtype, const QString& rtsp_url) {
    if (!isConnected()) {
        emit databaseError("Database not connected");
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
    
    bool success = executeNonQuery(
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
        emit operationCompleted("Update RTSP Config", true);
    } else {
        emit databaseError("Failed to update RTSP configuration");
    }
    
    return success;
}

bool DBManager::deleteRtspConfig(int id) {
    if (!isConnected()) {
        emit databaseError("Database not connected");
        return false;
    }
    
    QMap<QString, QVariant> params;
    params[":id"] = id;
    
    bool success = executeNonQuery("DELETE FROM rtsp_config WHERE id = :id", params);
    
    if (success) {
        emit operationCompleted("Delete RTSP Config", true);
    } else {
        emit databaseError("Failed to delete RTSP configuration");
    }
    
    return success;
}

QVector<QMap<QString, QVariant>> DBManager::getAllRtspConfigs() {
    QVector<QMap<QString, QVariant>> result;
    
    if (!isConnected()) {
        emit databaseError("Database not connected");
        return result;
    }
    
    QSqlQuery query = executeQuery("SELECT * FROM rtsp_config ORDER BY id");
    
    while (query.next()) {
        QMap<QString, QVariant> row;
        for (int i = 0; i < query.record().count(); i++) {
            row[query.record().fieldName(i)] = query.value(i);
        }
        result.append(row);
    }
    
    return result;
}

QMap<QString, QVariant> DBManager::getRtspConfigById(int id) {
    QMap<QString, QVariant> result;
    
    if (!isConnected()) {
        emit databaseError("Database not connected");
        return result;
    }
    
    QMap<QString, QVariant> params;
    params[":id"] = id;
    
    QSqlQuery query = executeQuery("SELECT * FROM rtsp_config WHERE id = :id", params);
    
    if (query.next()) {
        for (int i = 0; i < query.record().count(); i++) {
            result[query.record().fieldName(i)] = query.value(i);
        }
    }
    
    return result;
}

bool DBManager::addRecognitionResult(int camera_id, const QVector<QString>& text_content) {
    if (!isConnected()) {
        emit databaseError("Database not connected");
        return false;
    }
    
    // 将文本内容转换为PostgreSQL数组格式
    QString textArray = "{";
    for (int i = 0; i < text_content.size(); ++i) {
        if (i > 0) textArray += ",";
        //textArray += "\"" + text_content[i].replace("\"", "\"\"") + "\"";
    }
    textArray += "}";
    
    QMap<QString, QVariant> params;
    params[":camera_id"] = camera_id;
    params[":text_content"] = textArray;
    
    bool success = executeNonQuery(
        "INSERT INTO recognition_results (camera_id, text_content) "
        "VALUES (:camera_id, :text_content::text[])",
        params
    );
    
    if (!success) {
        emit databaseError("Failed to add recognition result");
    }
    
    return success;
}

QVector<QMap<QString, QVariant>> DBManager::getRecognitionResults(int camera_id,
                                                               const QDateTime& startTime,
                                                               const QDateTime& endTime,
                                                               int limit) {
    QVector<QMap<QString, QVariant>> result;
    
    if (!isConnected()) {
        emit databaseError("Database not connected");
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
    
    QSqlQuery query = executeQuery(queryStr, params);
    
    while (query.next()) {
        QMap<QString, QVariant> row;
        for (int i = 0; i < query.record().count(); i++) {
            row[query.record().fieldName(i)] = query.value(i);
        }
        result.append(row);
    }
    
    return result;
}


bool DBManager::addInfResult(int cam_id, const QString& keywords, const QString& inf_result) {
    if (!isConnected()) {
        emit databaseError("Database not connected");
        return false;
    }
    
    QMap<QString, QVariant> params;
    params[":cam_id"] = cam_id;
    params[":keywords"] = keywords;
    params[":inf_result"] = inf_result;
    
    bool success = executeNonQuery(
        "INSERT INTO inf_qwq_result (cam_id, keywords, inf_result) "
        "VALUES (:cam_id, :keywords, :inf_result)",
        params
    );
    
    if (!success) {
        emit databaseError("Failed to add inference result");
    } else {
        emit operationCompleted("Add Inference Result", true);
    }
    
    return success;
}

QVector<QMap<QString, QVariant>> DBManager::getInfResults(int cam_id, int limit) {
    QVector<QMap<QString, QVariant>> result;
    
    if (!isConnected()) {
        emit databaseError("Database not connected");
        return result;
    }
    
    QMap<QString, QVariant> params;
    params[":cam_id"] = cam_id;
    params[":limit"] = limit;
    
    QSqlQuery query = executeQuery(
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
