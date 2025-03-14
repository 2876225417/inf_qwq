

#ifdef TRT_INF_HPP
#define TRT_INF_HPP

#include "utils/ort_inf.hpp"
#include <NvInfer.h>
#include <NvInferRuntime.h>
#include <NvInferRuntimeBase.h>
#include <cuda_runtime_api.h>
#include <fstream>
#include <iterator>
#include <memory>
#include <opencv2/core/hal/interface.h>
#include <opencv2/flann/defines.h>
#include <qdebug.h>
#include <qlogging.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <chrono>
#include <any>
#include <tuple>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <NvOnnxParser.h>
#include <chrono>


namespace trt_inf{
class logger: public nvinfer1::ILogger {
public:
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING) qDebug() << msg;
    }
} g_logger;

struct TRT_destroy {
    template <class T>
    void operator()(T* obj) const {
        //if (obj) obj->destroy();
    }
};

using TRT_unique_ptr = std::unique_ptr<nvinfer1::IRuntime, TRT_destroy>;
using TRT_unique_netword_ptr = std::unique_ptr<nvinfer1::INetworkDefinition, TRT_destroy>;
using TRT_unique_builder_ptr = std::unique_ptr<nvinfer1::IBuilder, TRT_destroy>;
using TRT_unique_engine_ptr = std::unique_ptr<nvinfer1::ICudaEngine, TRT_destroy>;
using TRT_unique_context_ptr = std::unique_ptr<nvinfer1::IExecutionContext, TRT_destroy>;
using TRT_unique_parser_ptr = std::unique_ptr<nvonnxparser::IParser, TRT_destroy>;
using TRT_unique_config_ptr = std::unique_ptr<nvinfer1::IBuilderConfig, TRT_destroy>;
using TRT_unique_profile_ptr = std::unique_ptr<nvinfer1::IOptimizationProfile, TRT_destroy>;
using TRT_unique_memory_ptr = std::unique_ptr<nvinfer1::IHostMemory, TRT_destroy>;

class cuda_memory {
private:
    void* m_device_ptr = nullptr;
    size_t m_size = 0;

public:
    cuda_memory() = default;
    cuda_memory(size_t size): m_size(size) { cudaMalloc(&m_device_ptr, size); }
    ~cuda_memory() { if (m_device_ptr) cudaFree(m_device_ptr); }

    void* get() const { return m_device_ptr; }
    size_t size() const { return m_size; }

    void resize(size_t size) { 
        if (m_device_ptr) cudaFree(m_device_ptr);
        m_size = size;
        cudaMalloc(&m_device_ptr, size);
    }

    void copy_from_host(const void* host_data, size_t size) {
        if (size > m_size) resize(size);

        cudaMemcpy(m_device_ptr, host_data, size, cudaMemcpyHostToDevice);
    }

    void copy2host(void* host_data, size_t size) const {
        cudaMemcpy(host_data, m_device_ptr, size, cudaMemcpyDeviceToHost);
    }
};

template <typename InputType, typename OutputType>
class TRT_inferer {
protected:
    TRT_unique_engine_ptr m_engine;
    TRT_unique_context_ptr m_context;

    cudaStream_t m_cuda_stream;
    std::vector<void*> m_device_bindings;
    std::vector<cuda_memory> m_input_device_memory;
    std::vector<cuda_memory> m_output_device_memory;

    std::vector<nvinfer1::Dims> m_input_dims;
    std::vector<nvinfer1::Dims> m_output_dims;
    std::vector<std::string> m_input_names;
    std::vector<std::string> m_output_names;

    virtual std::tuple<std::vector<float>, cv::Mat> preprocess(InputType&) = 0;
    
    #ifdef ENABLE_EIGEN
    virtual std::tuple<std::vector<float>, cv::Mat> preprocess_eigen(InputType&) = 0;
    #endif
    
    virtual OutputType infer(InputType&) = 0;
    virtual OutputType 
    postprocess( std::vector<std::vector<float>>& output_tensors
               , const std::any& additinal_args) = 0;
   
