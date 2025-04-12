
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qobject.h>
#include <qstringview.h>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <utils/common.h>

class http_server: public QObject {
    Q_OBJECT
public:
    http_server(const http_server&) = delete;
    http_server& operator=(const http_server&) = delete; 

    static http_server& instance() {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        
        static http_server instance;
        return instance;
    }
 
    void send_hello(); 
    
    // new rtsp source by ip
    void add_rtsp_source( const QString& rtsp_type
                        , const QString& rtsp_username
                        , const QString& rtsp_ip
                        , int   rtsp_port
                        , const QString& rtsp_channel
                        , const QString& rtsp_subtype
                        , const QString& rtsp_url
                        , const QString& rtsp_name = "无"
                        );

    // update cropped coords
    void update_cropped_coords( int cam_id
                              , float x, float y
                              , float dx, float dy
                              );
    // fetch all rtsp_stream_info
    void fetch_all_rtsp_stream_info();

    // fetch server status info
    void check_server_status();

    void start_status_check(int interval_ms = 10000) {
        if (!m_status_check_timer) {
            m_status_check_timer = new QTimer(this);
            connect(m_status_check_timer, &QTimer::timeout, this, &http_server::check_server_status);
        }
        m_status_check_timer->start(interval_ms);
    }

    void stop_status_check() {
        if (m_status_check_timer && m_status_check_timer->isActive())
            m_status_check_timer->stop();
    }

    bool is_server_online() const { return m_server_online; }
    QString get_server_url() const { return m_api_route_header; }
    void set_server_url(const QString& url) {
        m_api_route_header = url;
        check_server_status();
    }
    
signals:
    void request_finished(QNetworkReply* reply);
    void rtsp_source_added(int rtsp_id);    
    void send_all_rtsp_stream_info(const QVector<rtsp_config>& rtsp_configs);
    void server_status_changed(bool online);

private:
    QNetworkAccessManager* m_network_mgr;
    QString m_api_route_header{"http://localhost:8080"};
    bool m_server_online{false};
    QTimer* m_status_check_timer{nullptr};
    
    

    explicit http_server(QObject* parent = nullptr)
        : QObject{parent}
        {
        m_network_mgr = new QNetworkAccessManager{this};
        connect(m_network_mgr, &QNetworkAccessManager::finished, this, &http_server::request_finished);
        
        QTimer::singleShot(0, this, &http_server::check_server_status);
    }
    ~http_server() {
        if (m_status_check_timer) {
            m_status_check_timer->stop();
            delete m_status_check_timer;
        }
    }
};


#endif // HTTP_SERVER_H
