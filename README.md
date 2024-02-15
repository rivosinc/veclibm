<!--
SPDX-FileCopyrightText: 2023 Rivos Inc.

SPDX-License-Identifier: Apache-2.0
-->

# veclibm

[![REUSE status](https://api.reuse.software/badge/github.com/rivosinc/veclibm)](https://api.reuse.software/info/github.com/rivosinc/veclibm)

A vector math library using RISC-V vector ISA via C intrinsic. 

WORK IN PROGRESS...

The basic structure is that each function, such as exp, is contained in one file.
The file includes a sequence of header files. Typically these three headers are present in this order
 1. rvvlm.h (Risc-V Vector Libm). Commonly used macros and function declaration (more detail below)
 2. rvvlm_fp64m1.h (or rvvlm_fp64m2.h, etc) which defines the LMUL choice for each function.
 3. function-specific header file, e.g. rvvlm_expD.h (for double precision exp function)

We sometimes provide multiple implementations for a single function as each may perform differently on different hardware implementations of the Risc-V vector ISA. Thus the function implementation use a macro for its name; and these macros are defined in the header file rvvlm.h. One can manually define one (and only one) of these macros to be the standard function name. Obviously, the "manual" process can be automated, allowing one to determine the highest performing choice experimentally.

A simple test program test_funcs.c uses the existing long double function as a reference. At this point we only aim to develop double-precision (and perhaps single-precision) functions, thus testing their accuracy with long-double function is deemed sufficient.

# Building & Testing

The veclibm project is built and tested on GitHub Actions. See `.github/workflows/build_and_test.yml` for reference.

## Cross-compiling

First, a sysroot is required with appropriate dependencies installed in it. For veclibm the following command will set up a sysroot:
```
sudo debootstrap --arch=riscv64 --verbose --include=fakeroot,symlinks,googletest --resolve-deps --variant=minbase --components=main,universe jammy sysroot
```

Then, assuming you've the necessary toolchains installed (refer to the [GHA workflow file](https://github.com/rivosinc/veclibm/blob/main/.github/workflows/build_and_test.yml) for versions and steps), veclibm is built with the following command:
```
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$(pwd)/install" -DCMAKE_TOOLCHAIN_FILE=$(pwd)/CMakeToolchain/riscv.clang.cross.cmake -DCMAKE_SYSROOT=$(pwd)/sysroot
```

Finally, veclibm is built with:
```
cmake --build build
```

This will build the `libvecm.so` library as well as the tests.

## Testing

_The following steps take the assumption the user is cross-compiling and using QEMU to execute the riscv64 binary on your host machine._

The easiest is to simply run `test/test_veclibm`:
```
QEMU_CPU=rv64,zba=true,zbb=true,zbs=true,v=true QEMU_LD_PREFIX=$(pwd)/sysroot build/test/test_veclibm
```

To run only a specific test, for example the `erf.test`, the user can filter with:
```
QEMU_CPU=rv64,zba=true,zbb=true,zbs=true,v=true QEMU_LD_PREFIX=$(pwd)/sysroot build/test/test_veclibm --gtest_filter=erf.test
```

Refer to the help for more options:
```
QEMU_CPU=rv64,zba=true,zbb=true,zbs=true,v=true QEMU_LD_PREFIX=$(pwd)/sysroot build/test/test_veclibm --help
```
