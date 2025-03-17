
#include "HCNetSDK.h"
#include "utils/db_manager_mini.h"
#include "windows/rtsp_config_window.h"
#include <components/draw_overlay.h>
#include <QTimer>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdatetime.h>
#include <qevent.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qoverload.h>
#include <qtimer.h>
#include <qwidget.h>

#include <windows/expanded_camera_window.h>

void draw_overlay::set_inference_result(const QString& result) {
    m_inference_result = result;
    update();
}

void draw_overlay::set_keywords(const QVector<QString>& keywords) {
    m_all_keywords = keywords;
    m_keywords = detect_matched_keywords(m_inference_result, m_all_keywords);
    update();
}

void draw_overlay::update_keywords_no_args() {
    m_keywords = detect_matched_keywords(m_inference_result, m_all_keywords);
    update();
}

void draw_overlay::set_status() {
    m_is_normal_status = m_keywords.isEmpty();
    update();
}

void draw_overlay::set_cam_name(const QString& cam_name) {
    m_cam_name = cam_name;
    update();
}

void draw_overlay::hint_warning() {
    qDebug() << "Last record id: " << m_last_record_id;
    // trigger keywords
    if (!m_keywords.isEmpty() && m_is_inf /* forbid hint repeatedly */) {
        m_show_warning = true;
        update();
        m_warning_timer->start(10000);
        // stop inferring
        m_rects.clear();
        m_is_inf = false;
         
        if (m_last_record_id > 0) send_http_alarm(m_last_record_id);
        
        emit reset_inf_result_after_hint(m_cam_id); 
    }
}

int draw_overlay::record_warning2db() {

    // but need to record all records
    // if (m_keywords.isEmpty()) return -1;
    
    if (!db_manager::instance().is_connected()) {
        qWarning() << "Database not connected, warning record not saved";
        return -1;
    }
    
    // data postprocess
    int cam_id_2db            = m_cam_id;
    QString rtsp_name_2db     = m_rtsp_config.rtsp_name;
    QString inference_res_2db = m_inference_result.isEmpty() ? "Detected nothing" : m_inference_result;
    bool status_2db           = m_is_normal_status;
    QString keywords_2db      = m_keywords.isEmpty() ? " " /* or not set keywords */ : m_keywords;
    QString rtsp_url_2db      = m_rtsp_config.config2url();
    

    int record_id 
        = db_manager::instance().add_warning_record( cam_id_2db
                                                   , rtsp_name_2db
                                                   , inference_res_2db
                                                   , status_2db
                                                   , keywords_2db
                                                   , rtsp_name_2db
                                                   , rtsp_url_2db
                                                   ) ;
    if (record_id > 0) qDebug() << "Warning record saved to database for camera " << m_cam_id
                                << "with ID: " << record_id;
    else qDebug() << "Failed to save warning record to database";

    return record_id;
}

void draw_overlay::draw_warning(QPainter& painter) {
    if (!m_show_warning || m_keywords.isEmpty()) {
        return;
    }

    painter.save();

    QFont warning_font("Arial", 16, QFont::Bold);
    painter.setFont(warning_font);

    QString warning_text = QString("检测到关键字: %1").arg(m_keywords);

    QFontMetrics fm(warning_font);
    QSize text_size = fm.size(Qt::TextSingleLine, warning_text);

    int x = (width() - text_size.width()) / 2;
    int y = (height() - text_size.height()) / 2;

    int padding = 20;
    QRect bg_rect(x - padding, y - padding, text_size.width() + 2 * padding, text_size.height() + 2 * padding);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 200));
    painter.drawRoundedRect(bg_rect, 10, 10);

    painter.setPen(QPen(Qt::red, 2));
    painter.drawRoundedRect(bg_rect, 10, 10);

    painter.setPen(Qt::red);
    painter.drawText(QRect(x, y, text_size.width(), text_size.height()), Qt::AlignCenter, warning_text);

    painter.restore();
}


void draw_overlay::draw_camera_id(QPainter& painter) {
    painter.save();

    QFont id_font("Arial", 10, QFont::Bold);
    painter.setFont(id_font);

    QString id_text = QString("视频流ID: %1").arg(m_cam_id);

    QFontMetrics fm(id_font);
    QSize text_size = fm.size(Qt::TextSingleLine, id_text);

    int margin = 10;
    int x = width() - text_size.width() - margin;
    int y = height() - text_size.height() - margin;

    QRect bg_rect(x - 5, y - 5, text_size.width() + 10, text_size.height() + 10);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 0));
    painter.drawRoundedRect(bg_rect, 5, 5);

    painter.setPen(Qt::white);
    painter.drawText(QRect( x, y
                          , text_size.width(), text_size.height())
                          , Qt::AlignLeft | Qt::AlignTop, id_text);
    painter.restore();
}

