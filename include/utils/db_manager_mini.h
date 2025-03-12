

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

class DBManager : public QObject {
    Q_OBJECT
    
public:
    // 获取单例实例
    static DBManager& instance();
    
    // 初始化数据库连接
    bool connect(const QString& host, const QString& dbName,
                const QString& user, const QString& password, int port = 5432);
    
    // 关闭数据库连接
    void disconnect();
    
    // 检查连接状态
    bool isConnected() const;
    
    // 执行查询操作
    QSqlQuery executeQuery(const QString& query, const QMap<QString, QVariant>& params = QMap<QString, QVariant>());
    
    // 执行非查询操作（插入、更新、删除）
    bool executeNonQuery(const QString& query, const QMap<QString, QVariant>& params = QMap<QString, QVariant>());
    
    // RTSP相关操作
    bool addRtspConfig(const QString& username, const QString& ip, const QString& port, 
                      const QString& channel, const QString& subtype, const QString& rtsp_url);
    bool updateRtspConfig(int id, const QString& username, const QString& ip, const QString& port,
                         const QString& channel, const QString& subtype, const QString& rtsp_url);
    bool deleteRtspConfig(int id);
    QVector<QMap<QString, QVariant>> getAllRtspConfigs();
    QMap<QString, QVariant> getRtspConfigById(int id);
    
    // 识别结果相关操作
    bool addRecognitionResult(int camera_id, const QVector<QString>& text_content);
    QVector<QMap<QString, QVariant>> getRecognitionResults(int camera_id, 
                                                         const QDateTime& startTime = QDateTime(),
                                                         const QDateTime& endTime = QDateTime(),
                                                         int limit = 100);

    bool addInfResult(int cam_id, const QString& keywords, const QString& inf_result);
    QVector<QMap<QString, QVariant>> getInfResults(int cam_id, int limit = 100);
    
signals:
    void databaseError(const QString& error);
    void operationCompleted(const QString& operation, bool success);
    
private:
    // 私有构造函数 - 单例模式
    explicit DBManager(QObject* parent = nullptr);
    ~DBManager();
    
    // 禁止拷贝和赋值
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;
    
    // 确保表存在
    bool ensureTablesExist();
    bool createRtspConfigTable();
    bool createRecognitionResultsTable();
    
    QSqlDatabase m_db;
    QMutex m_mutex;
    bool m_connected;
};



#endif
