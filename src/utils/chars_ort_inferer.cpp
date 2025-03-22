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

void inference_worker::set_inferers(std::unique_ptr<det_inferer>& det, std::unique_ptr<rec_inferer>& rec) {
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
        std::vector<cv::Mat> detected_regions = m_det_inferer->run_inf(task.image);
        
        if (detected_regions.empty()) {
            emit result_ready(inference_result(task.cam_id, {}));
            QMetaObject::invokeMethod(this, "process_next", Qt::QueuedConnection);
            return;
        }

        std::vector<std::string> result_set;
        for (auto& region: detected_regions) {
            std::string recognized_text = m_rec_inferer->run_inf(region);
            if (!recognized_text.empty()) { 
                result_set.push_back(recognized_text);
            }
        }
        
        emit result_ready(inference_result(task.cam_id, result_set));
    } catch (const std::exception& e) {
        qWarning() << "Error occurred when inferring: " << e.what();
        emit result_ready(inference_result(task.cam_id, {}));
    }

    QMetaObject::invokeMethod(this, "process_next", Qt::QueuedConnection);
}
