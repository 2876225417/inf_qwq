cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

  
find_package(onnxruntime REQUIRED)


add_executable(inf_qwq
    main.cpp 
    utils/ort_inf.cpp
)

target_include_directories(inf_qwq
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${ONNXRUNTIME_INCLUDE_DIR}
)

 
target_link_libraries(inf_qwq  
    PUBLIC
    onnxruntime::onnxruntime
)  