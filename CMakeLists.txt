# SPDX-FileCopyrightText: 2023 Rivos Inc.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.22)
project(veclibm C)

# Adds a test target
include(CTest)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_acosD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_acosDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_acospiD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_acospiDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_asinD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_asinDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_asinpiD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_asinpiDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atanD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atanDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atanpiD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atanpiDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atan2D.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atan2DI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atan2piD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atan2piDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_acoshD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_acoshDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_asinhD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_asinhDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atanhD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_atanhDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_cbrtD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_cbrtDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_cdfnormD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_cdfnormDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_erfD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_erfDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_erfcD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_erfcDI.c
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_2ovpi_tbl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_cosD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_cosDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_cospiD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_cospiDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sinD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sinDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sinpiD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sinpiDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sincosD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sincosDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sincospiD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sincospiDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_tanD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_tanDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_tanpiD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_tanpiDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_coshD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_coshDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sinhD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_sinhDI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_tanhD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rvvlm_tanhDI.c
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
