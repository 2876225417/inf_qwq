

#include <qjsondocument.h>
#include <utils/http_server.h>




void http_server::send_hello() {
        QUrl url{m_api_route_header + "/inf_qwq/update_cropped_coords"};
        QNetworkRequest request{url};
        
       request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject json_obj;
        json_obj["message"] = "hello";
    
        QJsonDocument json_doc(json_obj);
        QByteArray data = json_doc.toJson();

        m_network_mgr->post(request, data);
}

void http_server::add_rtsp_source( const QString& rtsp_type
                                 , const QString& rtsp_username
                                 , const QString& rtsp_ip
                                 , int   rtsp_port
                                 , const QString& rtsp_channel
                                 , const QString& rtsp_subtype
                                 , const QString& rtsp_url
                                 , const QString& rtsp_name
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
        connect (reply, &QNetworkReply::finished, [this, reply]() {
            QByteArray response_data = reply->readAll();
            QJsonDocument response_doc = QJsonDocument::fromJson(response_data);

            if (!response_doc.isNull() && response_doc.isObject()) {
                QJsonObject response_obj = response_doc.object();
                if (response_obj.contains("success") && response_obj["success"].toBool()) {
                    int rtsp_id = response_obj["rtsp_id"].toInt();
                    qDebug() << "added rtsp source id: " << rtsp_id;
                    emit rtsp_source_added(rtsp_id);
                    qDebug() << "Added rtsp id: " << rtsp_id; 
                } else {
                    int existed_rtsp_id = response_obj["existing_rtsp_id"].toInt();
                    emit rtsp_source_added(existed_rtsp_id);
                    qDebug() << "Existed rtsp id: " << existed_rtsp_id;
                } 
            }
            reply->deleteLater();
        });
}

void http_server::update_cropped_coords( int cam_id
                          , float x, float y
                          , float dx, float dy
                          ) {
        QUrl url{m_api_route_header + "/inf_qwq/update_cropped_coords"};
        QNetworkRequest request{url};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject json_obj;
        json_obj["rtsp_id"] = cam_id;
        json_obj["x"] = x;
        json_obj["y"] = y;
        json_obj["dx"] = dx;
        json_obj["dy"] = dy;
        
        QJsonDocument json_doc(json_obj);
        QByteArray data = json_doc.toJson();
        
        QNetworkReply* reply = m_network_mgr->post(request, data);
        connect (reply, &QNetworkReply::finished, [this, reply, cam_id](){
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response_data = reply->readAll();
                QJsonDocument response_doc = QJsonDocument::fromJson(response_data);

                if (!response_doc.isNull() && response_doc.isObject()) {
                    QJsonObject response_obj = response_doc.object();
                    if (response_obj.contains("success") && response_obj["success"].toBool()) {
                        // emit somthing
                        qDebug() << "Updated cropped coordinates for camera ID: " << cam_id; 
                    } else {
                        QString error = response_obj.contains("error") ? response_obj["error"].toString() : "Unknown error";
                        // emit something error
                        qDebug() << "Failed to update cropped coordinates: " << error;
                    }
                } 
            } else {
                    // emit somthing error
                    qDebug() << "Network error: " << reply->errorString();
            }
            reply->deleteLater();
        });
}

void http_server::fetch_all_rtsp_stream_info() {
     QUrl url{m_api_route_header + "/inf_qwq/get_all_rtsp_sources"};
        QNetworkRequest request{url};

        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* reply = m_network_mgr->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            QVector<rtsp_config> configs;

            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response_data = reply->readAll();
                QJsonDocument response_doc = QJsonDocument::fromJson(response_data);
                
                if (!response_doc.isNull() && response_doc.isObject()) {
                    QJsonObject response_obj = response_doc.object();

                    if (response_obj.contains("success") && response_obj["success"].toBool() &&
                        response_obj.contains("rtsp_sources") && response_obj["rtsp_sources"].isArray()
                        ) {
                        QJsonArray sources_array = response_obj["rtsp_sources"].toArray();

                        for (const QJsonValue& value: sources_array) {
                            if (value.isObject()) {
                                QJsonObject source = value.toObject();
                                
                                rtsp_config config;
                                config.rtsp_id = source["rtsp_id"].toInt();
                                config.rtsp_name = source["rtsp_name"].toString();
                                config.username = source["rtsp_username"].toString();
                                config.password = "";
                                config.ip = source["rtsp_ip"].toString();
                                config.port = QString::number(source["rtsp_port"].toInt());
                                config.channel = source["rtsp_channel"].toString();
                                config.subtype = source["rtsp_subtype"].toString();
                                config.rtsp_url = source["rtsp_url"].toString();

                                QString rtsp_type = source["rtsp_type"].toString().toLower();
                                if (rtsp_type == "hkvision") config.rpt = rtsp_proto_type::HIKVISION;
                                else if (rtsp_type == "dahua") config.rpt = rtsp_proto_type::ALHUA;

                                if (source.contains("rtsp_crop_coord_x") && !source["rtsp_crop_coord_x"].isNull()) 
                                    config.cropped_pos.cropped_x = source["rtsp_crop_coord_x"].toDouble();
                                if (source.contains("rtsp_crop_coord_y") && !source["rtsp_crop_coord_y"].isNull())
                                    config.cropped_pos.cropped_y = source["rtsp_crop_coord_y"].toDouble();
                                if (source.contains("rtsp_crop_coord_dx") && !source["rtsp_crop_coord_dx"].isNull())
                                    config.cropped_pos.cropped_dx = source["rtsp_crop_coord_dx"].toDouble();
                                if (source.contains("rtsp_crop_coord_dy") && !source["rtsp_crop_coord_dy"].isNull())
                                    config.cropped_pos.cropped_dy = source["rtsp_crop_coord_dy"].toDouble();
                                configs.push_back(config);
                            }
                        }
                        qDebug() << "Fetched: " << configs.size() << "rtsp configs";
                        emit send_all_rtsp_stream_info(configs);
                    } else { }// emit something error }
                } else { }// emit something error }
            } else { } // emit something error }
            reply->deleteLater();
        });
}

void http_server::check_server_status() {
    QNetworkRequest request{QUrl(m_api_route_header + "inf_qwq/health")};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_network_mgr->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply] {
        bool previous_status = m_server_online;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response_data = reply->readAll();
            int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (status_code >= 200 && status_code < 300) {
                try {
                    QJsonDocument json_doc =  QJsonDocument::fromJson(response_data);
                    QJsonObject json_obj = json_doc.object();

                    QString status = json_obj["status"].toString();
                    m_server_online = (status == "ok");

                    bool database_connected = json_obj["database_connected"].toBool();
                    long uptime = json_obj["uptime"].toInt();
                    
                    qDebug() << "Server health check: "
                             << "Status: " << status 
                             << "Database connected: " << database_connected
                             << "Uptime: " << uptime << "seconds";
                } catch(...) {
                    m_server_online = false;
                }
            } else {
                m_server_online = false;
            }
        } else {
                m_server_online = false;
        }

        if (previous_status != m_server_online) emit server_status_changed(m_server_online);

        reply->deleteLater();
    });

    QTimer::singleShot(5000, reply, [reply]() {
        if (!reply->isFinished()) reply->abort();
    });
}