void draw_overlay::draw_camera_name(QPainter& painter) {
    painter.save();

    QFont name_font("Arial", 10, QFont::Bold);
    painter.setFont(name_font);

    QString name_text = QString("视频流信息: %1").arg(m_cam_name);

    QFontMetrics fm(name_font);
    QSize text_size = fm.size(Qt::TextSingleLine, name_text);

    int margin = 10;
    int x = width() - text_size.width() - margin;
    int y = height() - text_size.height() - margin;
   
    QRect bg_rect(x - 5, y - 5, text_size.width() + 10, text_size.height() + 10);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 0));
    painter.drawRoundedRect(bg_rect, 5, 5);

    painter.setPen(Qt::white);
    painter.drawText(QRect( x, y - 50
                          , text_size.width(), text_size.height())
                          , Qt::AlignLeft | Qt::AlignTop, name_text);

    painter.restore();
}



void draw_overlay::draw_status_indicator(QPainter& painter) {
    painter.save();

    QFont status_font("Arial", 12, QFont::Bold);
    painter.setFont(status_font);

    QString status_text = m_is_normal_status ? "正常" : "异常";
    QColor status_color = m_is_normal_status ? Qt::green : Qt::red;

    int status_width = width() / 5;
    int status_X = (width() - status_width) / 2;

    QRect status_rect(status_X, 10, status_width, 30);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.drawRoundedRect(status_rect, 5, 5);

    painter.setPen(status_color);
    painter.drawText(status_rect, Qt::AlignCenter, status_text);
    painter.restore();
}



void draw_overlay::draw_inference_result(QPainter& painter) {
    painter.save();

    QFont result_font("Arial", 10);
    painter.setFont(result_font);
    
    int margin = 10;
    int line_height = 20;
    int text_width = width() / 2;

    QRect result_rect(margin, height() - 2 * line_height - margin, text_width, line_height);
    QRect keywords_rect(margin, height() - line_height - margin, text_width, line_height);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.drawRoundedRect(QRect( margin - 5
                                 , height() - 2 * line_height - margin - 5
                                 , text_width + 10
                                 , 2 * line_height + 10), 5, 5);

    painter.setPen(Qt::white);
    QString result_text = "识别内容: " + (m_inference_result.isEmpty() ? " " : m_inference_result);

    QFontMetrics fm(result_font);
    result_text = fm.elidedText(result_text, Qt::ElideRight, text_width);
    painter.drawText(result_rect, Qt::AlignLeft | Qt::AlignVCenter, result_text);

    painter.setPen(m_is_normal_status ? Qt::white : Qt::yellow);
    QString all_keywords; for(auto& keyword: m_keywords) all_keywords += keyword;
    QString keywords_text = "关键字: " + (all_keywords.isEmpty() ? " " : all_keywords);

    keywords_text = fm.elidedText(keywords_text, Qt::ElideRight, text_width);
    painter.drawText(keywords_rect, Qt::AlignLeft | Qt::AlignVCenter, keywords_text);
    painter.restore();
}


QString draw_overlay::detect_matched_keywords( const QString& text
                                             , const QVector<QString>& all_keywords) 
                                             {
    QStringList matched_keywords;

    for (const QString& keyword: all_keywords) {
        if (text.contains(keyword, Qt::CaseInsensitive)) {
            matched_keywords.append(keyword);
        }
    }
    return matched_keywords.join(" ");
}

draw_overlay::draw_overlay(int cam_id, QWidget* parent)
    : QWidget(parent)
    , m_cam_id{cam_id}
    //, m_number_combobox{new QComboBox(this)}
    , m_timer{new QTimer(this)}
    {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // default http alarm url
    m_http_url = "http://32.121.0.166:8018/TOOLS-M-AlarmMessage/send" ;

    connect (m_timer, &QTimer::timeout, this, [this]() {
        if (!m_rects.isEmpty()) emit timer_timeout_update(m_rects);
        //else qDebug() << "No rects selected.";
    }); m_timer->start(5000);


    m_warning_timer = new QTimer(this);
    m_warning_timer->setSingleShot(true);
    connect (m_warning_timer, &QTimer::timeout, [this]() {
        m_show_warning = false;
        update();
    });


    m_network_mgr = new QNetworkAccessManager(this); 
}

