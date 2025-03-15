

#ifndef GROUPING_RTSP_STREAM_H
#define GROUPING_RTSP_STREAM_H


#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qevent.h>
#include <qgridlayout.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>
#include <qwidget.h>

#include <QTimer>
#include <QGridLayout>
#include <QLabel>
#include <components/camera_wrapper.h>

#include <QMap>
#include <QHash>


class grouping_rtsp_stream: public QWidget{
    Q_OBJECT
public:
    // structed params 
    explicit grouping_rtsp_stream( int start_cam_id
                        , int total_cams
                        , QWidget* parent = nullptr
                        ) ;
    QMap<int, camera_wrapper*> m_id2cam;
    QVector<camera_wrapper*> m_cam_pool;
    QHash<QWidget*, int> m_widget2id;

    camera_wrapper* get_cam_by_id(int cam_id);
    
    camera_wrapper* get_cam_by_pos(int grid_pos);
    
    static inline int
    get_MAX_PER_PAGE() { return MAX_PER_PAGE; }

signals:
    void cam_expand_req(int cma_id); 
protected:
    void resizeEvent(QResizeEvent*) override;

private:
    QGridLayout* cams_grid;
    int m_start;
    int m_total;
    static const int MAX_PER_PAGE = 4;
    
    QWidget* create_video_widget(int cam_id);   
    QWidget* create_placeholder();
    
    void setup_cam(int cam_id, int grid_pos);
};






#endif
