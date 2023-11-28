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

