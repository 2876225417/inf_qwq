

cmake_minimum_required(VERSION 3.16)

set(3RDPARTY "${CMAKE_SOURCE_DIR}/3rdparty")

set(HKVISON_INCLUDE_DIR "${3RDPARTY}/hk_vision/include")
if (NOT_EXISTS "${HKVISION_INCLUDE_DIR}/HCNetSDK.h")
    message(FATAL_ERROR "Headers of HK_VISION is incomplete: ${HKVISION_INCLUDE_DIR}")
endif()

set(HKVISION_LIB "${3RDPARTY}/hk_vision/lib")
find_library(HKVISION_LIBRARY
    NAMES hk_vision
    PATH "${HKVISION_LIB}"
    NO_DEFAULT_PATH
    REQUIRED
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(hk_vision
    REQUIRED_ARGS
        HKVISION_INCLUDE_DIR
        HKVISION_LIBRARY
)

if (NOT TRAGET)

endif()
