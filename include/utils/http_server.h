
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
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qobject.h>
#include <qstringview.h>

class http_server: public QObject {
    Q_OBJECT
public:
    explicit http_server(QObject* parent = nullptr) 
        : QObject{parent}
        {
        m_network_mgr = new QNetworkAccessManager(this);
        connect (m_network_mgr, &QNetworkAccessManager::finished, this, &http_server::request_finished);
    }
    void send_hello() {
        QUrl url{m_api_route_header + "/inf_qwq/update_cropped_coords"};
        QNetworkRequest request{url};
        
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject json_obj;
        json_obj["message"] = "hello";
    
        QJsonDocument json_doc(json_obj);
        QByteArray data = json_doc.toJson();

        m_network_mgr->post(request, data);
    }
    
    // new rtsp source by ip
    void add_rtsp_source( const QString& rtsp_type
                        , const QString& rtsp_username
                        , const QString& rtsp_ip
                        , int   rtsp_port
                        , const QString& rtsp_channel
                        , const QString& rtsp_subtype
                        , const QString& rtsp_url
                        , const QString& rtsp_name = "无"
                        ) {
        QUrl url{m_api_route_header + "/inf_qwq/add_rtsp_source"};
        
        QNetworkRequest request{url};
        
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject json_obj;
        
        json_obj["rtsp_type"] = rtsp_type;
        json_obj["rtsp_username"] = rtsp_username;
        json_obj["rtsp_ip"] = rtsp_ip;
        json_obj["rtsp_port"] = rtsp_port;
        json_obj["rtsp_channel"] = rtsp_channel;
        json_obj["subtype"] = rtsp_subtype;
        json_obj["rtsp_url"] = rtsp_url;
        json_obj["rtsp_name"] = rtsp_name;

        QJsonDocument json_doc{json_obj};
        QByteArray data = json_doc.toJson();
        qDebug() << "Sending configs...";    

        QNetworkReply* reply = m_network_mgr->post(request, data);
        // wait for response connect signals

    }
    // fetch all rtsp sources

    // fetch all inf results

    // update cropped coords
    void update_cropped_coords(int cam_id, float x, float y, float dx, float dy);    
    
    // 
    

signals:
    void request_finished(QNetworkReply* reply);
private:
    QNetworkAccessManager* m_network_mgr;
    QString m_api_route_header{"http://localhost:8080"};
};


#endif // HTTP_SERVER_H
