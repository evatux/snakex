cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Core WebSockets)

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]pp)

set(TARGET "cc_websockets_server_qt")

add_executable(${TARGET} ${sources})
target_link_libraries(${TARGET}
    Qt5::Core Qt5::WebSockets ${SNAKEX_CORE_LIB})
