

# cmake/FindONNXRuntime.cmake
cmake_minimum_required(VERSION 3.16)

set(3RDPARTY "${CMAKE_SOURCE_DIR}/3rdparty")

if(ENABLE_GPU)
    set(ONNXRUNTIME_INCLUDE_DIR "${3RDPARTY}/onnxruntime_gpu/include")
    set(ONNXRUNTIME_LIB "${3RDPARTY}/onnxruntime_gpu/lib")
    set(ONNXRUNTIME_TYPE "GPU")
else()
    set(ONNXRUNTIME_INCLUDE_DIR "${3RDPARTY}/onnxruntime/include")
    set(ONNXRUNTIME_LIB "${3RDPARTY}/onnxruntime/lib")
    set(ONNXRUNTIME_TYPE "CPU")
endif()

if (NOT EXISTS "${ONNXRUNTIME_INCLUDE_DIR}/onnxruntime_c_api.h")
    message(FATAL_ERROR "Headers of ONNXRuntime ${ONNXRUNTIME_TYPE} is incomplete: ${ONNXRUNTIME_INCLUDE_DIR}")
endif()

find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime
    PATHS "${ONNXRUNTIME_LIB}"
    NO_DEFAULT_PATH
    REQUIRED
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ONNXRuntime
    REQUIRED_VARS
        ONNXRUNTIME_INCLUDE_DIR
        ONNXRUNTIME_LIBRARY
)

mark_as_advanced(
    ONNXRUNTIME_INCLUDE_DIR
    ONNXRUNTIME_LIB
)

if (NOT TARGET onnxruntime::onnxruntime)
    add_library(onnxruntime::onnxruntime UNKNOWN IMPORTED)
    set_target_properties(onnxruntime::onnxruntime PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
        IMPORTED_LOCATION "${ONNXRUNTIME_LIBRARY}"
    )

    if(ENABLE_GPU)
        find_package(CUDA REQUIRED)
        set_property(TARGET onnxruntime::onnxruntime APPEND PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${CUDA_INCLUDE_DIRS})
    endif()
endif()
