

#ifndef ORT_INF_H
#define ORT_INF_H

#include <string>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>


class ort_inferer {
public:
    ort_inferer( const std::string& model_path
               , int model_height = 48
               , const std::vector<std::string>& input_nodes = {"x"}
               , const std::vector<std::string>& output_nodes = {"save_infer_model/scale_0.tmp_0"}
               , int intra_op_num_threads = 1 
               ) : m_env{ORT_LOGGING_LEVEL_WARNING, "ort_inf"}
                 , m_session_options{}
                 , m_session{m_env, model_path.c_str(), m_session_options}
                 , m_memory_info{Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault)}
                 , m_model_height{model_height} 
                 {
                    m_session_options.SetIntraOpNumThreads(intra_op_num_threads);
                    
                    for (const auto& s: input_nodes) input_names.push_back(s.c_str());
                    for (const auto& s: output_nodes) output_names.push_back(s.c_str());
                 }

    std::string infer(const cv::Mat& img) {
        cv::Mat input_blob = preprocess_image(img);

        std::vector<int64_t> input_shape = {1, 3, m_model_height, input_blob.size[3]};

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            m_memory_info,
            input_blob.ptr<float>(),
            input_blob.total(),
            input_shape.data(),
            input_shape.size()
        );

        auto outputs = m_session.Run(Ort::RunOptions { nullptr }, input_names.data(), &input_tensor, 1, output_names.data(), 1);
        if (outputs.empty() || !outputs[0].IsTensor()) {
            throw std::runtime_error("Error in infer");
        }

        return decode_output(outputs[0]);
    }

private:

    cv::Mat preprocess_image(const cv::Mat& img) {
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        
        float scale = static_cast<float>(m_model_height) / img.rows;
        cv::resize(img, img, cv::Size(static_cast<int>(img.cols * scale), m_model_height));

        img.convertTo(img, CV_32F, 1.f / 255.f);
        std::vector<cv::Mat> channels{3};
        cv::split(img, channels);

        cv::Mat blob;
        cv::vconcat(channels[0].reshape(1, 1), channels[1].reshape(1, 1), blob);
        cv::vconcat(blob, channels[2].reshape(1, 1), blob);
        return blob.reshape(1, { 1, 3, m_model_height, static_cast<int>(img.cols * scale)});
    }

    std::string decode_output(Ort::Value& output_tensor) const {
        float* data = output_tensor.GetTensorMutableData<float>();
        auto shape = output_tensor.GetTensorTypeAndShapeInfo().GetShape();

        std::vector<int> predictions;
        for (int t = 0; t < shape[1]; ++t) {
            auto start = data + t * shape[2];
            predictions.push_back(std::max_element(start, start + shape[2]) - start);
        }

        std::string result;
        int last = -1;
        for (int idx: predictions) {
            const int adjusted = idx - 1;
            if (adjusted >= 0 && adjusted < char_mapping.size() && adjusted != last) {
                result += char_mapping[adjusted];
                last = adjusted;
            }
        }
        return result;
    }

    Ort::Env m_env;
    Ort::SessionOptions m_session_options;
    Ort::Session m_session;
    Ort::MemoryInfo m_memory_info;
    int m_model_height;
    std::vector<const char*> input_names;
    std::vector<const char*> output_names;

    inline static const std::vector<char> char_mapping = {
        '-', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    };
};

#endif