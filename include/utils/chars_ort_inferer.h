#ifndef CHARS_ORT_INFERER
#define CHARS_ORT_INFERER

#include <algorithm>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <future>
#include <functional>

#include <qcontainerfwd.h>
#include <qobject.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include <utils/ort_inf.hpp>
#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QCoreApplication>

// 结果通知器
class ResultNotifier : public QObject {
    Q_OBJECT
public:
    explicit ResultNotifier(QObject* parent = nullptr) : QObject(parent) {}
    
    void notify() {
        emit resultAvailable();
    }
    
signals:
    void resultAvailable();
};

struct inference_task {
    int cam_id;
    cv::Mat image;
    
    inference_task(int id, const cv::Mat& img): cam_id{id}, image{img} {}
    inference_task(): cam_id{-1}{}
};

struct inference_result {
    int cam_id;
    std::vector<std::string> texts;
    inference_result(): cam_id(-1) {} 
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

// 高性能线程安全队列实现
template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_cond;
    std::atomic<bool> m_done{false};

public:
    ThreadSafeQueue() = default;

    void push(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
        m_cond.notify_one();
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) return false;
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    bool wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        // 使用短超时
        if (!m_cond.wait_for(lock, std::chrono::milliseconds(10), [this]{ 
            return !m_queue.empty() || m_done; 
        })) {
            return false; // 超时
        }
        
        if (m_queue.empty()) return false;
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void done() {
        m_done = true;
        m_cond.notify_all();
    }

    bool is_done() const {
        return m_done;
    }
};

// 任务处理器类
class TaskProcessor {
private:
    ThreadSafeQueue<inference_task> m_task_queue;
    ThreadSafeQueue<inference_result> m_result_queue;
    std::vector<std::thread> m_threads;
    std::atomic<bool> m_stop{false};
    std::atomic<int> m_active_tasks{0};
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::function<void()> m_result_callback;

    std::vector<std::unique_ptr<det_inferer>> m_det_inferers;
    std::vector<std::unique_ptr<rec_inferer>> m_rec_inferers;

public:
    TaskProcessor(int num_threads, const std::string& det_model_path, const std::string& rec_model_path) {
        try {
            // 为每个线程创建独立的推理器
            for (int i = 0; i < num_threads; i++) {
                m_det_inferers.push_back(std::make_unique<det_inferer>(det_model_path));
                m_rec_inferers.push_back(std::make_unique<rec_inferer>(rec_model_path));
            }

            // 创建工作线程
            for (int i = 0; i < num_threads; i++) {
                m_threads.emplace_back(&TaskProcessor::worker_thread, this, i);
            }
        } catch (const std::exception& e) {
            qCritical() << "Failed to initialize task processor:" << e.what();
            throw;
        }
    }

    ~TaskProcessor() {
        stop();
    }

    void add_task(inference_task task) {
        m_task_queue.push(std::move(task));
        m_active_tasks++;
    }

    bool get_result(inference_result& result) {
        return m_result_queue.try_pop(result);
    }

    void set_result_callback(std::function<void()> callback) {
        m_result_callback = std::move(callback);
    }

