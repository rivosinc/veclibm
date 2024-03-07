// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

union sui32_fp32 {
  int32_t si;
  uint32_t ui;
  float f;
};
union sui64_fp64 {
  int64_t si;
  uint64_t ui;
  double f;
  uint32_t ui_hilo[2];
};

#define ui_hilo_HI 1
#define ui_hilo_LO 0
// so that union sui64_f64 X will have X.hilo[HI] as the high bits (containing
// expoent) and X.hilo[LO] has the lower order bits (containing the lsb for
// example)

#define API_SIGNAUTRE_11 1
#define API_SIGNATURE_21 2
#define API_SIGNATURE_12 3
#define API_SIGNATURE_22 4

#define UNIT_STRIDE 1
#define GENERAL_STRIDE 2

#ifndef FE_TONEAREST
#define FE_TONEAREST 0x000
#endif

#define read_frm()                                                             \
  ({                                                                           \
    unsigned long __value;                                                     \
    __asm__ __volatile__("frrm %0" : "=r"(__value)::"memory");                 \
    __value;                                                                   \
  })

#define write_frm(value)                                                       \
  ({                                                                           \
    unsigned long __value;                                                     \
    __asm__ __volatile__("fsrm %0, %1"                                         \
                         : "=r"(__value)                                       \
                         : "r"(value)                                          \
                         : "memory");                                          \
    __value;                                                                   \
  })

#define SET_ROUNDTONEAREST                                                     \
  int __original_frm = read_frm();                                             \
  if (__original_frm != FE_TONEAREST) {                                        \
    write_frm(FE_TONEAREST);                                                   \
  }

#define RESTORE_FRM                                                            \
  do {                                                                         \
    if (__original_frm != FE_TONEAREST) {                                      \
      write_frm(__original_frm);                                               \
    }                                                                          \
  } while (0)

#define PSTEP(coeff_j, x, poly, vlen)                                          \
  __riscv_vfmadd((poly), (x), VFMV_VF((coeff_j), (vlen)), (vlen))

#define PSTEP_ab(pick_a, coeff_a, coeff_b, x, poly, vlen)                      \
  __riscv_vfmadd((poly), (x),                                                  \
                 __riscv_vfmerge(VFMV_VF((coeff_b), (vlen)), (coeff_a),        \
                                 (pick_a), (vlen)),                            \
                 (vlen))

#define PSTEP_I(COEFF_j, X, POLY, vlen)                                        \
  __riscv_vsadd(__riscv_vsmul((POLY), (X), 1, (vlen)), (COEFF_j), (vlen))

#define PSTEP_I_SLL(COEFF_j, X, K, POLY, vlen)                                 \
  __riscv_vsadd(                                                               \
      __riscv_vsll(__riscv_vsmul((POLY), (X), 1, (vlen)), (K), (vlen)),        \
      (COEFF_j), (vlen))

#define PSTEP_I_SRA(COEFF_j, X, K, POLY, vlen)                                 \
  __riscv_vsadd(                                                               \
      __riscv_vsra(__riscv_vsmul((POLY), (X), 1, (vlen)), (K), (vlen)),        \
      (COEFF_j), (vlen))

#define PSTEPN_I(COEFF_j, X, POLY, vlen)                                       \
  __riscv_vrsub(__riscv_vsmul((POLY), (X), 1, (vlen)), (COEFF_j), (vlen))

#define PSTEP_I_ab(pick_a, COEFF_a, COEFF_b, X, POLY, vlen)                    \
  __riscv_vsadd(                                                               \
      __riscv_vsmul((POLY), (X), 1, (vlen)),                                   \
      __riscv_vmerge(VMVI_VX((COEFF_b), (vlen)), (COEFF_a), (pick_a), (vlen)), \
      (vlen))

#define FAST2SUM(X, Y, S, s, vlen)                                             \
  do {                                                                         \
    (S) = __riscv_vfadd((X), (Y), (vlen));                                     \
    (s) = __riscv_vfsub((X), (S), (vlen));                                     \
    (s) = __riscv_vfadd((s), (Y), (vlen));                                     \
  } while (0)

#define POS2SUM(X, Y, S, s, vlen)                                              \
  do {                                                                         \
    VFLOAT _first = __riscv_vfmax((X), (Y), (vlen));                           \
    VFLOAT _second = __riscv_vfmin((X), (Y), (vlen));                          \
    S = __riscv_vfadd((X), (Y), (vlen));                                       \
    s = __riscv_vfadd(__riscv_vfsub(_first, (S), (vlen)), _second, (vlen));    \
  } while (0)

