#include <utils/images_process.h>
#include <opencv2/opencv.hpp>


cv::Mat preprocess_image(const cv::Mat& img, int target_height) {
    cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

    float scale = static_cast<float>(target_height) / img.rows;
    int target_width = static_cast<int>(img.cols * scale);
    cv::resize(img, img, cv::Size(target_width, target_height));

    img.convertTo(img, CV_32F, 1.f / 255.f);
    cv::Mat channels[3];
    cv::split(img, channels);

    cv::Mat blob;
    cv::vconcat(channels[0].reshape(1, 1), channels[1].reshape(1, 1), blob);
    cv::vconcat(blob, channels[2].reshape(1, 1), blob);
    return blob.reshape(1, {1, 3, target_height, target_width}).clone();
}