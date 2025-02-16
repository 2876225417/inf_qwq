cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

  
add_subdirectory(windows)
add_subdirectory(utils)

add_executable(inf_qwq
    main.cpp
)

target_include_directories(inf_qwq
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# sub_module
target_link_libraries(inf_qwq
    PRIVATE
    windows
    utils
)