#define KNUTH2SUM(X, Y, S, s, vlen)                                            \
  do {                                                                         \
    S = __riscv_vfadd((X), (Y), (vlen));                                       \
    VFLOAT X_hat = __riscv_vfsub((S), (Y), (vlen));                            \
    s = __riscv_vfadd(                                                         \
        __riscv_vfsub((X), X_hat, (vlen)),                                     \
        __riscv_vfsub((Y), __riscv_vfsub((S), X_hat, (vlen)), (vlen)),         \
        (vlen));                                                               \
  } while (0)

#define PROD_X1Y1(x, y, prod_hi, prod_lo, vlen)                                \
  do {                                                                         \
    (prod_hi) = __riscv_vfmul((x), (y), (vlen));                               \
    (prod_lo) = __riscv_vfmsub((x), (y), (prod_hi), (vlen));                   \
  } while (0)

#define DIV_N1D2(numer, denom, delta_d, Q, q, vlen)                            \
  do {                                                                         \
    Q = __riscv_vfdiv((numer), (denom), (vlen));                               \
    q = __riscv_vfnmsub((Q), (denom), (numer), (vlen));                        \
    q = __riscv_vfnmsac((q), (Q), (delta_d), (vlen));                          \
    q = __riscv_vfmul(q, __riscv_vfrec7((denom), (vlen)), (vlen));             \
  } while (0)

#define DIV_N2D2(numer, delta_n, denom, delta_d, Q, vlen)                      \
  do {                                                                         \
    VFLOAT _q;                                                                 \
    (Q) = __riscv_vfdiv((numer), (denom), (vlen));                             \
    _q = __riscv_vfnmsub((Q), (denom), (numer), (vlen));                       \
    _q = __riscv_vfnmsac(_q, (Q), (delta_d), (vlen));                          \
    _q = __riscv_vfadd(_q, (delta_n), (vlen));                                 \
    _q = __riscv_vfmul(_q, __riscv_vfrec7((denom), (vlen)), (vlen));           \
    (Q) = __riscv_vfadd((Q), _q, (vlen));                                      \
  } while (0)

#define DIV2_N2D2(numer, delta_n, denom, delta_d, Q, delta_Q, vlen)            \
  do {                                                                         \
    VFLOAT _q;                                                                 \
    (Q) = __riscv_vfdiv((numer), (denom), (vlen));                             \
    _q = __riscv_vfnmsub((Q), (denom), (numer), (vlen));                       \
    _q = __riscv_vfnmsac(_q, (Q), (delta_d), (vlen));                          \
    _q = __riscv_vfadd(_q, (delta_n), (vlen));                                 \
    (delta_Q) = __riscv_vfmul(_q, __riscv_vfrec7((denom), (vlen)), (vlen));    \
  } while (0)

#define SQRT2_X2(x, delta_x, r, delta_r, vlen)                                 \
  do {                                                                         \
    VFLOAT xx = __riscv_vfadd((x), (delta_x), (vlen));                         \
    VBOOL x_eq_0 = __riscv_vmfeq(xx, fp_posZero, (vlen));                      \
    xx = __riscv_vfmerge(xx, fp_posOne, x_eq_0, (vlen));                       \
    (r) = __riscv_vfsqrt(xx, (vlen));                                          \
    (delta_r) = __riscv_vfnmsub((r), (r), (x), (vlen));                        \
    (delta_r) = __riscv_vfadd((delta_r), (delta_x), (vlen));                   \
    (delta_r) = __riscv_vfmul((delta_r), __riscv_vfrec7(xx, (vlen)), (vlen));  \
    /* (delta_r) = __riscv_vfdiv((delta_r), xx, (vlen)); */                    \
    (delta_r) = __riscv_vfmul((delta_r), 0x1.0p-1, (vlen));                    \
    (delta_r) = __riscv_vfmul((delta_r), (r), (vlen));                         \
    (r) = __riscv_vfmerge((r), fp_posZero, x_eq_0, (vlen));                    \
    (delta_r) = __riscv_vfmerge((delta_r), fp_posZero, x_eq_0, (vlen));        \
  } while (0)

#define IDENTIFY(vclass, stencil, identity_mask, vlen)                         \
  identity_mask =                                                              \
      __riscv_vmsgtu(__riscv_vand((vclass), (stencil), (vlen)), 0, (vlen))