    void stop() {
        m_stop = true;
        m_task_queue.done();
        for (auto& thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    bool is_idle() const {
        return m_active_tasks.load() == 0 && m_task_queue.empty();
    }

    int pending_tasks() const {
        return m_active_tasks.load();
    }

private:
    void notify_result_available() {
        if (m_result_callback) {
            // 从工作线程安全地调用回调
            QMetaObject::invokeMethod(QCoreApplication::instance(), [this]() {
                m_result_callback();
            }, Qt::QueuedConnection);
        }
    }

    void worker_thread(int thread_id) {
        qDebug() << "Worker thread" << thread_id << "started";
        
        det_inferer* det = m_det_inferers[thread_id].get();
        rec_inferer* rec = m_rec_inferers[thread_id].get();
        
        inference_task task;
        while (!m_stop) {
            // 尝试获取任务
            bool got_task = m_task_queue.wait_and_pop(task);
            if (!got_task) {
                if (m_stop) break;
                
                // 如果没有任务，短暂休眠避免CPU空转
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            // 处理任务
            try {
                // 检测阶段
                std::vector<cv::Mat> detected_regions = det->run_inf(task.image);
                
                // 识别阶段
                std::vector<std::string> result_set;
                for (auto& region : detected_regions) {
                    std::string recognized_text = rec->run_inf(region);
                    if (!recognized_text.empty()) {
                        result_set.push_back(recognized_text);
                        qDebug() << "rec res: " << QString(recognized_text.c_str());
                    }
                }
                
                // 将结果放入结果队列
                m_result_queue.push(inference_result(task.cam_id, result_set));
                
                // 通知有新结果
                notify_result_available();
            } catch (const std::exception& e) {
                qWarning() << "Error in worker thread" << thread_id << ":" << e.what();
                m_result_queue.push(inference_result(task.cam_id, {}));
                notify_result_available();
            }
            
            // 减少活动任务计数
            m_active_tasks--;
            m_cond.notify_all();
        }
        
        qDebug() << "Worker thread" << thread_id << "stopped";
    }
};

// 修改 chars_ort_inferer 类
class chars_ort_inferer: public QObject {
    Q_OBJECT
public:
    chars_ort_inferer(const std::string& det_model_path = "det_server.onnx",
                     const std::string& rec_model_path = "rec_server.onnx",
                     int num_threads = 1,
                     QObject* parent = nullptr)
        : QObject{parent}
        , m_processor{new TaskProcessor(num_threads, det_model_path, rec_model_path)}
        , m_result_timer{new QTimer(this)}
        , m_notifier{new ResultNotifier(this)}
    {
        // 连接通知器信号
        connect(m_notifier, &ResultNotifier::resultAvailable, this, &chars_ort_inferer::check_results);
        
        // 设置处理器回调
        m_processor->set_result_callback([this]() {
            QMetaObject::invokeMethod(m_notifier, "notify", Qt::QueuedConnection);
        });
        
        // 仍然保留定时器作为备用机制
        connect(m_result_timer, &QTimer::timeout, this, &chars_ort_inferer::check_results);
        m_result_timer->start(5); // 每5毫秒检查一次，作为后备
    }

    ~chars_ort_inferer() {
        if (m_processor) {
            delete m_processor;
        }
    }

    void run_inf(int cam_id, const cv::Mat& frame) {
        if (frame.empty()) {
            qWarning() << "Invalid input frame";
            return;
        }
        
        cv::Mat frameCopy = frame.clone();
        m_processor->add_task(inference_task(cam_id, frameCopy));
    }

    // 批量推理接口
    void run_inf_batch(const std::vector<std::pair<int, cv::Mat>>& frames) {
        for (const auto& [cam_id, frame] : frames) {
            if (!frame.empty()) {
                cv::Mat frameCopy = frame.clone();
                m_processor->add_task(inference_task(cam_id, frameCopy));
            }
        }
    }

    // 等待所有任务完成
    void wait_for_completion(int timeout_ms = -1) {
        auto start = std::chrono::steady_clock::now();
        while (!m_processor->is_idle()) {
            check_results(); // 处理结果
            
            if (timeout_ms > 0) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
                if (elapsed > timeout_ms) {
                    qWarning() << "Wait for completion timed out after" << timeout_ms << "ms";
                    break;
                }
            }
            
            QCoreApplication::processEvents(); // 处理Qt事件
        }
    }

signals:
    void inference_completed(int cam_id, const std::vector<std::string> results);

private slots:
    void check_results() {
        inference_result result;
        int processed_count = 0;
        const int MAX_PROCESS_PER_CALL = 10; // 每次调用最多处理的结果数
        
        while (processed_count < MAX_PROCESS_PER_CALL && m_processor->get_result(result)) {
            emit inference_completed(result.cam_id, result.texts);
            processed_count++;
        }
        
        // 如果还有更多结果等待处理，立即再次调用自己
        if (processed_count == MAX_PROCESS_PER_CALL) {
            QMetaObject::invokeMethod(this, "check_results", Qt::QueuedConnection);
        }
    }

private:
    TaskProcessor* m_processor;
    QTimer* m_result_timer;
    ResultNotifier* m_notifier;
};

#endif
