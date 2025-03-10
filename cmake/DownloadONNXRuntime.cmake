# cmake/DownloadONNXRuntime.cmake

option(ENABLE_GPU "Enable GPU support for ONNX Runtime" OFF)

if(NOT DEFINED ONNXRUNTIME_VERSION)
    set(ONNXRUNTIME_VERSION "1.21.0")
endif()

if(ENABLE_GPU)
    if(NOT DEFINED ONNXRUNTIME_DOWNLOAD_DIR)
        set(ONNXRUNTIME_DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime_gpu")
    endif()
    set(ONNXRUNTIME_VARIANT "gpu")
else()
    if(NOT DEFINED ONNXRUNTIME_DOWNLOAD_DIR)
        set(ONNXRUNTIME_DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime")
    endif()
    set(ONNXRUNTIME_VARIANT "")
endif()

if(WIN32)
    set(ONNXRUNTIME_OS "win")
    set(ONNXRUNTIME_DOWNLOAD_FILE_POSTFIX "zip")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ONNXRUNTIME_ARCH "x64")
    else()
        set(ONNXRUNTIME_ARCH "X86")
    endif()
elseif(APPLE)
    set(ONNXRUNTIME_DOWNLOAD_FILE_POSTFIX "tgz")
    set(ONNXRUNTIME_OS "osx")
    set(ONNXRUNTIME_ARCH "x64")
elseif(UNIX AND NOT APPLE)
    set(ONNXRUNTIME_OS "linux")
    set(ONNXRUNTIME_DOWNLOAD_FILE_POSTFIX "tgz")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(ONNXRUNTIME_ARCH "aarch64")
    else()
        set(ONNXRUNTIME_ARCH "x64")
    endif()
else()
    message(FATAL_ERROR "Unsupported platform for ONNX Runtime download")
endif()

if(ENABLE_GPU)
    set(ONNXRUNTIME_DOWNLOAD_URL "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/onnxruntime-${ONNXRUNTIME_OS}-${ONNXRUNTIME_ARCH}-gpu-${ONNXRUNTIME_VERSION}.${ONNXRUNTIME_DOWNLOAD_FILE_POSTFIX}")
    set(ONNXRUNTIME_TYPE_STR "GPU")
else()
    set(ONNXRUNTIME_DOWNLOAD_URL "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/onnxruntime-${ONNXRUNTIME_OS}-${ONNXRUNTIME_ARCH}-${ONNXRUNTIME_VERSION}.${ONNXRUNTIME_DOWNLOAD_FILE_POSTFIX}")
    set(ONNXRUNTIME_TYPE_STR "CPU")
endif()

set(ONNXRUNTIME_EXTRACT_DIR "${ONNXRUNTIME_DOWNLOAD_DIR}")
if(ENABLE_GPU)
    set(ONNXRUNTIME_DOWNLOAD_FILE "${ONNXRUNTIME_DOWNLOAD_DIR}/onnxruntime-gpu-${ONNXRUNTIME_VERSION}.${ONNXRUNTIME_DOWNLOAD_FILE_POSTFIX}")
else()
    set(ONNXRUNTIME_DOWNLOAD_FILE "${ONNXRUNTIME_DOWNLOAD_DIR}/onnxruntime-${ONNXRUNTIME_VERSION}.${ONNXRUNTIME_DOWNLOAD_FILE_POSTFIX}")
endif()

if(NOT EXISTS "${ONNXRUNTIME_EXTRACT_DIR}/include/onnxruntime_c_api.h")
    file(MAKE_DIRECTORY "${ONNXRUNTIME_DOWNLOAD_DIR}")
    message(STATUS "Downloading ONNX Runtime ${ONNXRUNTIME_VARIANT} v${ONNXRUNTIME_VERSION} from ${ONNXRUNTIME_DOWNLOAD_URL}")
    file(DOWNLOAD
        "${ONNXRUNTIME_DOWNLOAD_URL}"
        "${ONNXRUNTIME_DOWNLOAD_FILE}"
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS
    )

    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0) 
        list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
        message(FATAL_ERROR "Failed to download ONNX Runtime: ${ERROR_MESSAGE}")
    endif()
    # todo: Extract error
    # description: Extacting with an extra folder wrapped
    message(STATUS "Extracting ONNX Runtime to ${ONNXRUNTIME_EXTRACT_DIR}")

    set(TEMP_EXTRACT_DIR "${ONNXRUNTIME_DOWNLOAD_DIR}/temp_extract")
    file(MAKE_DIRECTORY "${TEMP_EXTRACT_DIR}")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${ONNXRUNTIME_DOWNLOAD_FILE}"
        WORKING_DIRECTORY "${TEMP_EXTRACT_DIR}"
        RESULT_VARIABLE EXTRACT_RESULT
    )

    if(NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract ONNX Runtime")
    endif()

    file(GLOB EXTRACTED_DIR "${TEMP_EXTRACT_DIR}/onnxruntime*")
    file(GLOB_RECURSE EXTRACTED_FILES "${EXTRACTED_DIR}/*")

    foreach(FILE ${EXTRACTED_FILES})
        file(RELATIVE_PATH REL_FILE "${EXTRACTED_DIR}" "${FILE}")
        if(NOT IS_DIRECTORY "${FILE}")
            file(COPY "${FILE}" DESTINATION "${ONNXRUNTIME_EXTRACT_DIR}/${REL_FILE}")
        else()
            file(MAKE_DIRECTORY "${ONNXRUNTIME_EXTRACT_DIR}/${REL_FILE}")
        endif()
    endforeach()
    
    file(REMOVE_RECURSE "${TEMP_EXTRACT_DIR}")
    file(REMOVE "${ONNXRUNTIME_DOWNLOAD_FILE}")

    message(STATUS "ONNX Runtime ${ONNXRUNTIME_VARIANT} v${ONNXRUNTIME_VERSION} has been downloaded and extracted to ${ONNXRUNTIME_EXTRACT_DIR}")
else()
    message(STATUS "ONNX Runtime ${ONNXRUNTIME_VARIANT} v${ONNXRUNTIME_VERSION} already exists at ${ONNXRUNTIME_EXTRACT_DIR}")
endif()