#define FCLIP(vx, x_min, x_max, vlen)                                          \
  __riscv_vfmin(__riscv_vfmax((vx), X_MIN, (vlen)), X_MAX, (vlen))

#define FAST_LDEXP(num, exp, vlen)                                             \
  do {                                                                         \
    VINT n1 = __riscv_vsra((exp), 1, (vlen));                                  \
    VINT n2 = __riscv_vsub((exp), n1, (vlen));                                 \
    n1 = __riscv_vsll(n1, MAN_LEN, (vlen));                                    \
    num = I_AS_F(__riscv_vadd(F_AS_I((num)), n1, (vlen)));                     \
    n2 = __riscv_vadd(n2, EXP_BIAS, (vlen));                                   \
    n2 = __riscv_vsll(n2, MAN_LEN, (vlen));                                    \
    num = __riscv_vfmul((num), I_AS_F(n2), (vlen));                            \
  } while (0)

// Some of the functions have multiple implementations using different
// algorithms or styles. The following configure the name of each of these
// variations, thus allowing one to be set to the standard libm name.

// FP64 acos function configuration
#define RVVLM_ACOSD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ACOSD_FIXEDPT rvvlm_acos

#define RVVLM_ACOSDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ACOSDI_FIXEDPT rvvlm_acosI

#define RVVLM_ACOSPID_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ACOSPID_FIXEDPT rvvlm_acospi

#define RVVLM_ACOSPIDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ACOSPIDI_FIXEDPT rvvlm_acospiI

// FP64 asin function configuration
#define RVVLM_ASIND_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ASIND_FIXEDPT rvvlm_asin

#define RVVLM_ASINDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ASINDI_FIXEDPT rvvlm_asinI

#define RVVLM_ASINPID_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ASINPID_FIXEDPT rvvlm_asinpi

#define RVVLM_ASINPIDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ASINPIDI_FIXEDPT rvvlm_asinpiI

// FP64 atan function configuration
#define RVVLM_ATAND_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATAND_FIXEDPT rvvlm_atan

#define RVVLM_ATANDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATANDI_FIXEDPT rvvlm_atanI

#define RVVLM_ATANPID_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATANPID_FIXEDPT rvvlm_atanpi

#define RVVLM_ATANPIDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATANPIDI_FIXEDPT rvvlm_atanpiI

// FP64 atan2 function configuration
#define RVVLM_ATAN2D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATAN2D_FIXEDPT rvvlm_atan2

#define RVVLM_ATAN2DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATAN2DI_FIXEDPT rvvlm_atan2I

#define RVVLM_ATAN2PID_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATAN2PID_FIXEDPT rvvlm_atan2pi

#define RVVLM_ATAN2PIDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATAN2PIDI_FIXEDPT rvvlm_atan2piI

// FP64 acosh function configuration
#define RVVLM_ACOSHD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ACOSHD_STD rvvlm_acosh

#define RVVLM_ACOSHDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ACOSHDI_STD rvvlm_acoshI

// FP64 asinh function configuration
#define RVVLM_ASINHD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ASINHD_STD rvvlm_asinh

#define RVVLM_ASINHDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ASINHDI_STD rvvlm_asinhI

// FP64 atanh function configuration
#define RVVLM_ATANHD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATANHD_MIXED rvvlm_atanh

#define RVVLM_ATANHDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ATANHDI_MIXED rvvlm_atanhI

// FP64 cbrt function configuration
#define RVVLM_CBRTD_VSET_CONFIG "rvvlm_fp64m4.h"
#define RVVLM_CBRTD_ITER rvvlm_cbrt

#define RVVLM_CBRTDI_VSET_CONFIG "rvvlm_fp64m4.h"
#define RVVLM_CBRTDI_ITER rvvlm_cbrtI

// FP64 cdfnorm function configuration
#define RVVLM_CDFNORMD_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_CDFNORMD_STD rvvlm_cdfnorm

#define RVVLM_CDFNORMDI_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_CDFNORMDI_STD rvvlm_cdfnormI

// FP64 erf function configuration
#define RVVLM_ERFD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ERFD_STD rvvlm_erf

#define RVVLM_ERFDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_ERFDI_STD rvvlm_erfI

// FP64 erfc function configuration
#define RVVLM_ERFCD_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_ERFCD_STD rvvlm_erfc

