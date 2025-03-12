


#include "utils/ort_inf.hpp"
#include <memory>
#include <qlogging.h>
#include <qmutex.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qthread.h>
#include <utils/chars_ort_inferer.h>


inference_worker::inference_worker(QObject* parent)
    : QObject{parent}
    , m_det_inferer{nullptr}
    , m_rec_inferer{nullptr}
    , m_processing{false} { }

void inference_worker::set_inferers( std::unique_ptr<det_inferer>& det
                                   , std::unique_ptr<rec_inferer>& rec
                                   ) {
    if (det && rec) {
        m_det_inferer = det.get();
        m_rec_inferer = rec.get();
    } else {
        qCritical() << "Trying to set null inferers!";
    }
}

void inference_worker::add_task(const inference_task& task) {
    QMutexLocker lock(&m_mutex);
    m_task_queue.enqueue(task);

    emit task_added();

    if (!m_processing) {
        QMetaObject::invokeMethod(this, "process_next", Qt::QueuedConnection);
    }
}


void inference_worker::process_next() {
    QMutexLocker locker(&m_mutex);

    if (m_task_queue.isEmpty()) {
        m_processing = false;
        return;
    }

    m_processing = true;
    inference_task task = m_task_queue.dequeue();
    locker.unlock();

    if (task.image.empty() || !m_det_inferer || !m_rec_inferer) {
        qWarning() << "Invalid input frame or inferers not set!";
        emit result_ready(inference_result(task.cam_id, {}));
        QMetaObject::invokeMethod(this, "process_next", Qt::QueuedConnection);
        return;
    }

    try {
        std::vector<cv::Mat> detected_regions 
            = m_det_inferer->run_inf(task.image);
        
        if (detected_regions.empty()) {
            emit result_ready(inference_result(task.cam_id, {}));
            QMetaObject::invokeMethod(this, "process_next", Qt::QueuedConnection);
            return;
        }

        std::vector<std::string> result_set;
        QString results_set;
        for (auto& region: detected_regions) {
            std::string recognized_text = m_rec_inferer->run_inf(region);
            if (!recognized_text.empty()) { result_set.push_back(recognized_text); results_set += recognized_text + '\t'; };
        }
        qDebug() << "result set: " << results_set; 
        emit result_ready(inference_result(task.cam_id, result_set));
    } catch (const std::exception& e) {
        qWarning() << "Error occurred when inferring: " << e.what();
        emit result_ready(inference_result(task.cam_id, {}));
    }

    QMetaObject::invokeMethod(this, "process_next", Qt::QueuedConnection);
}

chars_ort_inferer::chars_ort_inferer( const std::string& det_model_path
                                    , const std::string& rec_model_path
                                    , QObject* parent) 
                                    : QObject{parent}
                                    {
    try {
        m_chars_det_inferer = std::make_unique<det_inferer>(det_model_path);
        qDebug() << "Initialize detecting inferer successfully!";
        m_chars_rec_inferer = std::make_unique<rec_inferer>(rec_model_path);
        qDebug() << "Initialize recognizing inferer successfully!";
    } catch (const std::exception& e) {
        qCritical() << "Failed to initialze models: " << e.what();
        throw;
    }

    m_worker_thread = new QThread();
    m_worker = new inference_worker();
    m_worker->set_inferers(m_chars_det_inferer, m_chars_rec_inferer);
    m_worker->moveToThread(m_worker_thread);

    connect( m_worker
           , &inference_worker::result_ready
           , [this](const inference_result& result){
                emit inference_completed(result.cam_id, result.texts);
           });

    connect (m_worker_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    
    m_worker_thread->start();
    qDebug() << "Worker thread started";
}

chars_ort_inferer::~chars_ort_inferer() {
    if (m_worker_thread) {
        m_worker_thread->quit();
        m_worker_thread->wait();
        delete m_worker_thread;
        qDebug() << "Worker thread stopped";
    }
}

void chars_ort_inferer::run_inf(int cam_id, const cv::Mat& frame) {
    if (frame.empty()) {
        qWarning() << "Invalid input frame";
        return;
    }
    
    if (!m_worker) {
        qCritical() << "Worker is null!";
        return;
    }
    
    qDebug() << "Adding inference task for camera " << cam_id 
             << "image size: " << frame.cols << "x" << frame.rows;
    
    cv::Mat frameCopy = frame.clone();
    
    QMetaObject::invokeMethod(m_worker,
                             "add_task",
                             Qt::QueuedConnection,
                             Q_ARG(inference_task, inference_task(cam_id, frameCopy)));
    qDebug() << "Start inferring";
}
