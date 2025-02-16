cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


file(GLOB SRC_FILES *.cpp)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/windows/*.h)

add_library(windows
    STATIC
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_link_libraries(windows
    PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)