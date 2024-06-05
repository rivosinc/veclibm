# SPDX-FileCopyrightText: 2023 Rivos Inc.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.22)
project(veclibm C CXX)

option(VECLIBM_BUILD_TESTS "Build tests" ON)

if (VECLIBM_BUILD_TESTS)
    # Adds a test target
    include(CTest)

    include(GoogleTest)

    set(GTest_ROOT ${CMAKE_SYSROOT}/usr/src/googletest/googletest)
    add_subdirectory(${GTest_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/googletest" EXCLUDE_FROM_ALL)
endif()

set(PROJECT_SOURCES
    src/rvvlm_acosD.c
    src/rvvlm_acosDI.c
    src/rvvlm_acospiD.c
    src/rvvlm_acospiDI.c
    src/rvvlm_asinD.c
    src/rvvlm_asinDI.c
    src/rvvlm_asinpiD.c
    src/rvvlm_asinpiDI.c
    src/rvvlm_atanD.c
    src/rvvlm_atanDI.c
    src/rvvlm_atanpiD.c
    src/rvvlm_atanpiDI.c
    src/rvvlm_atan2D.c
    src/rvvlm_atan2DI.c
    src/rvvlm_atan2piD.c
    src/rvvlm_atan2piDI.c
    src/rvvlm_acoshD.c
    src/rvvlm_acoshDI.c
    src/rvvlm_asinhD.c
    src/rvvlm_asinhDI.c
    src/rvvlm_atanhD.c
    src/rvvlm_atanhDI.c
    src/rvvlm_cbrtD.c
    src/rvvlm_cbrtDI.c
    src/rvvlm_cdfnormD.c
    src/rvvlm_cdfnormDI.c
    src/rvvlm_cdfnorminvD.c
    src/rvvlm_cdfnorminvDI.c
    src/rvvlm_erfD.c
    src/rvvlm_erfDI.c
    src/rvvlm_erfcD.c
    src/rvvlm_erfcDI.c
    src/rvvlm_erfcinvD.c
    src/rvvlm_erfcinvDI.c
    src/rvvlm_erfinvD.c
    src/rvvlm_erfinvDI.c
    src/rvvlm_expD_tbl.c
    src/rvvlm_expD.c
    src/rvvlm_expDI.c
    src/rvvlm_exp2D.c
    src/rvvlm_exp2DI.c
    src/rvvlm_exp10D.c
    src/rvvlm_exp10DI.c
    src/rvvlm_expm1D.c
    src/rvvlm_expm1DI.c
    src/rvvlm_expint1D.c
    src/rvvlm_expint1DI.c
    src/rvvlm_logD_tbl.c
    src/rvvlm_logD.c
    src/rvvlm_logDI.c
    src/rvvlm_log2D.c
    src/rvvlm_log2DI.c
    src/rvvlm_log10D.c
    src/rvvlm_log10DI.c
    src/rvvlm_log1pD.c
    src/rvvlm_log1pDI.c
    src/rvvlm_powD_tbl.c
    src/rvvlm_powD.c
    src/rvvlm_powDI.c
    src/rvvlm_2ovpi_tbl.c
    src/rvvlm_cosD.c
    src/rvvlm_cosDI.c
    src/rvvlm_cospiD.c
    src/rvvlm_cospiDI.c
    src/rvvlm_sinD.c
    src/rvvlm_sinDI.c
    src/rvvlm_sinpiD.c
    src/rvvlm_sinpiDI.c
    src/rvvlm_sincosD.c
    src/rvvlm_sincosDI.c
    src/rvvlm_sincospiD.c
    src/rvvlm_sincospiDI.c
    src/rvvlm_tanD.c
    src/rvvlm_tanDI.c
    src/rvvlm_tanpiD.c
    src/rvvlm_tanpiDI.c
    src/rvvlm_coshD.c
    src/rvvlm_coshDI.c
    src/rvvlm_sinhD.c
    src/rvvlm_sinhDI.c
    src/rvvlm_tanhD.c
    src/rvvlm_tanhDI.c
    src/rvvlm_lgammaD.c
    src/rvvlm_lgammaDI.c
    src/rvvlm_tgammaD.c
    src/rvvlm_tgammaDI.c
)

add_library(vecm
    ${PROJECT_SOURCES}
)

target_include_directories(vecm
    PUBLIC
    include
)

target_compile_options(vecm
    PUBLIC
    $<$<COMPILE_LANGUAGE:C>:-std=c11>
)

if (VECLIBM_BUILD_TESTS)
    # Add tests as well. Split this out into its own file. This means that the tests
    # are in a 'test' subdirectory of the current build directory
    add_subdirectory(test)
endif()
