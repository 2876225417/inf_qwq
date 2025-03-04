

cmake_minimum_required(VERSION 3.16)

set(3RDPARTY "${CMAKE_SOURCE_DIR}/3rdparty")
set(HKVISION_INCLUDE_DIR "${3RDPARTY}/hkvision/include")

if (NOT EXISTS "${HKVISION_INCLUDE_DIR}/HCNetSDK.h")
    message(FATAL_ERROR "Headers of HK_VISION is incomplete: ${HKVISION_INCLUDE_DIR}")
endif()

set(HKVISION_LIB_DIR "${3RDPARTY}/hkvision/lib")

file(GLOB_RECURSE LIBS_LIST "${HKVISION_LIB_DIR}/*.so")

set(HKVISION_LIBS)
foreach (LIB_PATH ${LIBS_LIST})
    get_filename_component(LIB_RAW ${LIB_PATH} NAME_WE)

    string(REGEX REPLACE "^lib" "" LIB_RAW ${LIB_RAW})

    add_library(hkvision::${LIB_RAW} SHARED IMPORTED)
    set_target_properties(hkvision::${LIB_RAW} PROPERTIES
        IMPORTED_LOCATION "${LIB_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${HKVISION_INCLUDE_DIR}"
    )
    list(APPEND HKVISION_LIBS hkvision::${LIB_RAW})
endforeach()

set(HKVISION_INCLUDE_DIR ${HKVISION_INCLUDE_DIR})
set(HKVISION_LIBS ${HKVISION_LIBS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(hkvision
    REQUIRED_VARS
        HKVISION_INCLUDE_DIR
        HKVISION_LIBS
)



