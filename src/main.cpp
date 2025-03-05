


#include <QApplication>
#include <algorithm>
#include <memory>
#include <onnxruntime_c_api.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <qapplication.h>
#include <windows/mainwindow.h>
#include <utils/ort_inf.h>
#include <onnxruntime_cxx_api.h>


#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

//Helper function for ONNX Runtime
template <typename T>
static void softmax(T& input) {
    float rowmax = *std::max_element(input.begin(), input.end());
    std::vector<float> y(input.size());
    float sum = 0.0f;
    for (size_t i = 0; i < input.size(); ++i) {
        y[i] = std::exp(input[i] - rowmax);
        sum += y[i];
    }
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = y[i] / sum;
    }
}

std::pair<std::vector<float>, cv::Mat> preprocessImage(const std::string& imgPath) {
    // 加载图像
    cv::Mat img = cv::imread(imgPath);
    if (img.empty()) {
        throw std::runtime_error("Failed to load image: " + imgPath);
    }
    
    // 转换为RGB
    cv::Mat rgbImg;
    cv::cvtColor(img, rgbImg, cv::COLOR_BGR2RGB);
    
    // 更智能的缩放策略
    float maxSize = 1024.0f;  // 增加最大尺寸
    float scale = std::min(maxSize / rgbImg.cols, maxSize / rgbImg.rows);
    int newW = static_cast<int>(rgbImg.cols * scale);
    int newH = static_cast<int>(rgbImg.rows * scale);
    
    // 确保尺寸能被32整除
    newW = (newW + 31) / 32 * 32;
    newH = (newH + 31) / 32 * 32;
    
    cv::Mat resizedImg;
    cv::resize(rgbImg, resizedImg, cv::Size(newW, newH), 0, 0, cv::INTER_LINEAR);
    
    // 标准化处理
    std::vector<float> inputTensor(3 * newH * newW);
    for (int c = 0; c < 3; c++) {
        for (int h = 0; h < newH; h++) {
            for (int w = 0; w < newW; w++) {
                inputTensor[c * newH * newW + h * newW + w] = 
                    (resizedImg.at<cv::Vec3b>(h, w)[c] / 255.0f - 0.5f) * 2.0f;  // 标准化到 [-1, 1]
            }
        }
    }
    
    return {inputTensor, resizedImg};
}

std::vector<std::vector<cv::Point>> processDetectionOutput(const std::vector<float>& output, 
                                                          int height, int width, 
                                                          float threshold = 0.1f) {  // 降低阈值
    // 创建分数映射
    cv::Mat scoreMap(height, width, CV_32F);
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            scoreMap.at<float>(h, w) = output[h * width + w];
        }
    }
    
    // 二值化处理
    cv::Mat binaryMap;
    cv::threshold(scoreMap, binaryMap, threshold, 255, cv::THRESH_BINARY);
    binaryMap.convertTo(binaryMap, CV_8UC1);
    
    // 形态学膨胀，扩大检测区域
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::dilate(binaryMap, binaryMap, kernel, cv::Point(-1, -1), 2);
    
    // 寻找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binaryMap, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // 生成外接旋转矩形
    std::vector<std::vector<cv::Point>> boxes;
    for (const auto& contour : contours) {
        if (contour.size() < 4) continue;
        
        // 使用凸包来更好地包围文本区域
        std::vector<cv::Point> hull;
        cv::convexHull(contour, hull);
        
        cv::RotatedRect rect = cv::minAreaRect(hull);
        cv::Point2f boxPoints[4];
        rect.points(boxPoints);
        
        std::vector<cv::Point> box;
        for (int i = 0; i < 4; i++) {
            box.push_back(cv::Point(static_cast<int>(boxPoints[i].x), 
                                    static_cast<int>(boxPoints[i].y)));
        }
        boxes.push_back(box);
    }
    
    return boxes;
}


