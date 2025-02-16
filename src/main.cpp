#include "onnxruntime_c_api.h"
#include <iostream>

#include <onnxruntime_cxx_api.h>
#include <utils/ort_inf.h>

int main() {
 
    Ort::Env(ORT_LOGGING_LEVEL_WARNING, "test");
    std::cout << "Hello!";
}