#define RVVLM_ERFCDI_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_ERFCDI_STD rvvlm_erfcI

// FP64 erfinv function configuration
#define RVVLM_ERFINVD_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_ERFINVD_STD rvvlm_erfinv

#define RVVLM_ERFINVDI_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_ERFINVDI_STD rvvlm_erfinvI

// FP64 erfcinv function configuration
#define RVVLM_ERFCINVD_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_ERFCINVD_STD rvvlm_erfcinv

#define RVVLM_ERFCINVDI_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_ERFCINVDI_STD rvvlm_erfcinvI

// FP64 exp function configuration
#define RVVLM_EXPD_VSET_CONFIG "rvvlm_fp64m4.h"
#define RVVLM_EXPD_STD rvvlm_expD_std
#define RVVLM_EXPD_STD_R_EXTRA rvvlm_expD_std_r_extra
#define RVVLM_EXPD_STD_EPSIM rvvlm_exp
#define RVVLM_EXPD_TBL64 rvvlm_expD_tbl64

#define RVVLM_EXPDI_VSET_CONFIG "rvvlm_fp64m4.h"
#define RVVLM_EXPDI_STD rvvlm_expID_std
#define RVVLM_EXPDI_STD_R_EXTRA rvvlm_expI
#define RVVLM_EXPDI_STD_EPSIM rvvlm_expDI_std_epsim
#define RVVLM_EXPDI_TBL64 rvvlm_expDI_tbl64

// FP64 exp2 function configuration
#define RVVLM_EXP2D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_EXP2D_STD rvvlm_exp2D_std
#define RVVLM_EXP2D_STD_R_EXTRA rvvlm_exp2D_std_r_extra
#define RVVLM_EXP2D_STD_EPSIM rvvlm_exp2
#define RVVLM_EXP2D_TBL64 rvvlm_exp2D_tbl64

#define RVVLM_EXP2DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_EXP2DI_STD rvvlm_exp2ID_std
#define RVVLM_EXP2DI_STD_R_EXTRA rvvlm_exp2I
#define RVVLM_EXP2DI_STD_EPSIM rvvlm_exp2DI_std_epsim
#define RVVLM_EXP2DI_TBL64 rvvlm_exp2DI_tbl64

// FP64 exp10 function configuration
#define RVVLM_EXP10D_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_EXP10D_STD rvvlm_exp10D_std
#define RVVLM_EXP10D_STD_R_EXTRA rvvlm_exp10D_std_r_extra
#define RVVLM_EXP10D_STD_EPSIM rvvlm_exp10
#define RVVLM_EXP10D_TBL64 rvvlm_exp10D_tbl64

#define RVVLM_EXP10DI_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_EXP10DI_STD rvvlm_exp10ID_std
#define RVVLM_EXP10DI_STD_R_EXTRA rvvlm_exp10I
#define RVVLM_EXP10DI_STD_EPSIM rvvlm_exp10DI_std_epsim
#define RVVLM_EXP10DI_TBL64 rvvlm_exp10DI_tbl64

// FP64 expm1 function configuration
#define RVVLM_EXPM1D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_EXPM1D_STD_EPSIM rvvlm_expm1

#define RVVLM_EXPM1DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_EXPM1DI_STD_EPSIM rvvlm_expm1I

// FP64 log function configuration
#define RVVLM_LOGD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOGD_TBL128 rvvlm_logD_tbl128
#define RVVLM_LOGD_ATANH rvvlm_log

#define RVVLM_LOGDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOGDI_TBL128 rvvlm_logDI_tbl128
#define RVVLM_LOGDI_ATANH rvvlm_logI

#define RVVLM_LOG2D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG2D_TBL128 rvvlm_log2D_tbl128
#define RVVLM_LOG2D_ATANH rvvlm_log2

#define RVVLM_LOG2DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG2DI_TBL128 rvvlm_log2DI_tbl128
#define RVVLM_LOG2DI_ATANH rvvlm_log2I

#define RVVLM_LOG10D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG10D_TBL128 rvvlm_log10D_tbl128
#define RVVLM_LOG10D_ATANH rvvlm_log10

#define RVVLM_LOG10DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG10DI_TBL128 rvvlm_log10DI_tbl128
#define RVVLM_LOG10DI_ATANH rvvlm_log10I

// FP64 log1p function configuration
#define RVVLM_LOG1PD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG1PD_TBL128 rvvlm_log1p

