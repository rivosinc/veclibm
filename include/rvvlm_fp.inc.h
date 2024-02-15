// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __VECLIBM_RVVLM_FP_INC_H
#define __VECLIBM_RVVLM_FP_INC_H
#else
#warning "are you sure you want to include this file multiple times?"
#endif

#include <assert.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#define _NEED_UNDEF_GNU_SOURCE
#endif

#ifndef LMUL
static_assert(false, "Must assign an LMUL before including " __FILE__);
#endif
#ifndef BIT_WIDTH
static_assert(false, "Must assign BIT_WIDTH before including " __FILE__);
#endif
#ifndef API_SIGNATURE
static_assert(false, "Must assign API_SIGNATURE before including " __FILE__);
#endif
#ifndef STRIDE
static_assert(false, "Must assign STRIDE before including " __FILE__);
#endif

#include <math.h>
#ifndef NAN
_Static_assert(0, "NaN not available on this architecture")
#endif

#define __PASTE2_BASE(A, B) A##B
#define __PASTE2(A, B) __PASTE2_BASE(A, B)
#define __PASTE3_BASE(A, B, C) A##B##C
#define __PASTE3(A, B, C) __PASTE3_BASE(A, B, C)
#define __PASTE4_BASE(A, B, C, D) A##B##C##D
#define __PASTE4(A, B, C, D) __PASTE4_BASE(A, B, C, D)
#define __PASTE5_BASE(A, B, C, D, E) A##B##C##D##E
#define __PASTE5(A, B, C, D, E) __PASTE5_BASE(A, B, C, D, E)
#define __PASTE6_BASE(A, B, C, D, E, F) A##B##C##D##E##F
#define __PASTE6(A, B, C, D, E, F) __PASTE5_BASE(A, B, C, D, E, F)

#define MAKE_VTYPE(A) __PASTE3(A, BIT_WIDTH, __PASTE3(m, LMUL, _t))
#define MAKE_TYPE(A) __PASTE3(A, BIT_WIDTH, _t)
#define MAKE_FUNC(A) __PASTE3(A, BIT_WIDTH, __PASTE2(m, LMUL))
#define MAKE_VLOAD(A)                                                          \
  __PASTE3(__PASTE3(__riscv_vle, BIT_WIDTH, _v_), A,                           \
           __PASTE3(BIT_WIDTH, m, LMUL))
#define MAKE_VSLOAD(A)                                                         \
  __PASTE3(__PASTE3(__riscv_vlse, BIT_WIDTH, _v_), A,                          \
           __PASTE3(BIT_WIDTH, m, LMUL))

#if (BIT_WIDTH == 64)
#define NATIVE_TYPE double
#define TYPE_SIZE 8
#else
static_assert(false, "requested BIT_WIDTH unsupported" __FILE__);
#endif

#define API_11_US                                                              \
  size_t _inarg_n, const NATIVE_TYPE *_inarg1, NATIVE_TYPE *_outarg1
#define API_11_GS                                                              \
  size_t _inarg_n, const NATIVE_TYPE *_inarg1, size_t _inarg1_stride,          \
      NATIVE_TYPE *_outarg1, size_t _outarg1_stride
#define API_21_US                                                              \
  size_t _inarg_n, const NATIVE_TYPE *_inarg1, const NATIVE_TYPE *_inarg2,     \
      NATIVE_TYPE *_outarg1
#define API_21_GS                                                              \
  size_t _inarg_n, const NATIVE_TYPE *_inarg1, size_t _inarg1_stride,          \
      const NATIVE_TYPE *_inarg2, size_t _inarg2_stride,                       \
      NATIVE_TYPE *_outarg1, size_t _outarg1_stride
#define API_12_US                                                              \
  size_t _inarg_n, const NATIVE_TYPE *_inarg1, NATIVE_TYPE *_outarg1,          \
      NATIVE_TYPE *_outarg2
