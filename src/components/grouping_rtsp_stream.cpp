
#include "components/camera_wrapper.h"
#include <components/grouping_rtsp_stream.h>
#include <qcoreevent.h>
#include <qevent.h>
#include <qgridlayout.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qsharedpointer.h>
#include <qwidget.h>

grouping_rtsp_stream::grouping_rtsp_stream( int start_cam_id
                                          , int total_cams
                                          , QWidget* parent
                                          ):QWidget{parent}
                                          , m_start(start_cam_id)
                                          , m_total(total_cams)
                                          {
    int grid_size = static_cast<int>(std::sqrt(MAX_PER_PAGE));

    cams_grid = new QGridLayout(this);
    // int spacing_n_margin = 1;
    // cams_grid->setSpacing(spacing_n_margin);
    // cams_grid->setContentsMargins(spacing_n_margin, spacing_n_margin, spacing_n_margin, spacing_n_margin);
    
    cams_grid->setSpacing(0);
    cams_grid->setContentsMargins(0, 0, 0, 0);

    m_cam_pool.reserve(MAX_PER_PAGE);
    
    


    for (int pos = 0; pos < MAX_PER_PAGE; ++pos) {
        int cam_id = m_start + pos;
        setup_cam(cam_id, pos);
    }

    for (int i = 0; i < grid_size; i++) {
        cams_grid->setRowStretch(i, 1);
        cams_grid->setColumnStretch(i, 1);
    }
}

void grouping_rtsp_stream::setup_cam(int cam_id, int grid_pos) {
    QWidget* widget = nullptr;
    int grid_size = static_cast<int>(std::sqrt(MAX_PER_PAGE));

    int row = grid_pos / grid_size;
    int col = grid_pos % grid_size; 
    
    if (cam_id < m_total) {
        camera_wrapper* cam = new camera_wrapper(cam_id);
            
        connect ( cam
                , &camera_wrapper::cam_expand_req
                , this, [this, cam_id](int id){
                   emit cam_expand_req(id);
                });
        m_id2cam.insert(cam_id, cam);
        m_widget2id.insert(cam, cam_id);
        m_cam_pool.append(cam);
        widget = cam;


    } else {
        widget = create_placeholder();
    }
    cams_grid->addWidget(widget, row, col);
}

camera_wrapper* grouping_rtsp_stream::get_cam_by_id(int cam_id) {
    return m_id2cam.value(cam_id, nullptr);
}

camera_wrapper* grouping_rtsp_stream::get_cam_by_pos(int grid_pos) {
    if (grid_pos < 0 || grid_pos >= m_cam_pool.size()) return nullptr;
    return m_cam_pool.at(grid_pos);
}



QWidget* grouping_rtsp_stream::create_video_widget(int cam_id) {
    QWidget* container = new QWidget;
    container->setStyleSheet(R"(
        background: #111;
        border: 2px solid #333;
        margin: 0; padding: 0;
    )");

    QLabel* cam_label = new QLabel(QString("Cam %1").arg(cam_id + 1), container);
    cam_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    cam_label->setStyleSheet("color: white; padding: 4px;");

    QWidget* status_led = new QWidget(container);
    status_led->setFixedSize(12, 12);
    status_led->move(10, container->height() - 10);
    
    QTimer::singleShot(1000, [status_led]{
        status_led->setStyleSheet("background: #0F0; border-radius: 6px; ");
    });
    
    return container;
}

QWidget* grouping_rtsp_stream::create_placeholder() {
    QWidget* placeholder = new QWidget;
    placeholder->setStyleSheet("background: #222;");
    return placeholder;
}

void grouping_rtsp_stream::resizeEvent(QResizeEvent*) {
    int content_width = width();
    int content_height = height();
    
    int grid_size = static_cast<int>(std::sqrt(MAX_PER_PAGE));

    int cell_width = content_width / grid_size;
    int cell_height = content_height / grid_size;
    
    int final_height = cell_height;
    int final_width = cell_width;

    
    foreach(auto video, findChildren<QWidget*>()) {
        video->setFixedSize(final_width, final_height);
    }
}