cv::Mat warpAffineCrop(const cv::Mat& img, const std::vector<cv::Point>& box, 
                       float horizontalStretch = 1.0f, float verticalStretch = 1.0f) {
    try {
        cv::RotatedRect rect = cv::minAreaRect(box);
        cv::Point2f center = rect.center;
        cv::Size2f size = rect.size;
        float angle = rect.angle;
        
        // 打印原始尺寸
        std::cout << "Original size: width = " << size.width 
                  << ", height = " << size.height << std::endl;
        
        // 角度处理
        if (size.width < size.height) {
            angle += 90;
            std::swap(size.width, size.height);
        }
        
        // 应用横向和纵向拉伸
        size.width *= (1.0f + horizontalStretch);
        size.height *= (1.0f + verticalStretch);
        
        // 打印拉伸后的尺寸
        std::cout << "Stretched size: width = " << size.width 
                  << ", height = " << size.height 
                  << ", horizontal stretch = " << horizontalStretch 
                  << ", vertical stretch = " << verticalStretch << std::endl;
        
        // 确保尺寸正确
        size.width = std::max(std::abs(size.width), 10.0f);
        size.height = std::max(std::abs(size.height), 10.0f);
        
        // 旋转变换
        cv::Mat M = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::Mat rotated;
        cv::warpAffine(img, rotated, M, img.size(), cv::INTER_LINEAR);
        
        // 裁剪
        cv::Mat cropped;
        cv::getRectSubPix(rotated, cv::Size(static_cast<int>(size.width), 
                                            static_cast<int>(size.height)), 
                          center, cropped);
        
        return cropped;
    } catch (const std::exception& e) {
        std::cerr << "Crop Exception: " << e.what() << std::endl;
        return cv::Mat();
    }
}

// ================== Main Function ==================
// int main(int argc, char* argv[]) {
//     try {
//         // Check command line arguments
//         std::string imagePath = "c1.jpg";
//         if (argc > 1) {
//             imagePath = argv[1];
//         }
//         
//         // Initialize models
//         Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "text-detection");
//         Ort::SessionOptions sessionOptions;
//         sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);
//         
//         std::string detModelPath = "det_server.onnx";
//         
//         #ifdef _WIN32
//             std::wstring wideDetModelPath(detModelPath.begin(), detModelPath.end());
//             Ort::Session detSession(env, wideDetModelPath.c_str(), sessionOptions);
//         #else
//             Ort::Session detSession(env, detModelPath.c_str(), sessionOptions);
//         #endif
//         
//         // Preprocess image for text detection
//         auto [inputTensor, originalImg] = preprocessImage(imagePath);
//         
//         // Prepare input tensor for detection
//         std::vector<int64_t> inputShape = {1, 3, originalImg.rows, originalImg.cols};
//         auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
//         
//         Ort::Value inputOnnxTensor = Ort::Value::CreateTensor<float>(
//             memoryInfo, inputTensor.data(), inputTensor.size(),
//             inputShape.data(), inputShape.size());
//         
//         // Get input and output names
//         Ort::AllocatorWithDefaultOptions allocator;
//         
//         // Get input names
//         std::vector<std::string> inputNames;
//         std::vector<const char*> inputNamesCStr;
//         size_t numInputNodes = detSession.GetInputCount();
//         inputNames.resize(numInputNodes);
//         inputNamesCStr.resize(numInputNodes);
//         
//         for (size_t i = 0; i < numInputNodes; i++) {
//             Ort::AllocatedStringPtr input_name = detSession.GetInputNameAllocated(i, allocator);
//             inputNames[i] = input_name.get();
//             inputNamesCStr[i] = inputNames[i].c_str();
//         }
//         
//         // Get output names
//         std::vector<std::string> outputNames;
//         std::vector<const char*> outputNamesCStr;
//         size_t numOutputNodes = detSession.GetOutputCount();
//         outputNames.resize(numOutputNodes);
//         outputNamesCStr.resize(numOutputNodes);
//         
//         for (size_t i = 0; i < numOutputNodes; i++) {
//             Ort::AllocatedStringPtr output_name = detSession.GetOutputNameAllocated(i, allocator);
//             outputNames[i] = output_name.get();
//             outputNamesCStr[i] = outputNames[i].c_str();
//         }
//         
//         // Run text detection
//         auto outputTensors = detSession.Run(
//             Ort::RunOptions{nullptr}, 
//             inputNamesCStr.data(), 
//             &inputOnnxTensor, 
//             1, 
//             outputNamesCStr.data(), 
//             1);
//         
//         // Process detection output
//         float* outputData = outputTensors[0].GetTensorMutableData<float>();
//         std::vector<int64_t> outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
//         
//         // Extract score map from output
//         int height = outputShape[2];
//         int width = outputShape[3];
//         std::vector<float> scoreMap(outputData, outputData + (height * width));
//         
//         // Process detection output to get boxes
//         auto boxes = processDetectionOutput(scoreMap, height, width);
//         std::cout << "Detected " << boxes.size() << " text boxes" << std::endl;
//         
//         // Prepare visualization image
//         cv::Mat visImg;
//         cv::cvtColor(originalImg, visImg, cv::COLOR_RGB2BGR);
//        
//         int box_idx = 0;
//
//         // Process each detected box
//         for (const auto& box : boxes) {
//             // Crop the rotated box content
//             cv::Mat cropped = warpAffineCrop(visImg, box, 0.2f, 1.3f);
//             
//             if (cropped.empty()) continue;  // Skip invalid regions
//            
//             std::string crop_name = "Text Box " + std::to_string(++box_idx);
//             cv::imshow(crop_name, cropped);
//             cv::moveWindow(crop_name, 100 + box_idx * 50, 100);
//             cv::waitKey(0);
//
//             // Visualization
//             std::vector<std::vector<cv::Point>> contours{box};
//             // cv::polylines(visImg, contours, true, cv::Scalar(0, 255, 0), 2);
//         }
//         
//         // Show result
//         cv::Mat resized_result;
//         cv::resize(visImg, resized_result, cv::Size(), 2, cv::INTER_LINEAR);
//         cv::imshow("Detection with Orientation", resized_result);
//         cv::waitKey(0);
//         cv::destroyAllWindows();
//         
//         return 0;
//     } catch (const Ort::Exception& e) {
//         std::cerr << "ONNX Runtime Error: " << e.what() << std::endl;
//         return 1;
//     } catch (const cv::Exception& e) {
//         std::cerr << "OpenCV Error: " << e.what() << std::endl;
//         return 1;
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }
// }


