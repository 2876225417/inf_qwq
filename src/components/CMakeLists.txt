cmake_minimum_required(VERSION 3.16)


file(GLOB SRC_FILES *.cpp)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/components/*.h)

add_library(components
    STATIC
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_include_directories(components
    PUBLIC
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(components
    PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)