#define API_12_GS                                                              \
  size_t _inarg_n, const NATIVE_TYPE *_inarg1, size_t _inarg1_stride,          \
      NATIVE_TYPE *_outarg1, size_t _outarg1_stride, NATIVE_TYPE *_outarg2,    \
      size_t _outarg2_stride

#if (API_SIGNATURE == API_SIGNATURE_11)
#if (STRIDE == UNIT_STRIDE)
#define API API_11_US
#else
#define API API_11_GS
#endif
#elif (API_SIGNATURE == API_SIGNATURE_21)
#if (STRIDE == UNIT_STRIDE)
#define API API_21_US
#else
#define API API_21_GS
#endif
#elif (API_SIGNATURE == API_SIGNATURE_12)
#if (STRIDE == UNIT_STRIDE)
#define API API_12_US
#else
#define API API_12_GS
#endif
#else
static_assert(false, "API_SIGNATURE ill or undefined" __FILE__);
#endif

#if (STRIDE == UNIT_STRIDE)
#define VFLOAD_INARG1(vlen) MAKE_VLOAD(f)(_inarg1, (vlen))
#define VFLOAD_INARG2(vlen) MAKE_VLOAD(f)(_inarg2, (vlen))
#define VFSTORE_OUTARG1(vy, vlen)                                              \
  __PASTE2(__riscv_vse, BIT_WIDTH)(_outarg1, (vy), (vlen))
#define VFSTORE_OUTARG2(vy, vlen)                                              \
  __PASTE2(__riscv_vse, BIT_WIDTH)(_outarg2, (vy), (vlen))
#define INCREMENT_INARG1(vlen)                                                 \
  do {                                                                         \
    _inarg1 += (vlen);                                                         \
  } while (0)
#define INCREMENT_INARG2(vlen)                                                 \
  do {                                                                         \
    _inarg2 += (vlen);                                                         \
  } while (0)
#define INCREMENT_OUTARG1(vlen)                                                \
  do {                                                                         \
    _outarg1 += (vlen);                                                        \
  } while (0)
#define INCREMENT_OUTARG2(vlen)                                                \
  do {                                                                         \
    _outarg2 += (vlen);                                                        \
  } while (0)
#else
#define VFLOAD_INARG1(vlen)                                                    \
  MAKE_VSLOAD(f)(_inarg1, _inarg1_stride * TYPE_SIZE, (vlen))
#define VFLOAD_INARG2(vlen)                                                    \
  MAKE_VSLOAD(f)(_inarg2, _inarg2_stride * TYPE_SIZE, (vlen))
#define VFSTORE_OUTARG1(vy, vlen)                                              \
  __PASTE2(__riscv_vsse, BIT_WIDTH)                                            \
  (_outarg1, _outarg1_stride * TYPE_SIZE, (vy), (vlen))
#define VFSTORE_OUTARG2(vy, vlen)                                              \
  __PASTE2(__riscv_vsse, BIT_WIDTH)                                            \
  (_outarg2, _outarg2_stride * TYPE_SIZE, (vy), (vlen))
#define INCREMENT_INARG1(vlen)                                                 \
  do {                                                                         \
    _inarg1 += _inarg1_stride * (vlen);                                        \
  } while (0)
#define INCREMENT_INARG2(vlen)                                                 \
  do {                                                                         \
    _inarg2 += _inarg2_stride * (vlen);                                        \
  } while (0)
#define INCREMENT_OUTARG1(vlen)                                                \
  do {                                                                         \
    _outarg1 += _outarg1_stride * (vlen);                                      \
  } while (0)
#define INCREMENT_OUTARG2(vlen)                                                \
  do {                                                                         \
    _outarg2 += _outarg2_stride * (vlen);                                      \
  } while (0)
#endif

