# SPDX-FileCopyrightText: 2023 Rivos Inc.
#
# SPDX-License-Identifier: Apache-2.0

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_expm1D.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_expm1DI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_logD_tbl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_logD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_logDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_log2D.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_log2DI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_log10D.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_log10DI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_log1pD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_log1pDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_powD_tbl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_powD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_powDI.c
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
