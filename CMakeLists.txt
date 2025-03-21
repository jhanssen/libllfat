cmake_minimum_required(VERSION 3.18)
project(llfat)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

option(LLFAT_TOOLS "Build the llfat tools" On)

add_subdirectory(3rdparty)
add_subdirectory(lib)

if(LLFAT_TOOLS)
    add_subdirectory(tools)
endif()
