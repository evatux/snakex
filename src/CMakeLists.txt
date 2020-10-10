set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Include
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(cc_core)
add_subdirectory(cc_ncurses)
