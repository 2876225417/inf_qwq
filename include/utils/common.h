
#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <ostream>
#include <qdebug.h>


enum class rtsp_proto_type {
    HIKVISION,
    ALHUA
};

struct rtsp_config {
    int rtsp_id = 0;
    rtsp_proto_type rpt = rtsp_proto_type::HIKVISION;
    QString rtsp_name;
    QString username;
    QString password;
    QString ip;
    QString port;
    QString channel = "101";
    QString subtype = "0";
    QString rtsp_url;

    struct {
        float cropped_x = 0.f;
        float cropped_y = 0.f;
        float cropped_dx = 1.f;
        float cropped_dy = 1.f;
    } cropped_pos;


    bool is_hk() const { return rpt == rtsp_proto_type::HIKVISION; }
    bool is_dh() const { return rpt == rtsp_proto_type::ALHUA;     }

    QString config2url() const {
        switch(rpt) {
            case rtsp_proto_type::HIKVISION: return rtsp_hk();
            case rtsp_proto_type::ALHUA:     return rtsp_ah();
            default: return "";
        }
    }

    QString config2url_mask() const {
        switch(rpt) {
            case rtsp_proto_type::HIKVISION: return rtsp_mask_hk();
            case rtsp_proto_type::ALHUA:     return rtsp_mask_ah();
            default:                         return "";
        }

    }

private: 
    QString rtsp_hk() const { 
        return "rtsp://" + username 
             + ":"       + password
             + "@"       + ip       
             + ":"       + port     
             + "/Streaming/Channels/101";
    }

    QString rtsp_mask_hk() const {
        return "rtsp://" + username
             + ":"       + "********"
             + "@"       + ip
             + ":"       + port
             + "/Streaming/Channels/101";
   }

    QString rtsp_ah() const {
        return "rtsp://" + username
             + ":"       + password
             + "@"       + ip
             + ":"       + port
             + "/cam/realmonitor?channel=1@subtype=0";
    }

    QString rtsp_mask_ah() const {
        return "rtsp://" + username
             + ":"       + "********"
             + "@"       + ip
             + ":"       + port
             + "/cam/realmonitor?channel=1@subtype=0";
    }
};

#include <QDebug>
inline QDebug operator<<(QDebug debug, const rtsp_config& config) {
    QDebugStateSaver saver(debug);

    debug.nospace() << " rtsp_config {\n"
                    << " rtsp_id:       " << config.rtsp_id << '\n'
                    << " rtsp_protocal_type: " << (config.is_hk() ? "HKVISION" : "DAHUA") << '\n'
                    << " rt"

    return debug;
}




#endif  //COMMON_H
