set(SOURCES
    fs.c
    boot.c
    table.c
    unit.c
    entry.c
    directory.c
    reference.c
    inverse.c
    long.c
    complex.c
    ucs2conv.c
)

if (APPLE)
    list(APPEND SOURCES linux_tsearch.c)
endif()

# concat header files into llfat.h
set(HEADERS
    llfat.h.header
    fs.h
    boot.h
    entry.h
    table.h
    directory.h
    reference.h
    inverse.h
    long.h
    complex.h
    debug.h
)
set(OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/llfat.h")

add_custom_command(
    OUTPUT ${OUTPUT_HEADER}
    COMMAND ${CMAKE_COMMAND} -E cat ${HEADERS} > ${OUTPUT_HEADER}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${HEADERS}
)

add_custom_target(
    llfat_headers
    DEPENDS ${OUTPUT_HEADER}
)

add_library(llfat STATIC ${SOURCES})
target_include_directories(llfat PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(llfat PRIVATE c_std_99)
target_link_libraries(llfat utf8proc)
add_dependencies(llfat llfat_headers)
