cmake_minimum_required(VERSION 3.16)

file(GLOB SRC_FILES *.cpp)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/utils/*.h)
file(GLOB TEMPLATE_FILES ${CMAKE_SOURCE_DIR}/include/utils/*.hpp)


add_library(utils
    STATIC
    ${SRC_FILES}
    ${HEADER_FILES}
    ${TEMPLATE_FILES}
)

target_include_directories(utils
    PUBLIC
    ${ONNXRUNTIME_INCLUDE_DIR}
)

message(STATUS "ONNXRUNTIME_INCLUDE_DIR: ${ONNXRUNTIME_INCLUDE_DIR}")

target_link_libraries(utils
    PUBLIC
    onnxruntime::onnxruntime
    ${OpenCV_LIBS} 
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(utils PRIVATE OpenMP::OpenMP_CXX)
    target_compile_options(utils PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fopenmp>
        $<$<CXX_COMPILER_ID:MSVC>:/openmp>
    )
endif()
