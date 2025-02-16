cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

  
add_subdirectory(windows)
add_subdirectory(utils)
add_subdirectory(components)
add_executable(inf_qwq
    main.cpp
)



# sub_module
target_link_libraries(inf_qwq
    PRIVATE
    windows
    utils
    components
)