void draw_overlay::send_http_alarm(int record_id) {
    // force http alarm enabled
    m_enable_http_url = true;
    
    if (!m_enable_http_url || m_http_url.isEmpty()) return;

    // if (m_is_normal_status) return;


    if (record_id <= 0) {
        qWarning() << "Invalid record ID for HTTP alarm";
        return;
    }


    QUrl url(m_http_url);
    if (!url.isValid()) {
        qWarning() << "Invalid HTTP alarm URL: " << m_http_url;

        if (db_manager::instance().is_connected()) {
            db_manager::instance().update_warning_record_push_status(record_id, false, "Invalid HTTP alarm URL");
        }
        return;
    }
    
    qDebug() << "Alarming HTTP URL: " << m_http_url;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json_data;
    json_data["deviceId"] = QString::number(m_cam_id);
    json_data["alarmTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    QDateTime alarm_time = QDateTime::currentDateTime();
    QString time_str = alarm_time.toString(Qt::ISODate);

    QString alarm_msg = tr("地点 %1 检测到异常内容: %2 时间 %3")
                            .arg(m_cam_name)
                            .arg(m_keywords)
                            .arg(time_str);

    if (!m_rtsp_config.rtsp_name.isEmpty()) {
        alarm_msg = tr("设备 %1 地点(%2) 检测到异常内容: %3 时间: %4")
                            .arg(m_cam_id)
                            .arg(m_cam_name.isEmpty() ? " " : m_cam_name)
                            .arg(m_keywords)
                            .arg(time_str);

        json_data["deviceId"] = m_rtsp_config.rtsp_name;
    }

    qDebug() << "Alarm msg: " << alarm_msg;

    json_data["alarmMsg"] = alarm_msg;

    QJsonDocument doc(json_data);
    QByteArray json_bytes = doc.toJson();

    QNetworkReply* reply = m_network_mgr->post(request, json_bytes);

    reply->setProperty("record_id", record_id);

    connect (reply, &QNetworkReply::finished, [this, reply]() {
        handle_http_response(reply);
    });
}

void draw_overlay::handle_http_response(QNetworkReply* reply) {
    int record_id = reply->property("record_id").toInt();
    bool push_success = false;
    QString push_msg;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response_data = reply->readAll();
        QJsonDocument response_doc = QJsonDocument::fromJson(response_data);

        if (response_doc.isObject()) {
            QJsonObject response_obj = response_doc.object();
            QString code = response_obj["code"].toString();
            QString message = response_obj["message"].toString();

            push_msg = code + ": " + message;

            if (code == "200" || code.toInt() == 200){
                qDebug() << "HTTP alarm sent successfully: " << message;
                push_success = true;
            }
            else { 
                qWarning() << "HTTP alarm failed with code: " << code << message;
                push_success = false;
            }
        } else {
            push_msg = "Invalid response format";
            push_success = false;
        }
    } else {
        push_msg = "Request failed: " + reply->errorString();
        qWarning() << "HTTP alarm request failed: " << reply->errorString();
        push_success = false;
    }

    if (record_id > 0 && db_manager::instance().is_connected()) {
        db_manager::instance().update_warning_record_push_status(record_id, push_success, push_msg);
    }
    
    reply->deleteLater();
}


QRect draw_overlay::get_expand_btn_rect() const {
    const int btn_size = 24;
    const int margin = 10;
    return QRect (
        width() - btn_size - margin,
        margin,
        btn_size,
        btn_size
    );
}

QRect draw_overlay::get_switch_btn_rect() const {
    const int btn_size = 24;
    const int margin = 10;

    return QRect (
        width() - btn_size - margin,
        margin + btn_size + 5,
        btn_size,
        btn_size
    );
}



void draw_overlay::handle_rect_number_changed(int idx) {
    if (m_edit_idx >= 0 && m_edit_idx < m_rects.size()) {
        m_rects[m_edit_idx].number = idx + 1;
        update();
        qDebug() << "idx changed: " << m_rects[m_drag_idx].number;
        for(const auto& rd: m_rects) qDebug() << "number: " << rd.number;
    }

    m_edit_idx = -1;
}

