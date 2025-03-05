
#include <algorithm>
#include <exception>
#include <fstream>
#include <onnxruntime_cxx_api.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/opencv.hpp>
#include <utils/ort_inf.h>

const std::vector<char> char_dict = {
    '-', '.', '0', '1',
    '2', '3', '4', '5',
    '6', '7', '8', '9', };

ort_inferer::ort_inferer( const OrtLoggingLevel& logging_level
                        , const char* model_path
                        ) : m_env(logging_level, "ort_inferer")
                          , m_session(m_env, model_path, m_session_options)
                          { set_intra_threads(1); }

ort_inferer::~ort_inferer() { }

#include <QDebug>
#include <locale>
#include <codecvt>


void ort_inferer::read_class_from_source(const std::string& file_path) {
    std::ifstream in_file(file_path);
    if (!in_file.is_open()) {
        throw std::runtime_error("Failed to open class file" + file_path);
    }
    
    m_class_names.clear();

    std::string line;

    while (std::getline(in_file, line)) {
        line.erase(line.find_last_not_of("\n\r") + 1);
        if (!line.empty()) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            m_class_names.emplace_back(std::move(line));
        }
    }
    in_file.close();
    if (m_class_names.empty()) {
        throw std::runtime_error("Empty class list in file: " + file_path);
    }
}


std::string ort_inferer::exec_inf(cv::Mat frame) {
    const int MODEL_HEIGHT = 48;
    cv::Mat processed = img_preprocess(frame, MODEL_HEIGHT);
    auto output_tensors = run_inference(processed);
    return process_results(output_tensors[0]);
}

void ort_inferer::set_intra_threads(int num) {
    m_session_options.SetIntraOpNumThreads(num);
}

std::vector<Ort::Value> ort_inferer::run_inference(cv::Mat& input_blob) {
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator,
        OrtMemType::OrtMemTypeDefault
    );
    std::vector<int64_t> input_shape = {
        1, 3, input_blob.size[2], input_blob.size[3]
    };

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        input_blob.ptr<float>(),
        input_blob.total(),
        input_shape.data(),
        input_shape.size()
    );

    return m_session.Run(
        Ort::RunOptions { nullptr },
        m_input_names.data(),
        &input_tensor,
        1,
        m_output_names.data(),
        1
    );
}

std::string ort_inferer::process_results(Ort::Value& output_tensor) {
std::vector<int> predictions;
    if (output_tensor.IsTensor()) {
        float* output_data = output_tensor.GetTensorMutableData<float>();

        auto shape_info = output_tensor.GetTensorTypeAndShapeInfo();
        std::vector<int64_t> out_shape = shape_info.GetShape();

        const int seq_len = out_shape[1];
        const int num_classes = out_shape[2];

        for (int t = 0; t < seq_len; ++t) {
            float* timestep_data = output_data + t * num_classes;
            auto max_it = std::max_element(timestep_data, timestep_data + num_classes);
            predictions.push_back(std::distance(timestep_data, max_it));
        }
    }
    return decode_predictions(predictions);
}

std::string ort_inferer::decode_predictions(const std::vector<int>& predictions) {
    std::string result;
    int last_index = -1;
    for (int index: predictions) {
        int mapped_index = index - 1;
        if (mapped_index >= 0 && mapped_index < (int)char_dict.size()) {
            if (mapped_index != last_index) {
                result += char_dict[mapped_index];
                last_index = mapped_index;
            }
        }
    }
    return result;
}

cv::Mat ort_inferer::img_preprocess(cv::Mat frame, const int target_height) {
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    
    float scale = static_cast<float>(target_height) / frame.rows;
    int target_width = static_cast<int>(frame.cols * scale);
    cv::resize(frame, frame, cv::Size(target_width, target_height));

    frame.convertTo(frame, CV_32F, 1.f / 255.f);

    cv::Mat channels[3];
    cv::split(frame, channels);

    cv::Mat blob;
    cv::vconcat(channels[0].reshape(1, 1), channels[1].reshape(1, 1), blob);

    cv::vconcat(blob, channels[2].reshape(1, 1), blob);
    return blob.reshape(1, { 1, 3, target_height, target_width}).clone();
}

cv::Mat ort_inferer::qimage2mat(QImage& qimage) {
    QImage swapped = qimage.convertToFormat(QImage::Format_RGB888).rgbSwapped();
    

    cv::Mat mat( swapped.height()
               , swapped.width()
               , CV_8UC3
               , swapped.bits()
               , static_cast<size_t>(swapped.bytesPerLine()));

    if (!swapped.isDetached()) { return mat.clone(); }

    return mat;
}
