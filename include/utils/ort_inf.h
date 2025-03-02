

#ifndef ORT_INF_H
#define ORT_INF_H

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <onnxruntime_c_api.h>
#include <opencv2/imgcodecs.hpp>
#include <stdexcept>
#include <string>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <QImage>


class ort_inferer {
public:
    explicit ort_inferer( const OrtLoggingLevel& logging_level = OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING,
                          const char* model_path = "model.onnx"
                        );
    ~ort_inferer();
    std::string exec_inf(QImage& qimage);

    std::string exec_inf(cv::Mat frame); 
private:
    Ort::Env m_env;
    Ort::SessionOptions m_session_options;
    Ort::Session m_session;

    const std::vector<const char*> m_input_names    = { "x" };
    const std::vector<const char*> m_output_names   = { "save_infer_model/scale_0.tmp_0" };


    void set_intra_threads(int num);

    std::vector<Ort::Value> run_inference(cv::Mat& input_blob); 

    std::string process_results(Ort::Value& output_tensor); 

    std::string decode_predictions(const std::vector<int>& predictions); 

    cv::Mat img_preprocess(cv::Mat frame, const int target_height);
    cv::Mat qimage2mat(QImage& qimage);
};

#endif