// class chars_det_inferer{
// public:
//     chars_det_inferer(const std::string& model_path = "det_server.onnx")
//         : m_env(ORT_LOGGING_LEVEL_WARNING)
//         #ifdef _WIN32
//         m_session(m_env, std::wstring(model_path.begin(), model_path.end().c_str())
//         #else
//         , m_session(m_env,model_path.c_str(), Ort::SessionOptions{})
//         #endif 
//         { }
//
//         std::vector<cv::Mat> run_inf(cv::Mat& frame) {
//         
//         try {// preprocess image for text detection
//         auto [input_tensor, original_img] = preprocess_img(frame);
//         
//         std::vector<int64_t> input_shape = { 1, 3, original_img.rows, original_img.cols };
//         auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
//
//         Ort::Value input_onnx_tensor = Ort::Value::CreateTensor<float>(
//             memory_info, input_tensor.data(), input_tensor.size(),
//             input_shape.data(), input_shape.size());
//
//         Ort::AllocatorWithDefaultOptions allocator;
//
//         std::vector<std::string> input_names;
//         std::vector<const char*> input_names_c_str;
//         size_t num_input_nodes = m_session.GetInputCount();
//         input_names.resize(num_input_nodes);
//         input_names_c_str.resize(num_input_nodes);
//
//         for (size_t i = 0; i < num_input_nodes; i++) {
//             Ort::AllocatedStringPtr input_name = m_session.GetInputNameAllocated(i, allocator);
//             input_names[i] = input_name.get();
//             input_names_c_str[i] = input_names[i].c_str();
//         }
//
//         std::vector<std::string> output_names;
//         std::vector<const char*> output_names_c_str;
//         size_t num_output_nodes = m_session.GetOutputCount();
//         output_names.resize(num_output_nodes);
//         output_names_c_str.resize(num_output_nodes);
//
//         for (size_t i = 0; i < num_output_nodes; i++) {
//             Ort::AllocatedStringPtr output_name = m_session.GetOutputNameAllocated(i, allocator);
//             output_names[i] = output_name.get();
//             output_names_c_str[i] = output_names[i].c_str();
//         }
//     
//         auto output_tensors = m_session.Run(
//             Ort::RunOptions{ nullptr },
//             input_names_c_str.data(),
//             &input_onnx_tensor,
//             1,
//             output_names_c_str.data(),
//             1);
//
//         float* output_data = output_tensors[0].GetTensorMutableData<float>();
//         std::vector<int64_t> output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
//
//         int height = output_shape[2];
//         int width = output_shape[3];
//         std::vector<float> score_map(output_data, output_data + (height * width));
//
//         auto boxes = process_detection_output(score_map, height, width);
//         qDebug() << "Detected " << boxes.size() << " text boxes";
//         
//         cv::Mat vis_img;
//         cv::cvtColor(original_img, vis_img, cv::COLOR_RGB2BGR);
//
//         int box_idx = 0;
//         std::vector<cv::Mat> chars_croppeds{};
//         for (const auto& box: boxes) {
//             cv::Mat cropped = warp_affine_crop(vis_img, box, chars_croppeds, 0.2f, 1.3f);
//             
//             if (cropped.empty()) continue;
//         }
//
//         for (const auto& cropped: chars_croppeds) {
//             std::string cropped_name = "Text Box " +std::to_string(++box_idx);
//             cv::imshow(cropped_name, cropped);
//             cv::moveWindow(cropped_name, 100 + box_idx * 50, 100);
//             cv::waitKey(0);
//         }
//             return chars_croppeds;
//         // return boxes;
//         } catch (const Ort::Exception& e) {
//             return {};
//         } catch (const cv::Exception& e) {
//             return {};
//         } catch (const std::exception& e) {
//             return {};
//         }
//         return {};
//     }
// private:
//     Ort::Env m_env;
//     Ort::Session m_session;
//
//     std::pair<std::vector<float>, cv::Mat> preprocess_img(cv::Mat& img) {
//         cv::Mat rgb_img;
//         cv::cvtColor(img, rgb_img, cv::COLOR_BGR2RGB);
//         
//         float max_size = 1024.f;
//         float scale = std::min(max_size / rgb_img.cols, max_size / rgb_img.rows);
//         int scaled_w = static_cast<int>(rgb_img.cols * scale);
//         int scaled_h = static_cast<int>(rgb_img.rows * scale);
//
//         scaled_w = (scaled_w + 31) / 32 * 32;
//         scaled_h = (scaled_h + 31) / 32 * 32;
//
//         cv::Mat resized_img;
//         cv::resize(rgb_img, resized_img, cv::Size(scaled_w, scaled_h), 0, 0, cv::INTER_LINEAR);
//
//         std::vector<float> input_tensor(3 * scaled_h * scaled_w);
//         for (int c = 0; c < 3; c++) {
//             for (int h = 0; h < scaled_h; h++) {
//                 for (int w = 0; w < scaled_w; w++) {
//                     input_tensor[c * scaled_h * scaled_w + h * scaled_w + w] = 
//                         (resized_img.at<cv::Vec3b>(h, w)[c] / 255.f - 0.5f) * 2.f;
//                 }
//             }
//         }
//         return { input_tensor, resized_img };
//     }
//
//     std::vector<std::vector<cv::Point>>
//     process_detection_output( const std::vector<float>& output
//                             , int height
//                             , int width
//                             , float threshold = 0.1f
//                             ) {
//         cv::Mat score_map(height, width, CV_32F);
//         for (int h = 0; h < height; h++) {
//             for (int w = 0; w < width; w++) {
//                 score_map.at<float>(h, w) = output[h * width + w];
//             }
//         }
//
//         cv::Mat binary_map;
//         cv::threshold(score_map, binary_map, threshold, 255, cv::THRESH_BINARY);
//         binary_map.convertTo(binary_map, CV_8UC1);
//
//         cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
//         cv::dilate(binary_map, binary_map, kernel, cv::Point(-1, -1), 2);
//
//         std::vector<std::vector<cv::Point>> contours;
//         cv::findContours(binary_map, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
//
//         std::vector<std::vector<cv::Point>> boxes;
//         // 旋转判断有问题
//         for (const auto& contour: contours) {
//             if (contour.size() < 4) continue;
//             
//             std::vector<cv::Point> hull;
//             cv::convexHull(contour, hull);
//
//             cv::RotatedRect rect = cv::minAreaRect(hull);
//             cv::Point2f box_points[4];
//             rect.points(box_points);
//
//             std::vector<cv::Point> box;
//             for (int i = 0; i < 4; i++) {
//                box.push_back(cv::Point( static_cast<int>(box_points[i].x)
//                                       , static_cast<int>(box_points[i].y))); 
//             }
//             boxes.push_back(box);
//         }
//         return boxes;
//     }
//
//     cv::Mat 
//     warp_affine_crop( const cv::Mat& img
//                     , const std::vector<cv::Point>& box
//                     , std::vector<cv::Mat>& chars_croppeds
//                     , float horizontal_stretch = 1.f
//                     , float vertical_stretch = 1.f
//                     ) {
//         try {
//             cv::RotatedRect rect = cv::minAreaRect(box);
//             cv::Point2f center = rect.center;
//             cv::Size2f size = rect.size;
//             float angle = rect.angle;
//
//             if (size.width < size.height) {
//                 angle += 90;
//                 std::swap(size.width, size.height);
//             }
//
//             size.width *= (1.f + horizontal_stretch);
//             size.height *= (1.f + vertical_stretch);
//
//             size.width = std::max(std::abs(size.width), 10.f);
//             size.height = std::max(std::abs(size.height), 10.f);
//
//             cv::Mat M = cv::getRotationMatrix2D(center, angle, 1.0);
//             cv::Mat rotated;
//             cv::warpAffine(img, rotated, M, img.size(), cv::INTER_LINEAR);
//
//             cv::Mat cropped;
//             cv::getRectSubPix( rotated
//                              , cv::Size(static_cast<int>(size.width)
//                                        , static_cast<int>(size.height)
//                                        )
//                              , center
//                              , cropped
//                              ) ;
//             chars_croppeds.push_back(cropped); 
//             return cropped;
//         } catch (const std::exception& e) {
//             qDebug() << "Exception: " << e.what();
//             return cv::Mat();
//         }
//     }
// };