    bool load_engine(const std::string& engine_path) {
        std::ifstream file(engine_path, std::ios::binary);
        if (!file.good()) {
            qDebug() << "Error: Cannot open engine file: " << engine_path;
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> engine_data(size);
        file.read(engine_data.data(), size);
        file.close();

        TRT_unique_ptr
        runtime{nvinfer1::createInferRuntime(g_logger)};
        m_engine.reset(runtime->deserializeCudaEngine(engine_data.data(), size));

        if (!m_engine) {
            qWarning() << "Error: Failed to deserialize CUDA engine";
            return false;
        }

        m_context.reset(m_engine->createExecutionContext());
        if (!m_context) {
            qWarning() << "Error: Failed to create execution context";
            return false;
        }

        cudaStreamCreate(&m_cuda_stream);

        int num_io_tensors = m_engine->getNbIOTensors();
        m_device_bindings.resize(num_io_tensors, nullptr);

        for (int i = 0; i < num_io_tensors; ++i) {
            const char* name = m_engine->getIOTensorName(i);
            bool is_input = m_engine->getTensorIOMode(name) == nvinfer1::TensorIOMode::kINPUT;
            nvinfer1::Dims dims = m_engine->getTensorShape(name);

            if (is_input) {
                m_input_names.push_back(name);
                m_input_dims.push_back(dims);
                m_input_device_memory.emplace_back();
            } else {
                m_output_names.push_back(name);
                m_output_dims.push_back(dims);
                m_output_device_memory.emplace_back();
            }
        }
        return true;
    }

    bool convertONNX2TensorRT( const std::string& onnx_path
                             , const std::string& engine_path
                             , int batch_size = 1
                             , size_t workspace_size = 1 << 30
                             ) {
        TRT_unique_builder_ptr
        builder{nvinfer1::createInferBuilder(g_logger)};
        if (!builder) { 
            qWarning() << "Error: Failed to create build";
            return false;
        }

        const auto explicit_batch = 1U << static_cast<uint32_t>(nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
        TRT_unique_netword_ptr
        network{builder->createNetworkV2(explicit_batch)};
        if (!network) {
            qWarning() << "Error: Failed to create netword";
            return false;
        }

        TRT_unique_parser_ptr
        parser{nvonnxparser::createParser(*network, g_logger)};
        if (!parser) {
            qWarning() << "Error: Failed to create parser";
            return false;
        }

        bool parsed = parser->parseFromFile( onnx_path.c_str()
                                           , static_cast<int>(nvinfer1::ILogger::Severity::kWARNING));
        if (!parsed) {
            qWarning() << "Error: Failed to parse ONNX file";
            return false;
        }

        TRT_unique_config_ptr
        config{builder->createBuilderConfig()};
        if (!config) {
            qWarning() << "Error: Failed to create builder config";
            return false;
        }

        config->setMemoryPoolLimit(nvinfer1::MemoryPoolType::kWORKSPACE, workspace_size);
        
        bool has_dynamic_inputs = false;
        for (int i = 0; i < network->getNbInputs(); i++) {
            nvinfer1::ITensor* input = network->getInput(i);
            nvinfer1::Dims dims = input->getDimensions();


            for (int j = 0; j < dims.nbDims; j++) {
                if (dims.d[j] == -1) {
                    has_dynamic_inputs = true;
                    break;
                }
            }
            if (has_dynamic_inputs) break;
        }
        
        if (has_dynamic_inputs) {
            TRT_unique_profile_ptr
            profile{builder->createOptimizationProfile()};
            if(!profile) {
                qWarning() << "Error: Failed to create optimization profile";
                return false;
            }

            for (int i = 0; i < network->getNbInputs(); i++) {
                nvinfer1::ITensor* input = network->getInput(i);
                const char* input_name = input->getName();
                nvinfer1::Dims dims = input->getDimensions();

                qDebug() << "Input name: " << input_name;
                qDebug() << "Input dimension: ";
                for (int j = 0; j < dims.nbDims; j++) {
                    qDebug() << "  dim[" << j << "] = " << dims.d[j];
                }

                nvinfer1::Dims min_dims = dims;
                nvinfer1::Dims opt_dims = dims;
                nvinfer1::Dims max_dims = dims;

                for (int j = 0; j < dims.nbDims; j++) {
                    if (dims.d[j] == -1) {
                        if (j == 0) {
                            min_dims.d[j] = 1;
                            opt_dims.d[j] = batch_size;
                            max_dims.d[j] = batch_size * 2;
                        } else {
                            min_dims.d[j] = 1;
                            opt_dims.d[j] = 512;
                            max_dims.d[j] = 1024;
                        }
                    }
                }

                bool all_valid = true;
                for (int j = 0; j < min_dims.nbDims; j++) {
                    if (min_dims.d[j] < 0 || opt_dims.d[j] < 0 || max_dims.d[j] < 0) {
                        all_valid = false;
                        qWarning() << "Invalid dimension value at index " << j << ":"
                                   << "min = " << min_dims.d[j]
                                   << "opt = " << opt_dims.d[j]
                                   << "max = " << max_dims.d[j];
                        break;
                    }
                }

                if (!all_valid) {
                    qWarning() << "Skipping profile setup due to invalid dimensions";
                    continue;
                }
                profile->setDimensions(input_name, nvinfer1::OptProfileSelector::kMIN, min_dims);
                profile->setDimensions(input_name, nvinfer1::OptProfileSelector::kOPT, opt_dims);
                profile->setDimensions(input_name, nvinfer1::OptProfileSelector::kMAX, max_dims);
            }
            config->addOptimizationProfile(profile.get());
        }
        
        if (builder->platformHasFastFp16()) {
            config->setFlag(nvinfer1::BuilderFlag::kFP16);
        }

        TRT_unique_memory_ptr
        serialized_engine{builder->buildSerializedNetwork(*network, *config)};
        if (!serialized_engine) {
            qWarning() << "Error: Failed to build serialized engine";
            return false;
        }
        
        std::ofstream engine_file(engine_path, std::ios::binary);
        if (!engine_file) {
            qWarning() << "Failed to open engine file for writing";
            return false;
        }

        engine_file.write(static_cast<const char*>(serialized_engine->data()), serialized_engine->size());
        engine_file.close();

        return true;
    }
public:
    TRT_inferer(const std::string& model_path, bool force_rebuild = false) {
        std::string engine_path = model_path.substr(0, model_path.find_first_of('.')) + ".engine";

        std::fstream engine_file(engine_path, std::ios::binary);
        if (!engine_file.good() || force_rebuild) {
            qDebug() << "Converting ONNX model to TensorRT engine...";
            if (!convertONNX2TensorRT(model_path, engine_path)) {
                throw std::runtime_error("Failed to convert ONNX model to TensorRT engine");
            }
            qDebug() << "Conversion completed. Engine saved to: " << engine_path;
        }
        if(!load_engine(engine_path)) {
            throw std::runtime_error("Failed to load TensorRT engine");
        }
        qDebug() << "TensorRT Inferer intialized with model: " << model_path;
    }

    ~TRT_inferer() {
        cudaStreamDestroy(m_cuda_stream);
    }
};

struct det_post_arg { cv::Mat resize_img; };

class TRT_det_inferer: public TRT_inferer<cv::Mat, std::vector<cv::Mat>> {
public:
    TRT_det_inferer( const std::string& model_path = "det_server.onnx"
                   , bool force_rebuild = false)
                   : TRT_inferer<cv::Mat, std::vector<cv::Mat>>(model_path, force_rebuild)
                   { }
    std::vector<cv::Mat> run_inf(cv::Mat& frame) { return infer(frame); }
    
    void set_det_threshold(float threhsold) { m_threshold = threhsold; }
private:
    float m_threshold = 0.3f;

    std::vector<cv::Mat> infer(cv::Mat& frame) override {
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
                qDebug() << "OpenCV preprocess";
                #endif
            #endif
            
            auto end_preprocess_det = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_preprocess_det - start_preprocess_det);
            qDebug() << "Time cost: " << duration;

            int input_idx = 0;
            for (const auto& name: m_input_names) {
                nvinfer1::Dims dims = m_input_dims[input_idx];
                size_t size = 1;
                for (int j = 0; j < dims.nbDims; j++) {
                    size *= dims.d[j];
                }
                size *= sizeof(float);

                m_input_device_memory[input_idx].resize(size);
                
                m_input_device_memory[input_idx].copy_from_host(input_tensor_values.data(), size);

                m_context->setTensorAddress(name.c_str(), m_input_device_memory[input_idx].get());
                input_idx++;
            }

            int output_idx = 0;
            for (const auto& name: m_output_names) {
                nvinfer1::Dims dims = m_output_dims[output_idx];

                size_t size = 1;
                for (int j = 0; j < dims.nbDims; j++) {
                    size *= dims.d[j];
                }
                size *= sizeof(float);

                m_output_device_memory[output_idx].resize(size);

                m_context->setTensorAddress(name.c_str(), m_output_device_memory[output_idx].get());
                output_idx++;
            }
                
            auto start_infer = std::chrono::high_resolution_clock::now();

            bool status = m_context->enqueueV3(m_cuda_stream);
            if (!status) {
                throw std::runtime_error("TensorRT execution failed");
            }
            cudaStreamSynchronize(m_cuda_stream);

            auto end_infer = std::chrono::high_resolution_clock::now();
            auto infer_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_infer - start_infer);
            qDebug() << "TensorRT Infer time cost: " << infer_duration;

            std::vector<std::vector<float>> output_tensors;
            output_idx = 0;
            for (const auto& dims: m_output_dims) {
                size_t elem_count = 1;
                for (int j = 0; j < dims.nbDims; j++) {
                    elem_count *= dims.d[j];
                }

                std::vector<float> output_data(elem_count);
                m_output_device_memory[output_idx].copy2host(output_data.data(), elem_count * sizeof(float));
                output_tensors.push_back(std::move(output_data));
                        
                output_idx++;
            }
            det_post_arg args{resized_img};
            return postprocess(output_tensors, args); 
        } catch (const std::exception& e) {
            qDebug() << "TensorRT Error: " << e.what();
            return {};
        } 
    }
    
    std::tuple<std::vector<float>, cv::Mat> preprocess(cv::Mat& frame) override {
        if (frame.empty()) {
            qDebug() << "Invalid input frame";
            return {{}, {}};
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
        return {input_tensor, resized_img};
    }

    #ifdef ENABLE_EIGEN
    std::tuple<std::vector<float>, cv::Mat>
    preprocess_eigen(cv::Mat& frame) override {
        if (frame.empty()) {
            qDebug() << "Invalid input frame";
            return {{}, {}};
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
        resized_img.convertTo(float_img, CV_32FC3, 1.f / 255.f);

        std::vector<float> input_tensor(1 * 3 * new_h * new_w);
        
        Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        eigen_img((float*)float_img.data, new_h, new_w * 3);

        for (int c = 0; c < 3; c++) {
            for (int h = 0; h < new_h; h++) {
                for (int w = 0; w < new_w; w++) {
                    input_tensor[c * new_h * new_w + h * new_w + w] = eigen_img(h, w * 3 + c);
                }
            }
        }
        return {input_tensor, resized_img}; 
    }
    #endif

    std::vector<cv::Mat> postprocess( std::vector<std::vector<float>>& output_tensors
                                    , const std::any& additional_args) override {
        const auto& args = std::any_cast<det_post_arg>(additional_args);
        cv::Mat resized_img = args.resize_img;
        std::vector<cv::Mat> croppeds{};

        if (output_tensors.empty()) {
            qDebug() << "Empty output tensors";
            return croppeds;
        }

        nvinfer1::Dims output_dims = m_output_dims[0];
        int output_h = output_dims.d[1];
        int output_w = output_dims.d[2];

        std::vector<float>& output_data = output_tensors[0];

        auto boxes = process_detection_output(output_data, output_h, output_w, m_threshold);
        
        qDebug() << "Count of detected boxes: " << boxes.size();
        
        cv::Mat vis_img;
        cv::cvtColor(resized_img, vis_img, cv::COLOR_RGB2BGR);

        for (size_t i = 0; i < boxes.size(); i++) {
            float horizontal_ratio = 0.2f;
            float vertical_ratio = 0.5f;

            cv::Rect expanded_rect = expand_box(boxes[i], horizontal_ratio, vertical_ratio, vis_img.size());
            cv::Mat cropped = vis_img(expanded_rect).clone();
            croppeds.push_back(cropped);
        }
        return croppeds;
    }

    std::vector<std::vector<cv::Point>> 
    process_detection_output( const std::vector<float>& output
                            , int height, int width
                            , float threshold = 0.3f
                            ) {
        cv::Mat score_map(height, width, CV_32F, const_cast<float*>(output.data()));

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
                box.push_back(cv::Point(
                    static_cast<int>(vertices[i].x),
                    static_cast<int>(vertices[i].y)
                ));
            }
            boxes.push_back(box);
        }
        return boxes;
    }

    cv::Rect expand_box( const std::vector<cv::Point>& box
                       , float horizontal_ratio, float vertical_ratio
                       , const cv::Size& img_shape) {
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

}
#endif