QRect draw_overlay::paint_close_btn(const QRect& rect) const {
    const int btn_size = 12;
    return QRect (
        rect.right() - btn_size - 2,
        rect.top() + 2,
        btn_size,
        btn_size
    );
}

void draw_overlay::update_hover_state(const QPoint& pos) {
    m_expand_btn_hovered = get_expand_btn_rect().contains(pos);
    m_switch_btn_hovered = get_switch_btn_rect().contains(pos);
    
    int old_hover_idx = m_hover_idx;
    m_hover_idx = -1;

    for (int i = 0; i < m_rects.size(); ++i) {
        QRect btn_rect = paint_close_btn(m_rects[i].rect);
        if (btn_rect.contains(pos)) {
            m_hover_idx = i;
            return;
        }
        if (m_rects[i].rect.contains(pos)) m_hover_idx = i;
    }
    if (old_hover_idx != m_hover_idx) {
        if (m_hover_idx == -1) {
            setCursor(Qt::ArrowCursor);
        } else if (m_resize_handle == none) {
            setCursor(Qt::SizeAllCursor);
        }
    }
}

void draw_overlay::check_resize_handles(const QPoint& pos) {
    m_resize_handle = none;
    m_resize_idx = -1;
    
    if (m_hover_idx == -1) return;

    const QRect& rect = m_rects[m_hover_idx].rect;
    
    for (int handle = top_left; handle <= left; ++handle) {
        QRect handle_rect 
            = get_resize_handle_rect(rect, static_cast<resize_handle>(handle));
        if (handle_rect.contains(pos)) {
            m_resize_handle = static_cast<resize_handle>(handle);
            m_resize_idx = m_hover_idx;
            return;
        }
    }
}

QRect draw_overlay::get_resize_handle_rect(const QRect& rect, resize_handle handle) const {
    QRect handle_rect;
    int size = m_handle_size;

    switch (handle) {
        case top_left:      handle_rect = QRect(rect.left()         - size / 2, rect.top()          - size / 2, size, size); break;
        case top_right:     handle_rect = QRect(rect.right()        - size / 2, rect.top()          - size / 2, size, size); break;
        case bottom_left:   handle_rect = QRect(rect.left()         - size / 2, rect.bottom()       - size / 2, size, size); break;
        case bottom_right:  handle_rect = QRect(rect.right()        - size / 2, rect.bottom()       - size / 2, size, size); break;
        case top:           handle_rect = QRect(rect.center().x()   - size / 2, rect.top()          - size / 2, size, size); break;
        case right:         handle_rect = QRect(rect.right()        - size / 2, rect.center().y()   - size / 2, size, size); break;
        case bottom:        handle_rect = QRect(rect.center().x()   - size / 2, rect.bottom()       - size / 2, size, size); break;
        case left:          handle_rect = QRect(rect.left()         - size / 2, rect.center().y()   - size / 2, size, size); break;
        default:            break;
    }
    return handle_rect;
}

void draw_overlay::update_rect_with_resize(const QPoint& pos) {
    if (m_resize_idx == -1 || m_resize_handle == none) return;

    QRect& rect = m_rects[m_resize_idx].rect;

    switch (m_resize_handle) {
        case top_left:          rect.setTopLeft(pos);     break;
        case top_right:         rect.setTopRight(pos);    break;
        case bottom_left:       rect.setBottomLeft(pos);  break;
        case bottom_right:      rect.setBottomRight(pos); break;
        case top:               rect.setTop(pos.y());     break;
        case right:             rect.setRight(pos.x());   break;
        case bottom:            rect.setBottom(pos.y());  break;
        case left:              rect.setLeft(pos.x());    break;
        default: break;
    }
    rect = rect.normalized();
}

void draw_overlay::draw_resize_handles(QPainter& painter, const QRect& rect) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);

    for (int handle = top_left; handle <= left; ++handle) {
        QRect handle_rect = get_resize_handle_rect(rect, static_cast<resize_handle>(handle));
        painter.drawRect(handle_rect);
    }
}

