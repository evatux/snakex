file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]pp)

set(TARGET "snakex_tui")

add_executable(${TARGET} ${sources})
message("SNAKEX_CORE_LIB: ${SNAKEX_CORE_LIB}")
target_link_libraries(${TARGET} ${SNAKEX_CORE_LIB})
