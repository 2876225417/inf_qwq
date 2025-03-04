cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(inf_qwq)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_compile_options(-mno-direct-extern-access)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Requred CXX standard: ${CMAKE_CXX_STANDARD}")

find_package(hkvision REQUIRED)
find_package(onnxruntime REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
    MultimediaWidgets
    OpenGLWidgets
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${ONNXRUNTIME_INCLUDE_DIR}
    ${HKVISION_INCLUDE_DIR}
) 

message(STATUS "onnxruntime include dir: ${ONNXRUNTIME_INCLUDE_DIR}")
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

# hk test
# message(STATUS "hkvision include dir: ${HKVISION_INCLUDE_DIR}")
# add_executable(hk hk_test_demo.cpp)
# target_link_libraries(hk
#     PRIVATE
#     hkvision::hcnetsdk
#     hkvision::AudioRender
#     hkvision::HCCore
# )
 
