file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]pp)

add_library(${SNAKEX_CORE_LIB} SHARED ${sources})
