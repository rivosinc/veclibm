# SPDX-FileCopyrightText: 2023 Rivos Inc.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.22)
project(veclibm_test C CXX)

set(TEST_INFRA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_infra.cpp
)
set(TEST_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Build a library with the common test code in, to be used in every test
add_library(test_infra
    EXCLUDE_FROM_ALL
    ${TEST_INFRA_SOURCES}
)
target_include_directories(test_infra
    PRIVATE
    ${TEST_INCLUDES}
)
target_compile_options(test_infra
    PUBLIC
    $<$<COMPILE_LANGUAGE:C>:-std=c11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
)

# Adds a function, such that given the name of a test file will compile an
# executable, and add that to a target called 'test', which runs ctest on all
# defined test executables.
function(add_veclibm_test TEST_FILE)
    cmake_path(GET TEST_FILE STEM LAST_ONLY TEST_NAME)
    add_executable(${TEST_NAME}
        ${TEST_FILE}
    )
    target_include_directories(${TEST_NAME} PRIVATE ${TEST_INCLUDES})
    target_link_libraries(${TEST_NAME} 
        $<IF:$<NOT:$<STREQUAL:${BUILD_SHARED_LIBS},On>>,-static,>
        test_infra vecm
    )
    target_compile_options(${TEST_NAME}
        PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-std=c11>
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
    )
    add_test(
        NAME ${TEST_NAME}
        COMMAND ${VECLIBM_TEST_RUNNER} ${TEST_NAME}
    )
endfunction()

# Loop through the source test files, and add these to the 'test' target
set(TEST_SOURCES
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_acos.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_acosI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_acospi.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_acospiI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_asin.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_asinI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_asinpi.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_asinpiI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atan.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atanI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atanpi.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atanpiI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atan2.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atan2I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atan2pi.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atan2piI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_acosh.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_acoshI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_asinh.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_asinhI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atanh.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_atanhI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cbrt.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cbrtI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_exp.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_expI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_exp2.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_exp2I.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_exp10.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_exp10I.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_expm1.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_expm1I.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_log.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_logI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_log2.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_log2I.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_log10.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_log10I.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_log1p.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_log1pI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_pow.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_powI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cos.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cosI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sin.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sinI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cospi.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cospiI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sinpi.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sinpiI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sincos.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sincosI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sincospi.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sincospiI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_tan.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_tanI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_tanpi.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cosh.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_coshI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sinh.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sinhI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_tanh.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_tanhI.cpp
)

foreach(TEST_FILE IN LISTS TEST_SOURCES)
    add_veclibm_test(${TEST_FILE})
endforeach()
