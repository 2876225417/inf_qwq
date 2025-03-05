

#ifndef ORT_INF_H
#define ORT_INF_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <onnxruntime_c_api.h>
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


class chars_det_inferer{
public:
    chars_det_inferer(const std::string& model_path = "det_server.onnx")
        : m_env(ORT_LOGGING_LEVEL_WARNING)
        #ifdef _WIN32
        m_session(m_env, std::wstring(model_path.begin(), model_path.end()).c_str());
        #else
        , m_session(m_env,model_path.c_str(), Ort::SessionOptions{})
        #endif 
        { }

        std::vector<cv::Mat> run_inf(cv::Mat& frame) {
        
        try {// preprocess image for text detection
        auto [input_tensor, original_img] = preprocess_img(frame);
        
        std::vector<int64_t> input_shape = { 1, 3, original_img.rows, original_img.cols };
        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        Ort::Value input_onnx_tensor = Ort::Value::CreateTensor<float>(
            memory_info, input_tensor.data(), input_tensor.size(),
            input_shape.data(), input_shape.size());

        Ort::AllocatorWithDefaultOptions allocator;

        std::vector<std::string> input_names;
        std::vector<const char*> input_names_c_str;
        size_t num_input_nodes = m_session.GetInputCount();
        input_names.resize(num_input_nodes);
        input_names_c_str.resize(num_input_nodes);

        for (size_t i = 0; i < num_input_nodes; i++) {
            Ort::AllocatedStringPtr input_name = m_session.GetInputNameAllocated(i, allocator);
            input_names[i] = input_name.get();
            input_names_c_str[i] = input_names[i].c_str();
        }

        std::vector<std::string> output_names;
        std::vector<const char*> output_names_c_str;
        size_t num_output_nodes = m_session.GetOutputCount();
        output_names.resize(num_output_nodes);
        output_names_c_str.resize(num_output_nodes);

        for (size_t i = 0; i < num_output_nodes; i++) {
            Ort::AllocatedStringPtr output_name = m_session.GetOutputNameAllocated(i, allocator);
            output_names[i] = output_name.get();
            output_names_c_str[i] = output_names[i].c_str();
        }
    
        auto output_tensors = m_session.Run(
            Ort::RunOptions{ nullptr },
            input_names_c_str.data(),
            &input_onnx_tensor,
            1,
            output_names_c_str.data(),
            1);

        float* output_data = output_tensors[0].GetTensorMutableData<float>();
        std::vector<int64_t> output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();

        int height = output_shape[2];
        int width = output_shape[3];
        std::vector<float> score_map(output_data, output_data + (height * width));

        auto boxes = process_detection_output(score_map, height, width);
        qDebug() << "Detected " << boxes.size() << " text boxes";
        
        cv::Mat vis_img;
        cv::cvtColor(original_img, vis_img, cv::COLOR_RGB2BGR);

        int box_idx = 0;
        std::vector<cv::Mat> chars_croppeds{};
        for (const auto& box: boxes) {
            cv::Mat cropped = warp_affine_crop(vis_img, box, chars_croppeds, 0.2f, 1.3f);
            
            if (cropped.empty()) continue;
        }

            return chars_croppeds;
        // return boxes;
        } catch (const Ort::Exception& e) {
            return {};
        } catch (const cv::Exception& e) {
            return {};
        } catch (const std::exception& e) {
            return {};
        }
        return {};
    }
private:
    Ort::Env m_env;
    Ort::Session m_session;

    std::pair<std::vector<float>, cv::Mat> preprocess_img(cv::Mat& img) {
        cv::Mat rgb_img;
        cv::cvtColor(img, rgb_img, cv::COLOR_BGR2RGB);
        
        float max_size = 1024.f;
        float scale = std::min(max_size / rgb_img.cols, max_size / rgb_img.rows);
        int scaled_w = static_cast<int>(rgb_img.cols * scale);
        int scaled_h = static_cast<int>(rgb_img.rows * scale);

        scaled_w = (scaled_w + 31) / 32 * 32;
        scaled_h = (scaled_h + 31) / 32 * 32;

        cv::Mat resized_img;
        cv::resize(rgb_img, resized_img, cv::Size(scaled_w, scaled_h), 0, 0, cv::INTER_LINEAR);

        std::vector<float> input_tensor(3 * scaled_h * scaled_w);
        for (int c = 0; c < 3; c++) {
            for (int h = 0; h < scaled_h; h++) {
                for (int w = 0; w < scaled_w; w++) {
                    input_tensor[c * scaled_h * scaled_w + h * scaled_w + w] = 
                        (resized_img.at<cv::Vec3b>(h, w)[c] / 255.f - 0.5f) * 2.f;
                }
            }
        }
        return { input_tensor, resized_img };
    }

    std::vector<std::vector<cv::Point>>
    process_detection_output( const std::vector<float>& output
                            , int height
                            , int width
                            , float threshold = 0.1f
                            ) {
        cv::Mat score_map(height, width, CV_32F);
        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {
                score_map.at<float>(h, w) = output[h * width + w];
            }
        }

        cv::Mat binary_map;
        cv::threshold(score_map, binary_map, threshold, 255, cv::THRESH_BINARY);
        binary_map.convertTo(binary_map, CV_8UC1);

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::dilate(binary_map, binary_map, kernel, cv::Point(-1, -1), 2);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary_map, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<std::vector<cv::Point>> boxes;
        // 旋转判断有问题
        for (const auto& contour: contours) {
            if (contour.size() < 4) continue;
            
            std::vector<cv::Point> hull;
            cv::convexHull(contour, hull);

            cv::RotatedRect rect = cv::minAreaRect(hull);
            cv::Point2f box_points[4];
            rect.points(box_points);

            std::vector<cv::Point> box;
            for (int i = 0; i < 4; i++) {
               box.push_back(cv::Point( static_cast<int>(box_points[i].x)
                                      , static_cast<int>(box_points[i].y))); 
            }
            boxes.push_back(box);
        }
        return boxes;
    }

    cv::Mat 
    warp_affine_crop( const cv::Mat& img
                    , const std::vector<cv::Point>& box
                    , std::vector<cv::Mat>& chars_croppeds
                    , float horizontal_stretch = 1.f
                    , float vertical_stretch = 1.f
                    ) {
        try {
            cv::RotatedRect rect = cv::minAreaRect(box);
            cv::Point2f center = rect.center;
            cv::Size2f size = rect.size;
            float angle = rect.angle;

            if (size.width < size.height) {
                angle += 90;
                std::swap(size.width, size.height);
            }

            size.width *= (1.f + horizontal_stretch);
            size.height *= (1.f + vertical_stretch);

            size.width = std::max(std::abs(size.width), 10.f);
            size.height = std::max(std::abs(size.height), 10.f);

            cv::Mat M = cv::getRotationMatrix2D(center, angle, 1.0);
            cv::Mat rotated;
            cv::warpAffine(img, rotated, M, img.size(), cv::INTER_LINEAR);

            cv::Mat cropped;
            cv::getRectSubPix( rotated
                             , cv::Size(static_cast<int>(size.width)
                                       , static_cast<int>(size.height)
                                       )
                             , center
                             , cropped
                             ) ;
            chars_croppeds.push_back(cropped); 
            return cropped;
        } catch (const std::exception& e) {
            qDebug() << "Exception: " << e.what();
            return cv::Mat();
        }
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
