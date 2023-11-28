# SPDX-FileCopyrightText: 2023 Rivos Inc.
#
# SPDX-License-Identifier: Apache-2.0

if (RISCV_TOOLCHAIN_INCLUDED)
    return()
endif()
set(RISCV_TOOLCHAIN_INCLUDED)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET riscv64-unknown-linux-gnu)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET riscv64-unknown-linux-gnu)

set(RISCV_TOOL_BASE /opt/riscv)
set(SYSROOT_PATH ${RISCV_TOOL_BASE}/sysroot)

set(CMAKE_FIND_ROOT_PATH ${SYSROOT_PATH})
set(CMAKE_SYSROOT ${SYSROOT_PATH})

# Find includes and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# General options to enable cross-compilation
set(MARCH_OR_CPU -march=rv64gcv)
set(GCC_TOOLCHAIN --gcc-toolchain=${RISCV_TOOL_BASE})
set(ISYSTEM -isystem${RISCV_TOOL_BASE}/${CMAKE_C_COMPILER_TARGET}/include/c++/12.1.0/riscv64-unknown-linux-gnu -isystem${RISCV_TOOL_BASE}/${CMAKE_C_COMPILER_TARGET}/include/c++/12.1.0)

add_compile_options(
    ${MARCH_OR_CPU} ${GCC_TOOLCHAIN} ${ISYSTEM}
)

add_link_options(
    ${MARCH_OR_CPU}
    ${GCC_TOOLCHAIN}
)
