
#ifdef DB_MANAGER_H

#include <iterator>
#include <qmap.h>
#include <qobject.h>
#include <qshareddata.h>
#include <qsqlquery.h>
#include <qthread.h>
#include <qvariant.h>
#include <utils/db_manager.h>

#include <QUuid>

database_connection::database_connection()
        : m_connected{false}
        { m_connection_name = QUuid::createUuid().toString(); }


database_connection::~database_connection() { close(); }


bool database_connection::connect( const QString& driver, const QString& host
                                 , const QString& db_name, const QString& user
                                 , const QString& password, int port)
                                 {
    m_db = QSqlDatabase::addDatabase(driver, m_connection_name);
    m_db.setHostName(host);
    m_db.setDatabaseName(db_name);
    m_db.setUserName(user);
    m_db.setPassword(password);
    m_db.setPort(port);
    
    m_connected = m_db.open();
    if (!m_connected) { qWarning() << "Failed to connect to database: " << m_db.lastError().text(); }

    return m_connected;
}

bool database_connection::is_connected() const { return m_db.isOpen() && m_connected; }

void database_connection::close() {
    if (m_connected) {
        m_db.close();
        m_connected = false;
    }
    
    if (QSqlDatabase::contains(m_connection_name)) {
        QSqlDatabase::removeDatabase(m_connection_name);
    }
}


QSqlQuery database_connection::exec_query( const QString& query
                                         , const QMap<QString, QVariant>& params
                                         ) {
    QSqlQuery sql_query(m_db);
    sql_query.prepare(query);

    QMapIterator<QString, QVariant> i(params);
    while (i.hasNext()) {
        i.next();
        sql_query.bindValue(i.key(), i.value());
    }

    if (!sql_query.exec()) {
        qWarning() << "Query failed: " << sql_query.lastError().text();
        qWarning() << "Query was: " << query;
    }

    return sql_query;
}


QSqlQuery database_connection::prepare_query(const QString& query) {
    QSqlQuery sql_query(m_db);
    sql_query.prepare(query);
    return sql_query;
}

bool database_connection::execute_non_query( const QString& query
                                        , const QMap<QString, QVariant>& params
                                        ) {
    QSqlQuery sql_query = exec_query(query, params);
    return !sql_query.lastError().isValid();
}


db_manager::db_manager(QObject* parent)
    : QObject{parent} 
    , m_initialized{false}
    , m_max_poolsize{10} { }

db_manager::~db_manager(){ close_all(); }

db_manager& db_manager::instance() {
    static db_manager instance;
    return instance;
} 

bool db_manager::initialize( const QString& driver
                           , const QString& host
                           , const QString& db_name
                           , const QString& user
                           , const QString& password
                           , int port
                           ) {
    QMutexLocker locker(&m_pool_mutex);
    
    if (m_initialized) {
        qDebug() << "Database already initialized";
        return true;
    }

    m_driver = driver;
    m_host = host;
    m_db_name = db_name;
    m_user = user;
    m_password = password;
    m_port = port;

    auto conn = QSharedPointer<database_connection>(new database_connection());
    if (!conn->connect(driver, host, db_name, user, password, port)) {
        emit database_error("Failed to initialize database connection");
        return false;
    }

    bool table_created = ensure_tables_exist();

    m_connection_pool.append(conn);
    m_initialized = table_created;

    if (m_initialized) { qDebug() << "Database initialized successfully!"; }

    return m_initialized;
}


bool db_manager::is_initialized() const { return m_initialized; }

QSharedPointer<database_connection> db_manager::get_connection() {
    QMutexLocker locker(&m_pool_mutex);
    
    if (!m_initialized) {
        qWarning() << "Database not intitialized";
        return nullptr;
    }

    for (auto& conn: m_connection_pool) {
        if (!conn.isNull() && conn->is_connected()) {
            QSharedPointer<database_connection> result = conn;
            m_connection_pool.removeOne(conn);
            return result;
        }
    }

    if (m_connection_pool.size() < m_max_poolsize) {
        auto new_conn = QSharedPointer<database_connection>(new database_connection());
        if (new_conn->connect(m_driver, m_host, m_db_name, m_user, m_password, m_port)) {
            return new_conn;
        } else emit database_error("Failed to create new database conneciton");
    }

    locker.unlock();
    QThread::msleep(100);
    return get_connection();
}

#endif
