set(SOURCES
    tsearch.c
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
)

add_library(llfat STATIC ${SOURCES})
target_include_directories(llfat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(llfat PRIVATE c_std_99)
