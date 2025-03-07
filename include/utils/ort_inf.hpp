

#ifndef ORT_INF_HPP
#define ORT_INF_HPP


#include "onnxruntime_c_api.h"
#include "opencv2/core/types.hpp"
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <QDebug>
#include <sstream>
#include <stdexcept>
#include <string>

#ifdef ENABLE_EIGEN
#include <eigen3/Eigen/Dense>
#endif

#include <chrono>
#include <omp.h>

template <typename InputType, typename OutputType>
class common_inferer {
protected:
    Ort::Env m_env;
    Ort::Session m_session;

    #ifdef ENABLE_EIGEN
    virtual std::tuple<std::vector<float>, cv::Mat>
    preprocess_eigen(InputType&) = 0;
    #else
    virtual std::tuple<std::vector<float>, cv::Mat>
    preprocess(InputType&) = 0;
    #endif

    virtual OutputType infer(InputType&) = 0;

    // virtual OutputType 
    // postprocess( const std::vector<float>& output_data
    //            , const std::vector<int64_t>& output_shape) = 0;
public:
    common_inferer(const std::string& model_path)
        : m_env(ORT_LOGGING_LEVEL_WARNING)
        , m_session(m_env, model_path.c_str(), Ort::SessionOptions{})
        { qDebug() << "Inferer initialized with mode: " << model_path.c_str(); }

    ~common_inferer() = default;
};

class det_inferer: public common_inferer<cv::Mat, std::vector<cv::Mat>> {
public:
    inline std::vector<cv::Mat>
    infer(cv::Mat& frame) override {
        std::vector<cv::Mat> det_croppeds{};
    
        try {

            auto start_preprocess_det = std::chrono::high_resolution_clock::now(); 
            
            #ifdef ENABLE_EIGEN
            auto [input_tensor_values, resized_img] = preprocess_eigen(frame);
                #ifdef DEBUG_MODE
                qDebug() << "Eigen preprocess";
                #endif
            #else
            auto [input_tensor_values, resized_img] = preprocess(frame);
                #ifdef DEBUG_MODE
                qDebug() << "OpenCV preprocess!";
                #endif
            #endif

            auto end_process_det = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_process_det - start_preprocess_det);
            qDebug() << "图片预处理耗时: " << duration;

            Ort::AllocatorWithDefaultOptions allocator;
            Ort::AllocatedStringPtr input_name_ptr = m_session.GetInputNameAllocated(0, allocator);
            Ort::AllocatedStringPtr output_name_ptr = m_session.GetOutputNameAllocated(0, allocator);
            const char* input_name = input_name_ptr.get();
            const char* output_name = output_name_ptr.get();

            // prepare input
            std::vector<int64_t> input_shape = {1, 3, resized_img.rows, resized_img.cols};
            auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                memory_info, input_tensor_values.data(), input_tensor_values.size(),
                                                                  input_shape.data(), input_shape.size());

            // run inf
            const char* input_names[] = {input_name};
            const char* output_names[] = {output_name};

            std::vector<Ort::Value> outputs = m_session.Run(
                Ort::RunOptions{nullptr},
                input_names, &input_tensor, 1,
                output_names, 1);

            // process output
            auto output_tensor = outputs[0].GetTensorMutableData<float>();
            auto output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
            int output_h = output_shape[2];
            int output_w = output_shape[3];

            std::vector<float> output_data(output_tensor, output_tensor + output_h * output_w);
            auto boxes = process_detection_output(output_data, output_h, output_w);

            qDebug() << "检测到 " << boxes.size() << " 个文本框";

            // visualization
            cv::Mat vis_img;
            cv::cvtColor(resized_img, vis_img, cv::COLOR_RGB2BGR);

            // traverse every cropped
            for (size_t i = 0; i < boxes.size(); i++) {
                float horizontal_ratio = 0.2;
                float vertical_ratio = 0.5;

                cv::Rect expanded_rect = expand_box(boxes[i], horizontal_ratio, vertical_ratio, vis_img.size());
                cv::Mat cropped = vis_img(expanded_rect).clone();
                det_croppeds.push_back(cropped);

                // 如果需要可视化调试
                if (false) {  // 改为true开启可视化
                    std::vector<std::vector<cv::Point>> contours = {boxes[i]};
                    cv::polylines(vis_img, contours, true, cv::Scalar(0, 255, 0), 2);

                    std::string window_name = "文本区域" + std::to_string(i + 1);
                    cv::imshow(window_name, cropped);
                    cv::moveWindow(window_name, 100 + (i % 5) * 200, 100 + (i / 5) * 150);
                }
            }