#define RVVLM_LOG1PDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG1PDI_TBL128 rvvlm_log1pI

// FP64 pow function configuration
#define RVVLM_POWD_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_POWD_TBL rvvlm_pow

#define RVVLM_POWDI_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_POWDI_TBL rvvlm_powI

// FP64 cos function configuration
#define RVVLM_COSD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_COSD_MERGED rvvlm_cos

#define RVVLM_COSDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_COSDI_MERGED rvvlm_cosI

#define RVVLM_COSPID_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_COSPID_MERGED rvvlm_cospi

#define RVVLM_COSPIDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_COSPIDI_MERGED rvvlm_cospiI

// FP64 sin function configuration
#define RVVLM_SIND_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SIND_MERGED rvvlm_sin

#define RVVLM_SINDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINDI_MERGED rvvlm_sinI

#define RVVLM_SINPID_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINPID_MERGED rvvlm_sinpi

#define RVVLM_SINPIDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINPIDI_MERGED rvvlm_sinpiI

// FP64 sincos function configuration
#define RVVLM_SINCOSD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINCOSD_STD rvvlm_sincos

#define RVVLM_SINCOSDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINCOSDI_STD rvvlm_sincosI

#define RVVLM_SINCOSPID_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINCOSPID_STD rvvlm_sincospi

#define RVVLM_SINCOSPIDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINCOSPIDI_STD rvvlm_sincospiI

// FP64 tan function configuration
#define RVVLM_TAND_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_TAND_MERGED rvvlm_tan

#define RVVLM_TANDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_TANDI_MERGED rvvlm_tanI

#define RVVLM_TANPID_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_TANPID_MERGED rvvlm_tanpi

#define RVVLM_TANPIDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_TANPIDI_MERGED rvvlm_tanpiI

// FP64 cosh function configuration
#define RVVLM_COSHD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_COSHD_STD rvvlm_coshD_std
#define RVVLM_COSHD_MIXED rvvlm_cosh

#define RVVLM_COSHDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_COSHDI_STD rvvlm_coshI
#define RVVLM_COSHDI_MIXED rvvlm_coshDI_mixed

// FP64 sinh function configuration
#define RVVLM_SINHD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINHD_STD rvvlm_sinhD_std
#define RVVLM_SINHD_MIXED rvvlm_sinh

#define RVVLM_SINHDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_SINHDI_STD rvvlm_sinhI
#define RVVLM_SINHDI_MIXED rvvlm_sinhDI_mixed

// FP64 tanh function configuration
#define RVVLM_TANHD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_TANHD_STD rvvlm_tanh

#define RVVLM_TANHDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_TANHDI_STD rvvlm_tanhI

// Define the various tables for table-driven implementations
extern int64_t expD_tbl64_fixedpt[64];
extern int64_t logD_tbl128_fixedpt[128];
extern double logtbl_4_powD_128_hi_lo[256];
extern double dbl_2ovpi_tbl[28];

// Define the functions in the vector math library
void RVVLM_ACOSD_FIXEDPT(size_t x_len, const double *x, double *y);
void RVVLM_ACOSDI_FIXEDPT(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y);

void RVVLM_ACOSPID_FIXEDPT(size_t x_len, const double *x, double *y);
void RVVLM_ACOSPIDI_FIXEDPT(size_t x_len, const double *x, size_t stride_x,
                            double *y, size_t stride_y);

void RVVLM_ASIND_FIXEDPT(size_t x_len, const double *x, double *y);
void RVVLM_ASINDI_FIXEDPT(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y);

void RVVLM_ASINPID_FIXEDPT(size_t x_len, const double *x, double *y);
void RVVLM_ASINPIDI_FIXEDPT(size_t x_len, const double *x, size_t stride_x,
                            double *y, size_t stride_y);

void RVVLM_ATAND_FIXEDPT(size_t x_len, const double *x, double *y);
void RVVLM_ATANDI_FIXEDPT(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y);

void RVVLM_ATANPID_FIXEDPT(size_t x_len, const double *x, double *y);
void RVVLM_ATANPIDI_FIXEDPT(size_t x_len, const double *x, size_t stride_x,
                            double *y, size_t stride_y);

void RVVLM_ATAN2D_FIXEDPT(size_t xy_len, const double *y, const double *x,
                          double *z);
void RVVLM_ATAN2DI_FIXEDPT(size_t xy_len, const double *y, size_t stride_y,
                           const double *x, size_t stride_x, double *z,
                           size_t stride_z);

