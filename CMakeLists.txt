cmake_minimum_required(VERSION 3.22)
project(veclibm C)

# Adds a test target
include(CTest)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_expD_tbl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_expD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_expDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_exp2D.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_exp2DI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_exp10D.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_exp10DI.c
)

add_library(vecm
    ${PROJECT_SOURCES}
)

target_include_directories(vecm
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(vecm
    PUBLIC
    $<$<COMPILE_LANGUAGE:C>:-std=c11>
)

set(CMAKE_BUILD_TYPE "Release"
    CACHE STRING
    "The type of build to perform. Choose from [Release, Debug, RelWithDebInfo]"
)

# Add tests as well. Split this out into its own file. This means that the tests
# are in a 'test' subdirectory of the current build directory
add_subdirectory(test)
