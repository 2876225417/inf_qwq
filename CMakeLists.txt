cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(inf_qwq)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${ONNXRUNTIME_INCLUDE_DIR}
)

find_package(onnxruntime REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)


 