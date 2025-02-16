#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <ratio>
#include <vector>
#include <algorithm>
#include <chrono>

// 更新后的字符映射表 (索引从0开始)
const std::vector<char> char_dict = {
    '-',  // 索引0
    '.',  // 索引1
    '0',  // 索引2
    '1',  // 索引3
    '2',  // 索引4 
    '3',  // 索引5 
    '4',  // 索引6
    '5',  // 索引7
    '6',  // 索引8
    '7',  // 索引9 
    '8',  // 索引10
    '9'   // 索引11
};

cv::Mat preprocess_image(const std::string& image_path, int target_height) {
    cv::Mat img = cv::imread(image_path);
    if (img.empty()) {
        throw std::runtime_error("无法加载图像: " + image_path);
    }

    // 转换颜色空间 BGR -> RGB
    cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

    // 保持宽高比调整尺寸
    float scale = static_cast<float>(target_height) / img.rows;
    int target_width = static_cast<int>(img.cols * scale);
    cv::resize(img, img, cv::Size(target_width, target_height));

    // 转换到CHW格式并归一化
    img.convertTo(img, CV_32F, 1.0/255.0);
    cv::Mat channels[3];
    cv::split(img, channels);

    // 拼接为NCHW格式
    cv::Mat blob;
    cv::vconcat(channels[0].reshape(1,1), channels[1].reshape(1,1), blob);
    cv::vconcat(blob, channels[2].reshape(1,1), blob);
    return blob.reshape(1, {1, 3, target_height, target_width}).clone();
}

int main() {
    auto start_time_with_model = std::chrono::high_resolution_clock::now();
    // 初始化ONNX环境
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "DigitRecognition");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);

    // 加载模型 (确认模型路径)
    const std::string model_path = "model.onnx";
    Ort::Session session(env, model_path.c_str(), session_options);

    // 预处理设置
    const int MODEL_HEIGHT = 48;
    cv::Mat input_blob = preprocess_image("25.jpg", MODEL_HEIGHT);

    // 输入Tensor构造
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    std::vector<int64_t> input_shape = {
        1, 3, MODEL_HEIGHT, input_blob.size[3] // 动态宽度
    };

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        input_blob.ptr<float>(),
        input_blob.total(),
        input_shape.data(),
        input_shape.size()
    );

    // 节点名称验证确保正确
    std::vector<const char*> input_names = {"x"};
    std::vector<const char*> output_names = {"save_infer_model/scale_0.tmp_0"};

    // 执行推理
    auto start_time_with_inf = std::chrono::high_resolution_clock::now();
 
    auto output_tensors = session.Run(
        Ort::RunOptions{nullptr},
        input_names.data(),
        &input_tensor,
        1,
        output_names.data(),
        1
    );

    // 解析结果
    if (!output_tensors.empty() && output_tensors[0].IsTensor()) {
        float* output_data = output_tensors[0].GetTensorMutableData<float>();
        auto shape_info = output_tensors[0].GetTensorTypeAndShapeInfo();
        std::vector<int64_t> output_shape = shape_info.GetShape();

        // 确保是3D输出 [1, seq_len, num_classes]
        if (output_shape.size() == 3) {
            const int seq_len = output_shape[1];
            const int num_classes = output_shape[2];
            
            std::vector<int> predictions;
            for (int t = 0; t < seq_len; ++t) {
                float* timestep_data = output_data + t * num_classes;
                auto max_it = std::max_element(timestep_data, timestep_data + num_classes);
                predictions.push_back(std::distance(timestep_data, max_it));
            }

            // 解码逻辑：去重复+映射处理
            std::string result;
            int last_index = -1;
            for (int index : predictions) {
                // 转换到实际字符索引
                int mapped_index = index - 1;  // 根据训练映射调整偏移
                
                if (mapped_index >=0 && mapped_index < char_dict.size()) {
                    if (mapped_index != last_index) {
                        result += char_dict[mapped_index];
                        last_index = mapped_index;
                    }
                }
            }

            std::cout << "识别结果: \033[1;32m" << result << "\033[0m\n";

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration_with_model = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_with_model);
            auto duration_with_inf = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_with_inf);

            std::cerr << "识别耗时(含加载模型): \033[1;33m" << duration_with_model.count() << "ms\033[0m" << std::endl;
            std::cerr << "识别耗时(仅推理): \033[1;33m" << duration_with_inf.count() << "ms\033[0m" << std::endl;



            std::cerr << "原始预测索引: ";
            for (int idx : predictions) std::cerr << idx << " ";
            std::cerr << std::endl;
        }
    }

    return 0;
}