// int main() {
//     chars_det_inferer* det_inferer = new chars_det_inferer();
//     cv::Mat mat = cv::imread("c3.jpg");
//     cv::imshow("start", mat);
//     std::vector<cv::Mat> croppeds = det_inferer->run_inf(mat);
//     int box_idx = 0;
//         for (const auto& cropped: croppeds) {
//             std::string cropped_name = "Text Box " +std::to_string(++box_idx);
//             cv::imshow(cropped_name, cropped);
//             cv::moveWindow(cropped_name, 100 + box_idx * 50, 100);
//             cv::waitKey(0);
//         }
//
//     //cv::waitKey(0);
//
// }



// #include <iostream>
// #include <vector>
// #include <string>
// #include <fstream>
// #include <algorithm>
// #include <cmath>
//
// #include <onnxruntime_cxx_api.h>
// #include <opencv2/opencv.hpp>
//
// class chars_inferer {
// private:
//     Ort::Env env;
//     Ort::Session session;
//     std::vector<std::string> char_dict;
//
//     // 加载字符字典
//     void loadCharDict(const std::string& dict_path) {
//         std::ifstream file(dict_path);
//         std::string line;
//         
//         if (!file.is_open()) {
//             throw std::runtime_error("无法打开字典文件: " + dict_path);
//         }
//
//         while (std::getline(file, line)) {
//             // 去除行首尾空白
//             line.erase(0, line.find_first_not_of(" \t"));
//             line.erase(line.find_last_not_of(" \t") + 1);
//             
//             if (!line.empty()) {
//                 char_dict.push_back(line);
//             }
//         }
//     }
//
//     // 图像预处理
//     cv::Mat preprocessImage(const cv::Mat& input_img, int target_height = 48) {
//         cv::Mat img = input_img.clone();
//         if (img.empty()) {
//             throw std::runtime_error("输入图像为空");
//         }
//
//         cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
//         
//         // 保持宽高比缩放
//         int width = static_cast<int>(img.cols * (static_cast<float>(target_height) / img.rows));
//         cv::resize(img, img, cv::Size(width, target_height));
//
//         // 归一化到 [0, 1]
//         img.convertTo(img, CV_32F, 1.0 / 255.0);
//
//         return img;
//     }
//
// public:
//     // 构造函数
//     chars_inferer(const std::string& model_path, const std::string& dict_path) 
//         : env(ORT_LOGGING_LEVEL_WARNING), 
//           session(env, model_path.c_str(), Ort::SessionOptions{}) 
//     {
//         loadCharDict(dict_path);
//     }
//
//     // 推理接口，直接接受 cv::Mat
//     std::string infer(const cv::Mat& input_image) {
//         cv::Mat input_tensor = preprocessImage(input_image);
//         
//         // 准备输入 - 手动创建 float 数组
//         std::vector<float> input_data;
//         for (int c = 0; c < input_tensor.channels(); ++c) {
//             for (int h = 0; h < input_tensor.rows; ++h) {
//                 for (int w = 0; w < input_tensor.cols; ++w) {
//                     input_data.push_back(input_tensor.at<cv::Vec3f>(h, w)[c]);
//                 }
//             }
//         }
//
//         // 准备输入形状
//         std::vector<int64_t> input_shape = {1, 3, input_tensor.rows, input_tensor.cols};
//
//         // 获取输入和输出节点信息
//         auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
//         Ort::Value input_tensor_ort = Ort::Value::CreateTensor<float>(
//             memory_info, input_data.data(), input_data.size(), input_shape.data(), input_shape.size()
//         );
//
//         // 准备输出
//         std::vector<Ort::Value> outputs;
//         const char* input_names[] = {"x"};
//         const char* output_names[] = {"softmax_2.tmp_0"};
//
//         outputs = session.Run(
//             Ort::RunOptions{}, 
//             input_names, &input_tensor_ort, 1, 
//             output_names, 1
//         );
//
//         // 解析输出
//         float* output_tensor = outputs[0].GetTensorMutableData<float>();
//         std::vector<int64_t> output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
//
//         // 找到每个时间步的最大概率索引
//         std::vector<int> sequence_preds;
//         int sequence_length = output_shape[1];
//         int num_classes = output_shape[2];
//
//         for (int t = 0; t < sequence_length; ++t) {
//             float max_prob = -1;
//             int max_idx = -1;
//             
//             for (int c = 0; c < num_classes; ++c) {
//                 float prob = output_tensor[t * num_classes + c];
//                 if (prob > max_prob) {
//                     max_prob = prob;
//                     max_idx = c;
//                 }
//             }
//             sequence_preds.push_back(max_idx);
//         }
//
//         // 结果解析
//         std::vector<std::string> result;
//         int last_char_idx = -1;
//
//         for (int idx : sequence_preds) {
//             if (idx != last_char_idx) {
//                 int adjusted_idx = idx - 1;
//                 
//                 if (adjusted_idx >= 0 && adjusted_idx < char_dict.size()) {
//                     std::string current_char = char_dict[adjusted_idx];
//                     
//                     // 跳过占位符
//                     if (current_char != "■" && current_char != "<blank>" && current_char != " ") {
//                         result.push_back(current_char);
//                     }
//                     
//                     last_char_idx = idx;
//                 }
//             }
//         }
//
//         // 合并结果
//         std::string final_str;
//         for (const auto& c : result) {
//             final_str += c;
//         }
//
//         return final_str;
//     }
// };
//
// int main() {
//     try {
//         // 初始化推理器
//         chars_inferer inferer(
//             "rec_server.onnx", 
//             "inf_src/classes/chars.txt"
//         );
//
//         // 直接读取图像
//         cv::Mat image = cv::imread("c5.jpg");
//         
//         // 进行推理
//         std::string result = inferer.infer(image);
//         std::cout << "识别结果: " << result << std::endl;
//     }
//     catch (const std::exception& e) {
//         std::cerr << "错误: " << e.what() << std::endl;
//         return -1;
//     }
//
//     return 0;
// }