void draw_overlay::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::RightButton) {
        update_hover_state(e->pos());
        if (m_hover_idx != -1) {
            m_edit_idx = m_hover_idx;
            QRect target_rect = m_rects[m_hover_idx].rect;


        }
    } else if (e->button() == Qt::LeftButton) {
        update_hover_state(e->pos());

        // expand window
        if (m_expand_btn_hovered) {
             if (!m_expanded_camera_window) {
                m_expanded_camera_window = new expanded_camera_window(m_cam_id);
                m_expanded_camera_window->setWindowTitle(tr("Camera %1 Details").arg(m_cam_id));

                m_expanded_camera_window->set_rtsp_url(m_current_rtsp_url);

                m_expanded_camera_window->setAttribute(Qt::WA_DeleteOnClose);
                
                QObject::connect( m_expanded_camera_window
                                , &expanded_camera_window::destroyed
                                , this, [this]() { 
                                    qDebug() << "m_expanded window destroyed";
                                    m_expanded_camera_window = nullptr; 
                                });

                

            }
            m_expanded_camera_window->show();
            m_expanded_camera_window->raise();
            m_expanded_camera_window->activateWindow();

            //emit expand_camera_request(m_cam_id);
            return;
        }

        QRect switch_btn = get_switch_btn_rect();
            
        if (switch_btn.contains(e->pos()))  {   // switch down camera
            m_rects.clear();
             
            if (!m_rtsp_config_window) {
                m_rtsp_config_window = new rtsp_config_window();
                m_rtsp_config_window->setWindowTitle(tr("视频流 %1 配置").arg(m_cam_id));
                
                m_rtsp_config_window->m_close_conn_button->setEnabled(true);
                m_rtsp_config_window->m_save_button->setEnabled(false);
                connect ( m_rtsp_config_window
                        , &rtsp_config_window::send_rtsp_url
                        , this, [this](const QString& rtsp_url, const rtsp_config& rtsp_cfg){
                            set_cam_name(rtsp_cfg.rtsp_name);
                            emit switch_cam(rtsp_url, rtsp_cfg); 
                            qDebug() << "rtsp_url: " << rtsp_url; 
                        });

                connect ( m_rtsp_config_window
                        , &rtsp_config_window::on_close_conn
                        , this, [this]() {
                            qDebug() << "Close button";
                            emit suspend_cam();
                        });


            }
            m_rtsp_config_window->show();
            m_rtsp_config_window->raise();
            m_rtsp_config_window->activateWindow();

            return;
        }
        
        if (m_hover_idx != -1) {
            QRect btn_rect = paint_close_btn(m_rects[m_hover_idx].rect);
            if (btn_rect.contains(e->pos())) {
                m_rects.remove(m_hover_idx);
                m_hover_idx = -1;
                setCursor(Qt::ArrowCursor);
                update();
                return;
            }

            check_resize_handles(e->pos());

            if (m_resize_handle == none) {
                m_drag_idx = m_hover_idx;
                m_drag_start_pos = e->pos();
            }
        } else {
            m_hover_idx = -1;
            m_resize_handle = none;
            m_drag_idx = -1;
            m_resize_idx = -1;
             
            m_is_dragging = true;
            m_start = e->pos();
            m_current_rect = QRect();
            setCursor(Qt::ArrowCursor);
        }
    } update();
}

void draw_overlay::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        bool was_resizing = (m_resize_idx != -1 && m_resize_handle != none);
        bool was_dragging = m_is_dragging && m_current_rect.isValid();

        if (was_resizing) {
            emit selected(m_rects);
            m_resize_idx = -1;
            m_resize_handle = none;
        } else if (was_dragging) {
            rect_data new_rect;
            new_rect.rect = m_current_rect;
            m_rects.append(new_rect);
            emit selected(m_rects);
            m_is_inf = true;
            m_current_rect = QRect();
        }

        m_is_dragging = false;
        m_drag_idx = -1;

        update_hover_state(e->pos());
    } update();
}