void RVVLM_ATAN2PID_FIXEDPT(size_t xy_len, const double *y, const double *x,
                            double *z);
void RVVLM_ATAN2PIDI_FIXEDPT(size_t xy_len, const double *y, size_t stride_y,
                             const double *x, size_t stride_x, double *z,
                             size_t stride_z);

void RVVLM_ACOSHD_STD(size_t x_len, const double *x, double *y);
void RVVLM_ACOSHDI_STD(size_t x_len, const double *x, size_t stride_x,
                       double *y, size_t stride_y);

void RVVLM_ASINHD_STD(size_t x_len, const double *x, double *y);
void RVVLM_ASINHDI_STD(size_t x_len, const double *x, size_t stride_x,
                       double *y, size_t stride_y);

void RVVLM_ATANHD_MIXED(size_t x_len, const double *x, double *y);
void RVVLM_ATANHDI_MIXED(size_t x_len, const double *x, size_t stride_x,
                         double *y, size_t stride_y);

void RVVLM_CBRTD_ITER(size_t x_len, const double *x, double *y);
void RVVLM_CBRTDI_ITER(size_t x_len, const double *x, size_t stride_x,
                       double *y, size_t stride_y);

void RVVLM_CDFNORMD_STD(size_t x_len, const double *x, double *y);
void RVVLM_CDFNORMDI_STD(size_t x_len, const double *x, size_t stride_x,
                         double *y, size_t stride_y);

void RVVLM_ERFD_STD(size_t x_len, const double *x, double *y);
void RVVLM_ERFDI_STD(size_t x_len, const double *x, size_t stride_x, double *y,
                     size_t stride_y);

void RVVLM_ERFCD_STD(size_t x_len, const double *x, double *y);
void RVVLM_ERFCDI_STD(size_t x_len, const double *x, size_t stride_x, double *y,
                      size_t stride_y);

void RVVLM_ERFCINVD_STD(size_t x_len, const double *x, double *y);
void RVVLM_ERFCINVDI_STD(size_t x_len, const double *x, size_t stride_x,
                         double *y, size_t stride_y);

void RVVLM_ERFINVD_STD(size_t x_len, const double *x, double *y);
void RVVLM_ERFINVDI_STD(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);

void RVVLM_EXPD_STD(size_t x_len, const double *x, double *y);
void RVVLM_EXPDI_STD(size_t x_len, const double *x, size_t stride_x, double *y,
                     size_t stride_y);
void RVVLM_EXPD_STD_R_EXTRA(size_t x_len, const double *x, double *y);
void RVVLM_EXPDI_STD_R_EXTRA(size_t x_len, const double *x, size_t stride_x,
                             double *y, size_t stride_y);
void RVVLM_EXPD_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXPDI_STD_EPSIM(size_t x_len, const double *x, size_t stride_x,
                           double *y, size_t stride_y);
void RVVLM_EXPD_TBL64(size_t x_len, const double *x, double *y);
void RVVLM_EXPDI_TBL64(size_t x_len, const double *x, size_t stride_x,
                       double *y, size_t stride_y);

void RVVLM_EXP2D_STD(size_t x_len, const double *x, double *y);
void RVVLM_EXP2DI_STD(size_t x_len, const double *x, size_t stride_x, double *y,
                      size_t stride_y);
void RVVLM_EXP2D_STD_R_EXTRA(size_t x_len, const double *x, double *y);
void RVVLM_EXP2DI_STD_R_EXTRA(size_t x_len, const double *x, size_t stride_x,
                              double *y, size_t stride_y);
void RVVLM_EXP2D_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXP2DI_STD_EPSIM(size_t x_len, const double *x, size_t stride_x,
                            double *y, size_t stride_y);
void RVVLM_EXP2D_TBL64(size_t x_len, const double *x, double *y);
void RVVLM_EXP2DI_TBL64(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);

void RVVLM_EXP10D_STD(size_t x_len, const double *x, double *y);
void RVVLM_EXP10DI_STD(size_t x_len, const double *x, size_t stride_x,
                       double *y, size_t stride_y);
void RVVLM_EXP10D_STD_R_EXTRA(size_t x_len, const double *x, double *y);
void RVVLM_EXP10DI_STD_R_EXTRA(size_t x_len, const double *x, size_t stride_x,
                               double *y, size_t stride_y);
