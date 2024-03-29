# SPDX-FileCopyrightText: 2023 Rivos Inc.
#
# SPDX-License-Identifier: Apache-2.0

SET (CMAKE_CROSSCOMPILING   TRUE)
SET (CMAKE_SYSTEM_NAME      "Linux")
SET (CMAKE_SYSTEM_PROCESSOR "riscv64")

SET(CMAKE_FIND_ROOT_PATH  /usr/riscv64-linux-gnu /usr/include/riscv64-linux-gnu /usr/lib/riscv64-linux-gnu /lib/riscv64-linux-gnu)

find_program(CMAKE_C_COMPILER NAMES clang-18 clang)
set(CMAKE_C_COMPILER_TARGET riscv64-linux-gnu)
set(CMAKE_C_FLAGS "-march=rv64gcv_zba_zbb_zbs")

find_program(CMAKE_CXX_COMPILER NAMES clang++-18 clang++)
set(CMAKE_CXX_COMPILER_TARGET riscv64-linux-gnu)
set(CMAKE_CXX_FLAGS "-march=rv64gcv_zba_zbb_zbs")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