            if (false) { 
                cv::imshow("文本检测", vis_img);
                cv::waitKey(0);
                cv::destroyAllWindows();
            }

        } catch (const Ort::Exception& e) {
            qDebug() << "ONNX Runtime 错误: " << e.what();
        } catch (const std::exception& e) {
            qDebug() << "标准异常: " << e.what();
        }
        
        return det_croppeds;
    }

    #ifdef ENABLE_EIGEN
    std::tuple<std::vector<float>, cv::Mat>
    preprocess_eigen(cv::Mat& frame)  override {
        if (frame.empty()) { qDebug() << "Invalid input frame"; }

        cv::Mat rgb_img;
        cv::cvtColor(frame, rgb_img, cv::COLOR_BGR2RGB);

        int max_size = 960;
        float scale = max_size / static_cast<float>(std::max(rgb_img.cols, rgb_img.rows));

        int new_w = static_cast<int>(rgb_img.cols * scale) / 32 * 32;
        int new_h = static_cast<int>(rgb_img.rows * scale) / 32 * 32;
        cv::Mat resized_img;
        cv::resize(rgb_img, resized_img, cv::Size(new_w, new_h));

        cv::Mat float_img;
        resized_img.convertTo(float_img, CV_32FC3, 1.f / 255.f);

        std::vector<float> input_tensor(1 * 3 * new_h * new_w);

        Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
            eigen_img((float*)float_img.data, new_h, new_w * 3);

        #pragma omp parallel for collapse(2)
        for (int c = 0; c < 3; c++) {
            for (int h =0; h < new_h; h++) {
                #pragma omp simd
                for (int w = 0; w < new_w; w++) {
                    input_tensor[c * new_h * new_w + h * new_w + w] = eigen_img(h, w * 3 + c);
                }
            }
        }
        return {input_tensor, resized_img};
    }
    #else
    std::tuple<std::vector<float>, cv::Mat>
    preprocess(cv::Mat& frame) override {
        if (frame.empty()) { qDebug() << "Invalid input frame"; }

        cv::Mat rgb_img;
        cv::cvtColor(frame, rgb_img, cv::COLOR_BGR2RGB);

        int max_size = 960;
        float scale = max_size / static_cast<float>(std::max(rgb_img.cols, rgb_img.rows));
        
        int new_w = static_cast<int>(rgb_img.cols * scale) / 32 * 32;
        int new_h = static_cast<int>(rgb_img.rows * scale) / 32 * 32;
        cv::Mat resized_img;
        cv::resize(rgb_img, resized_img, cv::Size(new_w, new_h));

        cv::Mat float_img;
        resized_img.convertTo(float_img, CV_32FC3, 1.0 / 255.0);
        
        std::vector<float> input_tensor(1 * 3 * new_h * new_w);

        for (int c = 0; c < 3; c++) {
            for (int h = 0; h < new_h; h++) {
                for (int w = 0; w < new_w; w++) {
                    input_tensor[c * new_h * new_w + h * new_w + w] = 
                        float_img.at<cv::Vec3f>(h, w)[c];
                }
            }
        }
        return  { input_tensor, resized_img };
    }
    #endif

    inline std::vector<std::vector<cv::Point>>
    process_detection_output( const std::vector<float>& output
                            , int height, int width
                            , float threshold = 0.3f
                            ) {
        cv::Mat score_map( height, width
                         , CV_32F
                         , const_cast<float*>(output.data())
                         ) ;

        cv::Mat binary_map;
        cv::threshold(score_map, binary_map, threshold, 255, cv::THRESH_BINARY);
        binary_map.convertTo(binary_map, CV_8UC1);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary_map, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<std::vector<cv::Point>> boxes;
        for (const auto& cnt: contours) {
            cv::RotatedRect rect = cv::minAreaRect(cnt);
            cv::Point2f vertices[4];
            rect.points(vertices);

            std::vector<cv::Point> box;
            for (int i = 0; i < 4; i++) {
                box.push_back(cv::Point( static_cast<int>(vertices[i].x)
                                       , static_cast<int>(vertices[i].y))
                                       ) ;
            }
            boxes.push_back(box);
        }
        return boxes;
    }

    inline std::vector<std::vector<cv::Point>>
    process_detection_output_eigen( const std::vector<float>& output
                                  , int height, int width
                                  , float threshold = 0.3f
                                  ) {
        Eigen::Map<const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
            score_eigen(output.data(), height, width);

        Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>
            binary_eigen = (score_eigen.array() > threshold).cast<unsigned char>() * 255;

        cv::Mat binary_map(height, width, CV_8UC1);
        Eigen::Map<Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
            binary_map_eigen((unsigned char*)binary_map.data, height, width);
        binary_map_eigen = binary_eigen;

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary_map, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<std::vector<cv::Point>> boxes;
        for (const auto& cnt: contours) {
            cv::RotatedRect rect = cv::minAreaRect(cnt);
            cv::Point2f vertices[4];
            rect.points(vertices);

            std::vector<cv::Point> box;
            for (int i = 0; i < 4; i++) {
                box.push_back(cv::Point( static_cast<int>(vertices[i].x)
                                       , static_cast<int>(vertices[i].y)));
            }
            boxes.push_back(box);
        }
        return boxes;
    }

    inline cv::Rect 
    expand_box( const std::vector<cv::Point>& box
              , float horizontal_ratio
              , float vertical_ratio
              , const cv::Size& img_shape
              ) {
        cv::Rect rect = cv::boundingRect(box);

        int dx = static_cast<int>(rect.width * horizontal_ratio);
        int dy = static_cast<int>(rect.height * vertical_ratio);

        int new_x = std::max(0, rect.x - dx);
        int new_y = std::max(0, rect.y - dy);
        int new_w = std::min(img_shape.width - new_x, rect.width + 2 * dx);
        int new_h = std::min(img_shape.height - new_y, rect.height + 2 * dy);

        return cv::Rect{new_x, new_y, new_w, new_h};
    }

