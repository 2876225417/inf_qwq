cmake_minimum_required(VERSION 3.16)


file(GLOB SRC_FILES *.cpp)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/windows/*.h)

 

#message(STATUS "HEADERS_FILES: ${SRC_FILES}")
  



add_library(windows
    STATIC
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_include_directories(windows
    PUBLIC
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(windows
    PUBLIC
    components
)

#3rdparty
target_link_libraries(windows
    PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)