int main(int argc, char* argv[]) {

    QApplication app(argc, argv);

    mainwindow main_mainwindow;
    main_mainwindow.show();
    return app.exec();
}


// #include <iostream>
// #include <vector>
// #include <string>
// #include <fstream>
// #include <algorithm>
// #include <cmath>
//
// #include <onnxruntime_cxx_api.h>
// #include <opencv2/opencv.hpp>
//
// class OcrRecognizer {
// private:
//     Ort::Env env;
//     Ort::Session session;
//     std::vector<std::string> char_dict;
//
//     // 加载字符字典
//     void loadCharDict(const std::string& dict_path) {
//         std::ifstream file(dict_path);
//         std::string line;
//         
//         if (!file.is_open()) {
//             throw std::runtime_error("无法打开字典文件: " + dict_path);
//         }
//
//         while (std::getline(file, line)) {
//             // 去除行首尾空白
//             line.erase(0, line.find_first_not_of(" \t"));
//             line.erase(line.find_last_not_of(" \t") + 1);
//             
//             if (!line.empty()) {
//                 char_dict.push_back(line);
//             }
//         }
//     }
//
//     // 图像预处理
//     cv::Mat preprocessImage(const std::string& image_path, int target_height = 48) {
//         cv::Mat img = cv::imread(image_path);
//         if (img.empty()) {
//             throw std::runtime_error("无法读取图像: " + image_path);
//         }
//
//         cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
//         
//         // 保持宽高比缩放
//         int width = static_cast<int>(img.cols * (static_cast<float>(target_height) / img.rows));
//         cv::resize(img, img, cv::Size(width, target_height));
//
//         // 归一化到 [0, 1]
//         img.convertTo(img, CV_32F, 1.0 / 255.0);
//
//         return img;
//     }
//
// public:
//     OcrRecognizer(const std::string& model_path, const std::string& dict_path) 
//         : env(ORT_LOGGING_LEVEL_WARNING), 
//           session(env, model_path.c_str(), Ort::SessionOptions{}) 
//     {
//         loadCharDict(dict_path);
//     }
//
//     std::string recognize(const std::string& image_path) {
//         cv::Mat input_tensor = preprocessImage(image_path);
//         
//         // 准备输入 - 手动创建 float 数组
//         std::vector<float> input_data;
//         for (int c = 0; c < input_tensor.channels(); ++c) {
//             for (int h = 0; h < input_tensor.rows; ++h) {
//                 for (int w = 0; w < input_tensor.cols; ++w) {
//                     input_data.push_back(input_tensor.at<cv::Vec3f>(h, w)[c]);
//                 }
//             }
//         }
//
//         // 准备输入形状
//         std::vector<int64_t> input_shape = {1, 3, input_tensor.rows, input_tensor.cols};
//
//         // 获取输入和输出节点信息
//         auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
//         Ort::Value input_tensor_ort = Ort::Value::CreateTensor<float>(
//             memory_info, input_data.data(), input_data.size(), input_shape.data(), input_shape.size()
//         );
//
//         // 准备输出
//         std::vector<Ort::Value> outputs;
//         const char* input_names[] = {"x"};
//         const char* output_names[] = {"softmax_2.tmp_0"};
//
//         outputs = session.Run(
//             Ort::RunOptions{}, 
//             input_names, &input_tensor_ort, 1, 
//             output_names, 1
//         );
//
//         // 解析输出
//         float* output_tensor = outputs[0].GetTensorMutableData<float>();
//         std::vector<int64_t> output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
//
//         // 找到每个时间步的最大概率索引
//         std::vector<int> sequence_preds;
//         int sequence_length = output_shape[1];
//         int num_classes = output_shape[2];
//
//         for (int t = 0; t < sequence_length; ++t) {
//             float max_prob = -1;
//             int max_idx = -1;
//             
//             for (int c = 0; c < num_classes; ++c) {
//                 float prob = output_tensor[t * num_classes + c];
//                 if (prob > max_prob) {
//                     max_prob = prob;
//                     max_idx = c;
//                 }
//             }
//             sequence_preds.push_back(max_idx);
//         }
//
//         // 结果解析
//         std::vector<std::string> result;
//         int last_char_idx = -1;
//
//         for (int idx : sequence_preds) {
//             if (idx != last_char_idx) {
//                 int adjusted_idx = idx - 1;
//                 
//                 if (adjusted_idx >= 0 && adjusted_idx < char_dict.size()) {
//                     std::string current_char = char_dict[adjusted_idx];
//                     
//                     // 跳过占位符
//                     if (current_char != "■" && current_char != "<blank>" && current_char != " ") {
//                         result.push_back(current_char);
//                     }
//                     
//                     last_char_idx = idx;
//                 }
//             }
//         }
//
//         // 合并结果
//         std::string final_str;
//         for (const auto& c : result) {
//             final_str += c;
//         }
//
//         return final_str;
//     }
// };
//
// int main() {
//     try {
//         OcrRecognizer recognizer(
//             "rec_server.onnx", 
//             "inf_src/classes/chars.txt"
//         );
//
//         std::string result = recognizer.recognize("c7.jpg");
//         std::cout << "识别结果: " << result << std::endl;
//     }
//     catch (const std::exception& e) {
//         std::cerr << "错误: " << e.what() << std::endl;
//         return -1;
//     }
//
//     return 0;
// }
// }

