cmake_minimum_required(VERSION 3.16)

file(GLOB SRC_FILES *.cpp)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/utils/*.h )


add_library(utils
    STATIC
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_include_directories(utils
    PUBLIC
    ${ONNXRUNTIME_INCLUDE_DIR}
)


target_link_libraries(utils
    PUBLIC
    onnxruntime::onnxruntime
    ${OpenCV_LIBS} 
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)