void draw_overlay::mouseMoveEvent(QMouseEvent* e) {
    if (m_resize_idx != -1 && m_resize_handle != none) {
        switch (m_resize_handle) {
            case top_left:
            case bottom_right:  setCursor(Qt::SizeFDiagCursor); break;
            case top_right:
            case bottom_left:   setCursor(Qt::SizeBDiagCursor); break;
            case top:
            case bottom:        setCursor(Qt::SizeVerCursor); break;
            case left:
            case right:         setCursor(Qt::SizeHorCursor); break;
            default:            setCursor(Qt::ArrowCursor); break;
        }
        
        update_rect_with_resize(e->pos());
        emit update_frame_moving(m_rects);
        update();
        return;
    }
    
    if (m_drag_idx != -1) {
        QPoint delta = e->pos() - m_drag_start_pos;
        m_rects[m_drag_idx].rect.translate(delta);
        m_drag_start_pos = e->pos();
        emit update_frame_moving(m_rects);
        update();
        return;
    } 
    
    if (m_is_dragging) {
        m_current_rect = QRect(m_start, e->pos()).normalized();
        update();
        return;
    }
    
    update_hover_state(e->pos());
    
    if (m_hover_idx != -1) {
        check_resize_handles(e->pos());
        
        if (m_resize_handle != none) {
            switch (m_resize_handle) {
                case top_left:
                case bottom_right:  setCursor(Qt::SizeFDiagCursor); break;
                case top_right:
                case bottom_left:   setCursor(Qt::SizeBDiagCursor); break;
                case top:
                case bottom:        setCursor(Qt::SizeVerCursor); break;
                case left:
                case right:         setCursor(Qt::SizeHorCursor); break;
                default:            setCursor(Qt::SizeAllCursor); break;
            }
        } else {
            setCursor(Qt::SizeAllCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void draw_overlay::leaveEvent(QEvent* event) {
    m_hover_idx = -1;
    m_expand_btn_hovered = false;
    m_switch_btn_hovered = false;
    
    if (!m_is_dragging && m_drag_idx == -1 && m_resize_idx == -1) 
        setCursor(Qt::ArrowCursor);

    update();
    QWidget::leaveEvent(event);
}

void draw_overlay::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < m_rects.size(); ++i) {
        const auto& rd = m_rects[i];

        painter.setPen(QPen(i == m_hover_idx ? Qt::cyan : Qt::red, 2));
        painter.setBrush(QColor(255, 0, 0, i == m_hover_idx ? 80 : 30));
        painter.drawRect(rd.rect);

        if (i == m_hover_idx) {
            QRect btn = paint_close_btn(rd.rect);
            painter.setPen(QPen(Qt::white, 2));
            painter.drawLine(btn.topLeft(), btn.bottomRight());
            painter.drawLine(btn.topRight(), btn.bottomLeft());
            
            draw_resize_handles(painter, rd.rect);
        }

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(rd.rect.adjusted(5, 5, 0, 0), Qt::AlignLeft | Qt::AlignTop, QString::number(rd.number));
    }

    if (m_is_dragging && m_current_rect.isValid()) {
        painter.setPen(QPen(Qt::blue, 2));
        painter.setBrush(QColor(0, 0, 255, 30));
        painter.drawRect(m_current_rect);
    }

   //  QRect expand_btn = get_expand_btn_rect();
   //  painter.setPen(Qt::NoPen);
   //  painter.setBrush(m_expand_btn_hovered ? QColor(100, 100, 100, 180) : QColor(60, 60, 60, 180));
   //  painter.drawEllipse(expand_btn);
   //
   //  painter.setPen(QPen(Qt::white, 2));
   //  int center_X = expand_btn.center().x();
   //  int center_Y = expand_btn.center().y();
   //  int icon_size = expand_btn.width() / 3;
   //
   //  painter.drawEllipse(QPoint(center_X - 2, center_Y - 2), icon_size / 2, icon_size / 2);
   // 
   //  painter.drawLine(center_X + icon_size / 3, center_Y + icon_size / 3, center_X + icon_size / 2, center_Y + icon_size / 2);


    QRect switch_btn = get_switch_btn_rect();
    painter.setPen(QPen(Qt::white, 2)); 
    painter.setBrush(m_switch_btn_hovered ? QColor(100, 100, 100, 180) : QColor(60, 60, 60, 180));
    painter.drawRoundedRect(switch_btn, 4, 4);

    int cx = switch_btn.center().x();
    int cy = switch_btn.center().y();

   
    QPolygon arrow;
    arrow << QPoint(cx, cy - 6) << QPoint(cx - 6, cy + 2) << QPoint(cx + 6, cy + 2);
    painter.setBrush(Qt::white);
    painter.drawPolygon(arrow);


    draw_status_indicator(painter);
    draw_inference_result(painter);
    
    draw_camera_id(painter);
    draw_camera_name(painter);
    draw_warning(painter);
}

bool draw_overlay::eventFilter(QObject* obj, QEvent* e) {
  
    return QWidget::eventFilter(obj, e); 
}

bool draw_overlay::event(QEvent* e) {
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);

    }
    return QWidget::event(e);
}