public:
    det_inferer(const std::string& model_path = "det_server.onnx")
        : common_inferer<cv::Mat, std::vector<cv::Mat>>(model_path) 
        { }

    std::vector<cv::Mat> run_inf(cv::Mat& frame) { return infer(frame); }
    void set_det_threshold(float threshold) { }

};

#include <fstream>

class rec_inferer: public common_inferer<cv::Mat, std::string> {
private:

    std::vector<std::string> m_char_dict;
    const int TARGET_HEIGHT = 48;

    void load_chars(const std::string dict_path) {
        m_char_dict.clear();
        std::ifstream file(dict_path);
        std::string line;
        if (!file.is_open()) throw std::runtime_error("Can not open chars dict: " + dict_path);

        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            if (!line.empty()) m_char_dict.push_back(line);
        }
        #ifdef DEBUG_MODE
        int cnt = 0;
        for(const auto& char: char_dict) 
            cnt++;
            #ifdef WITH_QT_DEBUG_MODE
                qDebug() << char << cnt % 10 == 0 ? '\n' : ' ';
            #else
                std::cout << char << cnt % 10 == 0 ? '\n' : ' ';
            #endif
        #endif
    }

    #ifdef ENABLE_EIGEN
    inline std::tuple<std::vector<float>, cv::Mat>
    preprocess_eigen(cv::Mat& frame) override {
        cv::Mat img = frame.clone();
        if (img.empty()) throw std::runtime_error("Invalid input frame");

        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        
        int width = static_cast<int>(img.cols * (static_cast<float>(TARGET_HEIGHT) / img.rows));

        cv::Mat resized_img;
        cv::resize(img, resized_img, cv::Size(width, TARGET_HEIGHT));

        resized_img.convertTo(resized_img, CV_32F, 1.f / 255.f);

        std::vector<float> input_tensor_values;
        input_tensor_values.resize(TARGET_HEIGHT * width * 3);

        Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
            eigen_img((float*)resized_img.data, TARGET_HEIGHT, width * 3);

        #pragma omp parallel for collapse(2) 
        for (int c = 0; c < 3; c++) {
            for (int h = 0; h < TARGET_HEIGHT; h++) {
                #pragma omp simd
                for (int w = 0; w < width; w++) {
                    input_tensor_values[c * TARGET_HEIGHT * width + h * width + w] = eigen_img(h, w * 3 + c);
                }
            }
        }
        return {input_tensor_values, resized_img};
    }
    #else
    inline std::tuple<std::vector<float>, cv::Mat>
    preprocess(cv::Mat& frame) override {
        cv::Mat img = frame.clone();
        if (img.empty()) throw std::runtime_error("Invalid input frame!");

        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

        int width = static_cast<int>(img.cols * (static_cast<float>(TARGET_HEIGHT) / img.rows));
        
        cv::Mat resized_img;
        cv::resize(img, resized_img, cv::Size(width, TARGET_HEIGHT));

        resized_img.convertTo(resized_img, CV_32F, 1.f / 255.f);

        std::vector<float> input_tensor_values;
        input_tensor_values.reserve(TARGET_HEIGHT * width * 3);

        for (int c = 0; c < 3; c++) {
            for (int h = 0; h < resized_img.rows; h++) {
                for (int w = 0; w < resized_img.cols; w++) {
                    input_tensor_values.push_back(resized_img.at<cv::Vec3f>(h, w)[c]);
                }
            }
        } 
        return { input_tensor_values, resized_img};
    }
    #endif



    inline std::string
    infer(cv::Mat& frame) override {
        try {

            auto start_preprocess = std::chrono::high_resolution_clock::now();

            #ifdef ENABLE_EIGEN
            auto [input_tensor_values, resized_img] = preprocess_eigen(frame);  
            #else
            auto [input_tenosr_values, resized_img] = preprocess(frame);
            #endif

            auto end_preprocess = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_preprocess - start_preprocess);
            
            qDebug() << "Rec inferer preprocess time cost: " << duration;


            Ort::AllocatorWithDefaultOptions allocator;
            Ort::AllocatedStringPtr input_name_ptr = m_session.GetInputNameAllocated(0, allocator);
            Ort::AllocatedStringPtr output_name_ptr = m_session.GetOutputNameAllocated(0, allocator);
            const char* input_name = input_name_ptr.get();
            const char* output_name = output_name_ptr.get();

            std::vector<int64_t> input_shape = {1, 3, resized_img.rows, resized_img.cols};
            auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                memory_info, input_tensor_values.data(), input_tensor_values.size(),
                input_shape.data(), input_shape.size()
            );

            const char* input_names[] = {input_name};
            const char* output_names[] = {output_name};

            std::vector<Ort::Value> outputs = m_session.Run(
                Ort::RunOptions{nullptr},
                input_names, &input_tensor, 1,
                output_names, 1
            );

            float* output_tensor = outputs[0].GetTensorMutableData<float>();
            std::vector<int64_t> output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();

        std::vector<int> sequence_preds;
        int sequence_length = output_shape[1];
        int num_classes = output_shape[2];

        Eigen::Map<const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
            output_eigen(output_tensor, sequence_length, num_classes);

        for (int t = 0; t < sequence_length; ++t) {
            Eigen::Index max_idx;
            output_eigen.row(t).maxCoeff(&max_idx);
            sequence_preds.push_back(static_cast<int>(max_idx));
        }

        std::vector<std::string> result;
        int last_char_idx = -1;

        for (int idx: sequence_preds) {
            if (idx != last_char_idx) {
                int adjusted_idx = idx - 1;
                if (adjusted_idx >= 0 &&adjusted_idx < m_char_dict.size()) {
                    std::string current_char = m_char_dict[adjusted_idx];
                    
                    if (current_char != "■" && current_char != "<blank>" && current_char != " ") {
                        result.push_back(current_char);
                    }
                    last_char_idx = idx;
                }
            }
        }

        std::string final_str;
        for (const auto& c: result) {
            final_str += c;
        }

        return final_str;
        } catch (const Ort::Exception& e) {
            qDebug() << "Ort::Excetion: " << e.what();
        } catch (const std::exception& e) {
            qDebug() << "Exception: " << e.what();
        }
        return {};
    }
    
public:
    rec_inferer(const std::string& model_path = "rec_server.onnx")
        : common_inferer<cv::Mat, std::string>(model_path)
        { load_chars("inf_src/classes/chars.txt"); }

    std::string run_inf(cv::Mat& frame) { return infer(frame); }
    void set_char_dict(const std::string& char_dict_path) { }

};



// class ort_inferer {
// private:
//     std::unique_ptr<det_inferer> m_chars_det_inferer;
//     std::unique_ptr<rec_inferer> m_chars_rec_inferer;
// };






#endif // ORT_INF_HPP
