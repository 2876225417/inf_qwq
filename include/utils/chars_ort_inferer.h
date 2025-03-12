

#ifndef CHARS_ORT_INFERER
#define CHARS_ORT_INFERER

#include <algorithm>
#include <memory>


#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <utils/ort_inf.hpp>
#include <QMutex>
#include <QQueue>


struct inference_task {
    int cam_id;
    cv::Mat image;

    inference_task(int id, const cv::Mat& img): cam_id{id}, image{img} {}
    inference_task(): cam_id{-1}{}
};

struct inference_result {
    int cam_id;
    std::vector<std::string> texts;

    inference_result(int id, const std::vector<std::string>& results)
        : cam_id{id}
        , texts{results} { }
};


class inference_worker: public QObject {
    Q_OBJECT
public:
    explicit inference_worker(QObject* parent = nullptr);
    void set_inferers(std::unique_ptr<det_inferer>& det, std::unique_ptr<rec_inferer>& rec);


public slots:
    void process_next();
    void add_task(const inference_task& task);

signals:
    void result_ready(const inference_result& result);
    void task_added();
private:
    det_inferer* m_det_inferer;
    rec_inferer* m_rec_inferer;

    QMutex m_mutex;
    QQueue<inference_task> m_task_queue;
    bool m_processing;
};


class chars_ort_inferer: public QObject {
    Q_OBJECT
public:
    chars_ort_inferer( const std::string& det_model_path = "det_server.onnx"
                     , const std::string& rec_model_path = "rec_server.onnx"
                     , QObject* parent = nullptr
                     ) ;

    ~chars_ort_inferer();

    void run_inf(int cam_id, const cv::Mat& frame);

signals:
    void inference_completed(int cam_id, const std::vector<std::string> results);
private:
    std::unique_ptr<det_inferer> m_chars_det_inferer;
    std::unique_ptr<rec_inferer> m_chars_rec_inferer;
    QThread* m_worker_thread;
    inference_worker* m_worker;
};

#endif