// For MAKE_VBOOL, the value is 64/LMUL
#if (LMUL == 1)
#define MAKE_VBOOL(A) __PASTE3(A, 64, _t)
#elif (LMUL == 2)
#define MAKE_VBOOL(A) __PASTE3(A, 32, _t)
#elif (LMUL == 4)
#define MAKE_VBOOL(A) __PASTE3(A, 16, _t)
#elif (LMUL == 8)
#define MAKE_VBOOL(A) __PASTE3(A, 8, _t)
#endif
#define VSET __PASTE2(__riscv_vsetvl_e, __PASTE3(BIT_WIDTH, m, LMUL))
#define VSE __PASTE2(__riscv_vse, BIT_WIDTH)
#define VSSE __PASTE2(__riscv_vsse, BIT_WIDTH)
#define MAKE_REINTERPRET(A, B)                                                 \
  __PASTE5(__riscv_vreinterpret_v_, A, __PASTE4(BIT_WIDTH, m, LMUL, _), B,     \
           __PASTE3(BIT_WIDTH, m, LMUL))

#define FLOAT MAKE_TYPE(float)
#define VFLOAT MAKE_VTYPE(vfloat)
#define INT MAKE_TYPE(int)
#define VINT MAKE_VTYPE(vint)
#define UINT MAKE_TYPE(uint)
#define VUINT MAKE_VTYPE(vuint)
#define VBOOL MAKE_VBOOL(vbool)

#define F_AS_I MAKE_REINTERPRET(f, i)
#define F_AS_U MAKE_REINTERPRET(f, u)
#define I_AS_F MAKE_REINTERPRET(i, f)
#define U_AS_F MAKE_REINTERPRET(u, f)
#define I_AS_U MAKE_REINTERPRET(i, u)
#define U_AS_I MAKE_REINTERPRET(u, i)

#define VFLOAD MAKE_VLOAD(f)
#define VILOAD MAKE_VLOAD(i)
#define VULOAD MAKE_VLOAD(u)
#define VFSLOAD MAKE_VSLOAD(f)
#define VMVI_VX MAKE_FUNC(__riscv_vmv_v_x_i)
#define VMVU_VX MAKE_FUNC(__riscv_vmv_v_x_u)
#define VFMV_VF MAKE_FUNC(__riscv_vfmv_v_f_f)

    static const INT int_Zero = 0;
static const UINT uint_Zero = 0;

#if (BIT_WIDTH == 64)
#define EXP_BIAS 1023
#define MAN_LEN 52
static const uint64_t class_sNaN = 0x100;
static const uint64_t class_qNaN = 0x200;
static const uint64_t class_NaN = 0x300;
static const uint64_t class_negInf = 0x1;
static const uint64_t class_posInf = 0x80;
static const uint64_t class_Inf = 0x81;
static const uint64_t class_negZero = 0x8;
static const uint64_t class_posZero = 0x10;
static const uint64_t class_Zero = 0x18;
static const uint64_t class_negDenorm = 0x4;
static const uint64_t class_posDenorm = 0x20;
static const uint64_t class_Denorm = 0x24;
static const uint64_t class_negNormal = 0x2;
static const uint64_t class_posNormal = 0x40;
static const uint64_t class_Normal = 0x42;
static const uint64_t class_negative = 0x7;
static const uint64_t class_positive = 0xe0;
static const uint64_t class_finite_neg = 0x06;
static const uint64_t class_finite_pos = 0x60;

static const double fp_sNaN = __builtin_nans("");
static const double fp_qNaN = __builtin_nan("");
static const double fp_posInf = __builtin_inf();
static const double fp_negInf = -__builtin_inf();
static const double fp_negZero = -0.;

static const union sui64_fp64 SNAN = {fp_sNaN};
static const union sui64_fp64 QNAN = {fp_qNaN};
static const union sui64_fp64 PINF = {fp_posInf};
static const union sui64_fp64 NINF = {fp_negInf};
static const union sui64_fp64 NEGZERO = {fp_negZero};

static const double fp_posZero = 0.0;
static const double fp_posOne = 0x1.0p0;
static const double fp_negOne = -0x1.0p0;
static const double fp_posHalf = 0x1.0p-1;
static const double fp_negHalf = -0x1.0p-1;
#endif

#ifdef _NEED_UNDEF_GNU_SOURCE
#undef _GNU_SOURCE
#undef _NEED_UNDEF_GNU_SOURCE
#endif
