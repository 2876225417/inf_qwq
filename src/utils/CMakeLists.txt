cmake_minimum_required(VERSION 3.16)

file(GLOB SRC_FILES *.cpp)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/utils/*.h )


add_library(utils
    STATIC
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_link_libraries(utils
    PUBLIC
    onnxruntime::onnxruntime
    ${OpenCV_LIBS}
)