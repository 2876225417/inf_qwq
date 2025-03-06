

#ifndef ORT_INF_H
#define ORT_INF_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <iterator>
#include <onnxruntime_c_api.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <stdexcept>
#include <string>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QDebug>

#include <iostream>
#include <fstream>

class chars_inferer {
private:
    Ort::Env env;
    Ort::Session session;
    std::vector<std::string> char_dict;

    void loadCharDict(const std::string& dict_path) {
        std::ifstream file(dict_path);
        std::string line;
        
        if (!file.is_open()) {
            throw std::runtime_error("无法打开字典文件: " + dict_path);
        }

        while (std::getline(file, line)) {
            // 去除行首尾空白
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            if (!line.empty()) {
                char_dict.push_back(line);
            }
        }
    }

    cv::Mat preprocessImage(const cv::Mat& input_img, int target_height = 48) {
        cv::Mat img = input_img.clone();
        if (img.empty()) {
            throw std::runtime_error("输入图像为空");
        }

        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        
        int width = static_cast<int>(img.cols * (static_cast<float>(target_height) / img.rows));
        cv::resize(img, img, cv::Size(width, target_height));

        img.convertTo(img, CV_32F, 1.0 / 255.0);

        return img;
    }

public:
    chars_inferer(const std::string& model_path = "rec_server.onnx", const std::string& dict_path = "inf_src/classes/chars.txt") 
        : env(ORT_LOGGING_LEVEL_WARNING), 
          session(env, model_path.c_str(), Ort::SessionOptions{}) 
    {
        loadCharDict(dict_path);
    }

    std::string infer(const cv::Mat& input_image) {
        cv::Mat input_tensor = preprocessImage(input_image);
        
        std::vector<float> input_data;
        for (int c = 0; c < input_tensor.channels(); ++c) {
            for (int h = 0; h < input_tensor.rows; ++h) {
                for (int w = 0; w < input_tensor.cols; ++w) {
                    input_data.push_back(input_tensor.at<cv::Vec3f>(h, w)[c]);
                }
            }
        }

        std::vector<int64_t> input_shape = {1, 3, input_tensor.rows, input_tensor.cols};

        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        Ort::Value input_tensor_ort = Ort::Value::CreateTensor<float>(
            memory_info, input_data.data(), input_data.size(), input_shape.data(), input_shape.size()
        );

        // 准备输出
        std::vector<Ort::Value> outputs;
        const char* input_names[] = {"x"};
        const char* output_names[] = {"softmax_2.tmp_0"};

        outputs = session.Run(
            Ort::RunOptions{}, 
            input_names, &input_tensor_ort, 1, 
            output_names, 1
        );

        // 解析输出
        float* output_tensor = outputs[0].GetTensorMutableData<float>();
        std::vector<int64_t> output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();

        // 找到每个时间步的最大概率索引
        std::vector<int> sequence_preds;
        int sequence_length = output_shape[1];
        int num_classes = output_shape[2];

        for (int t = 0; t < sequence_length; ++t) {
            float max_prob = -1;
            int max_idx = -1;
            
            for (int c = 0; c < num_classes; ++c) {
                float prob = output_tensor[t * num_classes + c];
                if (prob > max_prob) {
                    max_prob = prob;
                    max_idx = c;
                }
            }
            sequence_preds.push_back(max_idx);
        }

        std::vector<std::string> result;
        int last_char_idx = -1;

        for (int idx : sequence_preds) {
            if (idx != last_char_idx) {
                int adjusted_idx = idx - 1;
                
                if (adjusted_idx >= 0 && adjusted_idx < char_dict.size()) {
                    std::string current_char = char_dict[adjusted_idx];
                    
                    if (current_char != "■" && current_char != "<blank>" && current_char != " ") {
                        result.push_back(current_char);
                    }
                    
                    last_char_idx = idx;
                }
            }
        }

        // 合并结果
        std::string final_str;
        for (const auto& c : result) {
            final_str += c;
        }

        return final_str;
    }
};

#include <fstream>

