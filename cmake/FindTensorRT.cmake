

# cmake/FindTensorRT.cmake

set(_TensorRT_PATHS
    ${TensorRT_ROOT_DIR}
    $ENV{TensorRT_ROOT_DIR}
    $ENV{TRT_ROOT}
    /usr
    /usr/local
    /usr/local/tensorrt
    /opt/tensorrt
    /usr/lib/tensorrt
)

find_path(TensorRT_INCLUDE_DIR
    NAMES NvInfer.h
    PATHS ${_TensorRT_PATHS}
    PATH_SUFFIXES include include/x86_64-linux-gnu
    HINTS 
          "/usr/include/x86_64-linux-gnu"
          "/usr/include"
          ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
    DOC "TensorRT include directory"
)

find_library(TensorRT_LIBRARY
    NAMES nvinfer
    PATHS ${_TensorRT_PATHS}
    PATH_SUFFIXES lib lib64 lib/x86_64-linux-gnu
    HINTS 
          "/usr/lib/x86_64-linux-gnu"
          "/usr/lib"
          $ENV{TRT_ROOT}/lib/
          ${CMAKE_CUDA_TOOLKIT_LIBRARY_DIR}
    DOC "TensorRT core library"
)

if(TensorRT_LIBRARY)
    get_filename_component(TensorRT_LIBRARY_DIR ${TensorRT_LIBRARY} DIRECTORY)
endif()

find_library(TensorRT_PLUGIN_LIBRARY
    NAMES nvinfer_plugin
    PATHS ${_TensorRT_PATHS}
    PATH_SUFFIXES lib lib64 lib/x86_64-linux-gnu
    HINTS ${TensorRT_LIBRARY_DIR}
    DOC "TensorRT plugins library"
)

find_library(TensorRT_ONNX_LIBRARY
    NAMES nvonnxparser
    PATHS ${_TensorRT_PATHS}
    PATH_SUFFIXES lib lib64 lib/x86_64-linux-gnu
    HINTS ${TensorRT_LIBRARY_DIR}
    DOC "TensorRT ONNX parser library"
)

find_library(TensorRT_STATIC_LIBRARY
    NAMES nvinfer_static
    PATHS ${_TensorRT_PATHS}
    PATH_SUFFIXES lib lib64 lib/x86_64-linux-gnu
    HINTS ${TensorRT_LIBRARY_DIR}
    DOC "TensorRT static library"
)


if(TensorRT_INCLUDE_DIR)
    if(EXISTS "${TensorRT_INCLUDE_DIR}/NvInferVersion.h")
        file(READ "${TensorRT_INCLUDE_DIR}/NvInferVersion.h" _tensorrt_version_content)
        
        string(REGEX MATCH "define NV_TENSORRT_MAJOR ([0-9]+)" _ "${_tensorrt_version_content}")
        set(TensorRT_MAJOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "define NV_TENSORRT_MINOR ([0-9]+)" _ "${_tensorrt_version_content}")
        set(TensorRT_MINOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "define NV_TENSORRT_PATCH ([0-9]+)" _ "${_tensorrt_version_content}")
        set(TensorRT_PATCH ${CMAKE_MATCH_1})

        SET(TensorRT_VERSION "${TensorRT_MAJOR}.${TensorRT_MINOR}.${TensorRT_PATCH}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TensorRT
    REQUIRED_VARS
        TensorRT_INCLUDE_DIR
        TensorRT_LIBRARY
        TensorRT_PLUGIN_LIBRARY
        TensorRT_ONNX_LIBRARY
    VERSION_VAR TensorRT_VERSION
)

if(TensorRT_FOUND)
    set(TensorRT_INCLUDE_DIRS ${TensorRT_INCLUDE_DIR})
    set(TensorRT_LIBRARIES
        ${TensorRT_LIBRARY}
        ${TensorRT_PLUGIN_LIBRARY}
        ${TensorRT_ONNX_LIBRARY}
    )

    if(TensorRT_STATIC_LIBRARY)
        list(APPEND TensorRT_LIBRARIES ${TensorRT_STATIC_LIBRARY})
    endif()

    message(STATUS "Found TensorRT: version ${TensorRT_VERSION}")
    message(STATUS "- Include path: ${TensorRT_INCLUDE_DIRS}")
    message(STATUS "- Libraries: ${TensorRT_LIBRARIES}")

    mark_as_advanced(
        TensorRT_INCLUDE_DIR
        TensorRT_LIBRARY
        TensorRT_PLUGIN_LIBRARY
        TensorRT_ONNX_LIBRARY
        TensorRT_STATIC_LIBRARY
        TensorRT_VERSION
    )
endif()
