#
#   Copyright 2023  Rivos Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
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
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_exp.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_expI.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_exp2.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_exp2I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_expm1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_expm1I.cpp
)

foreach(TEST_FILE IN LISTS TEST_SOURCES)
    add_veclibm_test(${TEST_FILE})
endforeach()