class chars_inf_det {
private:
    // must m_env then m_session
    Ort::Env m_env;
    Ort::Session m_session;
  
public:
    chars_inf_det(const std::string& model_path = "det_server.onnx")
        : m_env(ORT_LOGGING_LEVEL_WARNING)
        , m_session(m_env, model_path.c_str(), Ort::SessionOptions{})
        {
            qDebug() << "Try to initialize!";
        
        }

//     inline std::vector<cv::Mat>
//     run_inf(cv::Mat& frame) {
//         cv::imshow("123", frame);
//         std::vector<cv::Mat> det_croppeds{};
//         try {
//
//             Ort::AllocatorWithDefaultOptions allocator;
//             Ort::AllocatedStringPtr  input_name_ptr = m_session.GetInputNameAllocated(0, allocator);
//             Ort::AllocatedStringPtr  output_name_ptr = m_session.GetOutputNameAllocated(0, allocator);
//             const char* input_name = input_name_ptr.get();
//             const char* output_name = output_name_ptr.get();
//
//             auto [input_tensor_values, original_img] = preprocess_image(frame);
//
//             std::vector<int64_t> input_shape = { 1, 3, original_img.rows, original_img.cols };
//
//             auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
//             Ort::Value input_tensor = Ort::Value::CreateTensor<float>( memory_info
//                                                                      , input_tensor_values.data()
//                                                                      , input_tensor_values.size()
//                                                                      , input_shape.data()
//                                                                      , input_shape.size()
//                                                                      ) ;
//             const char* input_names[] = {input_name};
//             const char* output_names[] = {output_name};
//
//             std::vector<Ort::Value> outputs = m_session.Run(
//                 Ort::RunOptions{nullptr},
//                 input_names,
//                 &input_tensor,
//                 1,
//                 output_names,
//                 1
//                 ) ;
//
//             auto output_tensor = outputs[0].GetTensorMutableData<float>();
//             auto output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
//             int output_h = output_shape[2];
//             int output_w = output_shape[3];
//
//             std::vector<float> output_data(output_tensor, output_tensor + output_h * output_w);
//             auto boxes = process_detection_output(output_data, output_h, output_w);
//
//             qDebug() << "检测到 " << boxes.size() << " 个文本框";
//
//             cv::Mat vis_img;
//             cv::cvtColor(original_img, vis_img, cv::COLOR_RGB2BGR);
//
//             for (size_t i = 0; i < boxes.size(); i++) {
//                 float horizontal_ratio = 0.2;
//                 float vertical_ratio = 0.5;
//
//                 cv::Rect expanded_rect = expand_box( boxes[i]
//                                                    , horizontal_ratio, vertical_ratio
//                                                    , vis_img.size()
//                                                    ) ;
//                 cv::Mat cropped = vis_img(expanded_rect).clone();
//                 det_croppeds.push_back(cropped);
//                 if (true) {
//                     std::vector<std::vector<cv::Point>> contours = {boxes[i]};
//                     cv::polylines(vis_img, contours, true, cv::Scalar(0, 255, 0), 2);
//
//                     std::string window_name = "文本区域" + std::to_string(i + 1);
//                     cv::imshow(window_name, cropped);
//
//                     cv::moveWindow(window_name, 100 + (i % 5) * 200, 100 + (i / 5) * 150);
//                 }
//             }
//             if (true) {
//                 cv::imshow("文本检测: ", vis_img);
//                 cv::waitKey(0);
//                 cv::destroyAllWindows();
//             }
//
//         } catch (const Ort::Exception& e) {
//             qDebug() << "错误: " << e.what();
//             return {};
//         } catch (const std::exception& e) {
//             qDebug() << "错误: " << e.what();
//             return {};
//         }
//         return det_croppeds;
//
//     }

inline std::vector<cv::Mat>
run_inf(cv::Mat& frame) {
    std::vector<cv::Mat> det_croppeds{};

    try {
        // 预处理图像 - 需要修改preprocess_image接受Mat而不是文件路径
        cv::Mat rgb_img;
        cv::cvtColor(frame, rgb_img, cv::COLOR_BGR2RGB);

        // 保持长边为960并确保32的倍数
        int max_size = 960;
        float scale = max_size / static_cast<float>(std::max(rgb_img.cols, rgb_img.rows));
        int new_w = static_cast<int>(rgb_img.cols * scale) / 32 * 32;
        int new_h = static_cast<int>(rgb_img.rows * scale) / 32 * 32;
        cv::Mat resized_img;
        cv::resize(rgb_img, resized_img, cv::Size(new_w, new_h));

        // 转换为浮点型并归一化
        cv::Mat float_img;
        resized_img.convertTo(float_img, CV_32FC3, 1.0/255.0);

        // 准备ONNX输入 (NCHW格式)
        std::vector<float> input_tensor_values(1 * 3 * new_h * new_w);
        // HWC -> CHW
        for (int c = 0; c < 3; c++) {
            for (int h = 0; h < new_h; h++) {
                for (int w = 0; w < new_w; w++) {
                    input_tensor_values[c * new_h * new_w + h * new_w + w] =
                    float_img.at<cv::Vec3f>(h, w)[c];
                }
            }
        }

        // 获取输入输出名称
        Ort::AllocatorWithDefaultOptions allocator;
        Ort::AllocatedStringPtr input_name_ptr = m_session.GetInputNameAllocated(0, allocator);
        Ort::AllocatedStringPtr output_name_ptr = m_session.GetOutputNameAllocated(0, allocator);
        const char* input_name = input_name_ptr.get();
        const char* output_name = output_name_ptr.get();

        // 准备输入
        std::vector<int64_t> input_shape = {1, 3, resized_img.rows, resized_img.cols};
        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info, input_tensor_values.data(), input_tensor_values.size(),
                                                                  input_shape.data(), input_shape.size());

        // 运行推理
        const char* input_names[] = {input_name};
        const char* output_names[] = {output_name};

        std::vector<Ort::Value> outputs = m_session.Run(
            Ort::RunOptions{nullptr},
            input_names, &input_tensor, 1,
            output_names, 1);

        // 处理输出
        auto output_tensor = outputs[0].GetTensorMutableData<float>();
        auto output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
        int output_h = output_shape[2];
        int output_w = output_shape[3];

        std::vector<float> output_data(output_tensor, output_tensor + output_h * output_w);
        auto boxes = process_detection_output(output_data, output_h, output_w);

        qDebug() << "检测到 " << boxes.size() << " 个文本框";

        // 准备可视化
        cv::Mat vis_img;
        cv::cvtColor(resized_img, vis_img, cv::COLOR_RGB2BGR);

        // 遍历每个检测框
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

        // 调试可视化
        if (false) {  // 改为true开启可视化
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

private:
    inline std::tuple<std::vector<float>, cv::Mat>
    preprocess_image(cv::Mat& frame) {
        if (frame.empty()) {
            qDebug() << "Invalid input frame!";
        }

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

        return { input_tensor, resized_img };
    }

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

    inline cv::Rect
    expand_box( const std::vector<cv::Point>& box
              , float horizontal_ratio, float vertical_ratio
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

};

class ort_inferer {
public:
    explicit ort_inferer( const OrtLoggingLevel& logging_level = OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING,
                          const char* model_path = "rec_gen.onnx"
                        );
    ~ort_inferer();
    void set_intra_threads(int num);
    std::string exec_inf(cv::Mat frame); 
    std::vector<std::string>&& get_classes() { return std::move(m_class_names); }
    void read_class_from_source(const std::string& file_path);

private:
    Ort::Env m_env;
    Ort::SessionOptions m_session_options;
    Ort::Session m_session;
    std::vector<std::string> m_class_names;


    
    const std::vector<const char*> m_input_names    = { "x" };
    const std::vector<const char*> m_output_names   = { "softmax_11.tmp_0" };


    std::vector<Ort::Value> run_inference(cv::Mat& input_blob); 

    std::string process_results(Ort::Value& output_tensor); 

    std::string decode_predictions(const std::vector<int>& predictions); 

    cv::Mat img_preprocess(cv::Mat frame, const int target_height);
    cv::Mat qimage2mat(QImage& qimage);
};

#endif