void RVVLM_EXP10D_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXP10DI_STD_EPSIM(size_t x_len, const double *x, size_t stride_x,
                             double *y, size_t stride_y);
void RVVLM_EXP10D_TBL64(size_t x_len, const double *x, double *y);
void RVVLM_EXP10DI_TBL64(size_t x_len, const double *x, size_t stride_x,
                         double *y, size_t stride_y);

void RVVLM_EXPM1D_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXPM1DI_STD_EPSIM(size_t x_len, const double *x, size_t stride_x,
                             double *y, size_t stride_y);

void RVVLM_LOGD_TBL128(size_t x_len, const double *x, double *y);
void RVVLM_LOGDI_TBL128(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);
void RVVLM_LOGD_ATANH(size_t x_len, const double *x, double *y);
void RVVLM_LOGDI_ATANH(size_t x_len, const double *x, size_t stride_x,
                       double *y, size_t stride_y);

void RVVLM_LOG10D_TBL128(size_t x_len, const double *x, double *y);
void RVVLM_LOG10DI_TBL128(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y);
void RVVLM_LOG10D_ATANH(size_t x_len, const double *x, double *y);
void RVVLM_LOG10DI_ATANH(size_t x_len, const double *x, size_t stride_x,
                         double *y, size_t stride_y);

void RVVLM_LOG2D_TBL128(size_t x_len, const double *x, double *y);
void RVVLM_LOG2DI_TBL128(size_t x_len, const double *x, size_t stride_x,
                         double *y, size_t stride_y);
void RVVLM_LOG2D_ATANH(size_t x_len, const double *x, double *y);
void RVVLM_LOG2DI_ATANH(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);

void RVVLM_LOG1PD_TBL128(size_t x_len, const double *x, double *y);
void RVVLM_LOG1PDI_TBL128(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y);

void RVVLM_POWD_TBL(size_t x_len, const double *x, const double *y, double *z);
void RVVLM_POWDI_TBL(size_t x_len, const double *x, size_t stride_x,
                     const double *y, size_t stride_y, double *z,
                     size_t stride_z);

void RVVLM_COSD_MERGED(size_t x_len, const double *x, double *y);
void RVVLM_COSDI_MERGED(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);

void RVVLM_COSPID_MERGED(size_t x_len, const double *x, double *y);
void RVVLM_COSPIDI_MERGED(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y);

void RVVLM_SIND_MERGED(size_t x_len, const double *x, double *y);
void RVVLM_SINDI_MERGED(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);

void RVVLM_SINPID_MERGED(size_t x_len, const double *x, double *y);
void RVVLM_SINPIDI_MERGED(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y);

void RVVLM_SINCOSD_STD(size_t x_len, const double *x, double *y, double *z);
void RVVLM_SINCOSDI_STD(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y, double *z, size_t stride_z);

void RVVLM_SINCOSPID_STD(size_t x_len, const double *x, double *y, double *z);
void RVVLM_SINCOSPIDI_STD(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y, double *z,
                          size_t stride_z);

void RVVLM_TAND_MERGED(size_t x_len, const double *x, double *y);
void RVVLM_TANDI_MERGED(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);

void RVVLM_TANPID_MERGED(size_t x_len, const double *x, double *y);
void RVVLM_TANPIDI_MERGED(size_t x_len, const double *x, size_t stride_x,
                          double *y, size_t stride_y);

void RVVLM_COSHD_STD(size_t x_len, const double *x, double *y);
void RVVLM_COSHDI_STD(size_t x_len, const double *x, size_t stride_x, double *y,
                      size_t stride_y);

void RVVLM_COSHD_MIXED(size_t x_len, const double *x, double *y);
void RVVLM_COSHDI_MIXED(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);

void RVVLM_SINHD_STD(size_t x_len, const double *x, double *y);
void RVVLM_SINHDI_STD(size_t x_len, const double *x, size_t stride_x, double *y,
                      size_t stride_y);

void RVVLM_SINHD_MIXED(size_t x_len, const double *x, double *y);
void RVVLM_SINHDI_MIXED(size_t x_len, const double *x, size_t stride_x,
                        double *y, size_t stride_y);

void RVVLM_TANHD_STD(size_t x_len, const double *x, double *y);
void RVVLM_TANHDI_STD(size_t x_len, const double *x, size_t stride_x, double *y,
                      size_t stride_y);

#ifdef __cplusplus
}
#endif
