
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
#include <qjsonobject.h>
#include <qobject.h>

class http_server: public QObject {
    Q_OBJECT
public:
    explicit http_server(QObject* parent = nullptr) 
        : QObject{parent}
        {
        m_network_mgr = new QNetworkAccessManager(this);
        connect (m_network_mgr, &QNetworkAccessManager::finished, this, &http_server::request_finished);
    }
    void send_hello(float x, float y, float dx, float dy) {
        QUrl url{"http://localhost:8080/inf_qwq/update_cropped_coords"};
        QNetworkRequest request{url};
        
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject json_obj;
        json_obj["message"] = "hello";
    
        QJsonDocument json_doc(json_obj);
        QByteArray data = json_doc.toJson();

        m_network_mgr->post(request, data);
    }

    void update_cropped_coords(int cam_id, float x, float y, float dx, float dy);    
    
    
    

signals:
    void request_finished(QNetworkReply* reply);
private:
    QNetworkAccessManager* m_network_mgr;
};


#endif // HTTP_SERVER_H
