

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

#include <iostream>


class ort_inferer {
public:
    ort_inferer() {
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ort_inferer");
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);

        const std::string model_path = "/* model path */ ";
        Ort::Session session(env, model_path.c_str(), session_options);

        const int MODEL_HEIGHT = 48;
        cv::Mat img = cv::imread("25.jpg");

        if (img.empty()) {
            throw std::runtime_error("Failed to load image: 25.jpg");
        }

        cv::Mat input_blob = preprocess_image(img, MODEL_HEIGHT);
        
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
            OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault
        );

        std::vector<int64_t> input_shape = {
            1, 3, MODEL_HEIGHT, input_blob.size[3]
        };

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            input_blob.ptr<float>(),
            input_blob.total(),
            input_shape.data(),
            input_shape.size()
        );

        std::vector<const char*> input_names = {"x"};
        std::vector<const char*> output_names = {"save_infer_model/scale_0.tmp_0"};

        auto output_tensors = session.Run(
            Ort::RunOptions{nullptr},
            input_names.data(),
            &input_tensor,
            1,
            output_names.data(),
            1
        );

        if (!output_tensors.empty() && output_tensors[0].IsTensor()) {
            float* output_data = output_tensors[0].GetTensorMutableData<float>();
            auto shape_info = output_tensors[0].GetTensorTypeAndShapeInfo();
            std::vector<int64_t> output_shape = shape_info.GetShape();
            
            if (output_shape.size() == 3) {
                const int seq_len = output_shape[1];
                const int num_classes = output_shape[2];

                std::vector<int> predictions;
                for (int t = 0; t < seq_len; ++t) {
                    float* timestep_data = output_data + t * num_classes;
                    auto max_it = std::max_element(timestep_data, timestep_data + num_classes);
                    predictions.push_back(std::distance(timestep_data, max_it));
                }

                std::string result;
                int last_index = -1;
                for (int index: predictions) {
                   int mapped_index = index - 1;
                    
                    if (mapped_index >= 0 && mapped_index < char_dict.size()) {
                        if (mapped_index != last_index) {
                            result += char_dict[mapped_index];
                            last_index = mapped_index;
                        }
                    }
                }

                std::cout << "Inferred result: \033[1;32m" << result << "\033[0m\n";

                std::cerr << "Orginal infferd index: ";
                for (int idx: predictions) std::cerr << idx << ' ';
                std::cerr << std::endl;

            }
        }
    }
private:
    cv::Mat preprocess_image(const cv::Mat& frame, int target_height) {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        /** keep ratio
         *  get transformed scale -> target_height / rows
         */
        float scale = static_cast<float>(target_height) / frame.rows;
        int target_width = static_cast<int>(frame.cols * scale);
        cv::resize(frame, frame, cv::Size(target_width, target_height));
        // normalize
        frame.convertTo(frame, CV_32F, 1.f / 255.f);
        cv::Mat channels[3];
        cv::split(frame, channels);

        cv::Mat blob;
        cv::vconcat(channels[0].reshape(1, 1), channels[1].reshape(1, 1), blob);
        cv::vconcat(blob, channels[2].reshape(1, 1), blob);
        return blob.reshape(1, {1, 3, target_height, target_width}).clone();
    }

    const std::vector<char> char_dict = {
        '-',
        '.',
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
    }; 
};

#endif
