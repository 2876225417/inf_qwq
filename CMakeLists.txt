cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(inf_qwq LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
    -Wno-unused-parameter
    -Wno-sign-compare
    -mno-direct-extern-access
)





message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Requred CXX standard: ${CMAKE_CXX_STANDARD}")

option(ENABLE_EIGEN "Eigen support for accelarating mat calculating" OFF)
message(STATUS "ENABLE_EIGEN: ${ENABLE_EIGEN}")

if(ENABLE_EIGEN)
    find_package(Eigen3 REQUIRED)
    add_definitions(-DENABLE_EIGEN=1)
else()
    add_definitions(-DENABLE_EIGEN=0)
endif()

#find_package(OpenMP REQUIRED)
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



if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
    add_compile_options(
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CMAKE_CXX_COMPILER_ID:Clang>>:-O3>
        $<$<CXX_COMPILER_ID:MSVC>:/O2>
        -march=native
        -flto=auto
    )
    add_link_options($<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-flto=auto>)
else()
    add_compile_options(-O0 -g3)
endif()



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


install(TARGETS inf_qwq
    RUNTIME DESTINATION bin
    BUNDLE  DESTINATION bin
)


install(TARGETS utils windows components
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)



