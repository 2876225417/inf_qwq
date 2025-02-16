
function(download_onnxruntime)
    set(options ENABLE_GPU)
    cmake_parse_arguments(ORT "${options}" "" "" ${ARGN})

    set(3RDPARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty")
    set(ONNXRUNTIME_ROOT "${3RDPARTY_DIR}/onnxruntime")

    if(NOT EXISTS "${ONNXRUNTIME_ROOT}/LICENSE")
        if(CMAKE_SYSTEM_NAME MATCHES "Linux")
            set(ORT_OS "linux")
            set(EXT "tgz")
        elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
            set(ORT_OS "win")
            set(EXT "zip")
        else()
            message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
        endif()

        if(ORT_ENABLE_GPU)
            set(GPU_SUFFIX "-gpu")
            set(VALIDATE_FILE "cuda")
        else()
            set(GPU_SUFFIX "")
            set(VALIDATE_FILE "LICENSE")
        endif()

        set(ORT_FILENAME "onnxruntime-${ORT_OS}-x64${GPU_SUFFIX}-1.20.1.${EXT}")
        set(DOWNLOAD_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/${ORT_FILENAME}")

        message(STATUS "Downloading ONNX Runtime: ${DOWNLOAD_URL}")
        file(DOWNLOAD
            ${DOWNLOAD_URL}
            "${3RDPARTY_DIR}/${ORT_FILENAME}"
            TLS_VERIFY ON
            SHOW_PROGRESS
        )

        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf "${3RDPARTY_DIR}/${ORT_FILENAME}"
            WORKING_DIRECTORY ${3RDPARTY_DIR}
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "解压失败: ${result}")
        endif()

        file(REMOVE "${3RDPARTY_DIR}/${ORT_FILENAME}")
        if(NOT EXISTS "${ONNXRUNTIME_ROOT}/${VALIDATE_FILE}")
            message(FATAL_ERROR "安装验证失败，请检查下载文件完整性")
        endif()
    endif()
    set(onnxruntime_DIR "${ONNXRUNTIME_ROOT}/lib/cmake/onnxruntime" PARENT_SCOPE)
endfunction()
