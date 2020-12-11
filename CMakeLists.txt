cmake_minimum_required (VERSION 3.1.0)

project(snakex CXX)
set(CMAKE_CXX_STANDARD 17)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    message(STATUS "CMAKE_BUILD_TYPE is unset, defaulting to Release")
    set(CMAKE_BUILD_TYPE "RelWithAssert" CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel RelWithAssert ...")
endif()

set(SNAKEX_CORE_LIB "snakexcore")

# sources
add_subdirectory(src)
