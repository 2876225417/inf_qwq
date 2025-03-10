

# cmake/FindCUDNN.cmake

include(FindPackageHandleStandardArgs)


# some common installation dirs
set(_CUDNN_PATHS
    ${CUDNN_ROOT_DIR}
    $ENV{CUDNN_ROOT_DIR}
    ${CUDA_TOOLKIT_ROOT_DIR}
    /usr
    /usr/local
    /usr/local/cuda
    /opt/cuda
    /usr/lib/cuda
    /usr/local/cuda-12.0
    /usr/local/cuda-12.1
    /usr/local/cuda-12.2
    # add more...
    /opt/cuda-12.0
    /opt/cuda-12.1
    /opt/cuda-12.2
    # add more...
)

find_path(CUDNN_INCLUDE_DIR
    NAMES cudnn.h cudnn_version.h
    PATHS ${_CUDNN_PATHS}
    PATH_SUFFIXES include include/cudnn cuda/include
)


if(CUDNN_INCLUDE_DIR)
    # new version -> cudnn_version.h
    if (EXISTS "${CUDNN_INCLUDE_DIR}/cudnn_version.h")
        file(READ "${CUDNN_INCLUDE_DIR}/cudnn_version.h" CUDNN_VERSION_CONTENT)
    
        string(REGEX MATCH "define CUDNN_MAJOR ([0-9]+)" _ "${CUDNN_VERSION_CONTENT}")
        set(CUDNN_VERSION_MAJOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "define CUDNN_MINOR ([0-9]+)" _ "${CUDNN_VERSION_CONTENT}")
        set(CUDNN_VERSION_MINOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "define CUDNN_PATCHLEVEL ([0-9]+)" _ "${CUDNN_VERSION_CONTENT}")
        set(CUDNN_VERSION_PATCH ${CMAKE_MATCH_1})
    elseif(EXISTS "${CUDNN_INCLUDE_DIR}/cudnn.h")
        file(READ "${CUDNN_INCLUDE_DIR}/cudnn.h" CUDNN_VERSION_CONTENT)
        
        string(REGEX MATCH "define CUDNN_MAJOR ([0-9]+)" _ "${CUDNN_VERSION_CONTENT}")
        set(CUDNN_VERSION_MAJOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "define CUDNN_MINOR ([0-9]+)" _ "${CUDNN_VERSION_CONTENT}")
        set(CUDNN_VERSION_MINOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "define CUDNN_PATCHLEVEL ([0-9]+)" _ "${CUDNN_VERSION_CONTENT}")
        set(CUDNN_VERSION_PATCH ${CMAKE_MATCH_1})
    endif()

    if(DEFINED CUDNN_VERSION_MAJOR AND DEFINED CUDNN_VERSION_MINOR AND DEFINED CUDNN_VERSION_PATCH)
        set(CUDNN_VERSION "${CUDNN_VERSION_MAJOR}.${CUDNN_VERSION_MINOR}.${CUDNN_VERSION_PATCH}")
    endif()

endif()

find_library(CUDNN_LIBRARY
    NAMES cudnn libcudnn
    PATHS ${_CUDNN_PATHS}
    PATH_SUFFIXES lib lib64 lib/x64 cuda/lib64 cuda/lib
)

find_library(CUDNN_STATIC_LIB
    NAMES libcudnn_static.a cudnn_static
    PATHS ${_CUDNN_PATHS}
    PATH_SUFFIXES lib lib64 lib/x64 cuda/lib64 cuda/lib
)

find_package_handle_standard_args(CUDNN
    REQUIRED_VARS
        CUDNN_INCLUDE_DIR
        CUDNN_LIBRARY
    VERSION_VAR
        CUDNN_VERSION
)

if(CUDNN_FOUND)
    set(CUDNN_INCLUDE_DIRS ${CUDNN_INCLUDE_DIR})
    set(CUDNN_LIBRARIES ${CUDNN_LIBRARY})
    if(CUDNN_STATIC_LIB)
        list(APPEND CUDNN_LIBRARIES ${CUDNN_STATIC_LIB})
    endif()
    
    message(STATUS "Found cuDNN: version ${CUDNN_VERSION}")
    message(STATUS "- Include path: ${CUDNN_INCLUDE_DIRS}")
    message(STATUS "- Library: ${CUDNN_LIBRARIES}")
endif()

mark_as_advanced(
    CUDNN_INCLUDE_DIR
    CUDNN_LIBRARY
    CUDNN_STATIC_LIB
)
