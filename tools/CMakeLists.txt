set(PROGS
    fattest
    fatexample
    fatview
    fatshrink
    fattool
    fatbackup
)

foreach(PROG ${PROGS})
    add_executable(${PROG} "${PROG}.c")
    target_link_libraries(${PROG} llfat)
    target_include_directories(${PROG} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${PROG} PRIVATE -Wall -Wextra -D_FORTIFY_SOURCE=2)
    target_compile_features(${PROG} PRIVATE c_std_99)
endforeach()
