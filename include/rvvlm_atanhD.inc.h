// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "rvvlm_invhyperD.h"

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ATANHD_MIXED
#else
#define F_VER1 RVVLM_ATANHDI_MIXED
#endif

#include <fenv.h>

// Atanh(x) is defined only for |x| <= 1. As atanh(-x) = -atanh(x), the
// main computation works with |x|.
// For |x| > 1 and x being a sNaN, the invalid signal has to be generated
// together with a returned valued of NaN. For a qNaN input, no signal is
// generated. And atan(+/- 1) yiedls +/- Inf, but a div-by-zero signal has
// to be generated.
//
// For 0 < x < 1, we use the formula atan(x) = (1/2) log( (1+x)/(1-x) ).
// The usual technique is to find a scale s = 2^(-n) so that
// r = s * (1+x)/(1-x) falls roughly in the region [1/sqrt(2), sqrt(2)].
// Thus the desired result is (1/2)(n * log(2) + log(r)).
// Somewhat ironically, log(r) is usually approximated in terms of atanh,
// as its Taylor series around 0 converges much faster than that of log(r)
// around 1. log(r) = 2 atanh( (r-1)/(r+1) ).
// Hence, atan(x) = (n/2)log(2) + atan([(1+x)-(1-x)/s]/[(1+x)+(1-x)/s]).
//
// This implementation obtains s=2^(-n) using the approximate reciprocal
// instruction rather than computing (1+x)/(1-x) to extra precision.
// It then combines the two transformations into
// atanh( [(1+x) - (1-x)/s] / [(1+x) + (1-x)/s] ) requiring only
// one division, instead of two.
// We further observe that instead of using multiple extra-precise
// simulations to obtain both the numerator and denominator accurately,
// we can use fixed-point computations.
// As long as the original input |x| >= 0.248, a scale of 60 allows
// both numerator and denominator to maintain high precision without overflow,
// elminating many double-double like simulations. For |x| < 0.248, the
// core polynomial evaluated at x yields the result.
//
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vx_orig, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);
    vx_orig = vx;

    // Handle Inf, NaN, |input| >= 1, and  |input| < 2^(-30)
    EXCEPTION_HANDLING_ATANH(vx, special_args, vy_special, vlen);
    vx = __riscv_vfsgnj(vx, fp_posOne, vlen);

    // At this point vx are positive number, either 0, or 2^(-30) <= x < 1.

    // Get n so that 2^(-n) * (1+x)/(1-x) is in roughly in the range [1/rt2,
    // rt2]
    VUINT n;
    VFLOAT one_plus_x, one_minus_x;
    one_plus_x = __riscv_vfadd(vx, fp_posOne, vlen);
    one_minus_x = __riscv_vfrsub(vx, fp_posOne, vlen);
    // note one_minus_x >= 2^(-53) is never 0
    VFLOAT ratio =
        __riscv_vfmul(one_plus_x, __riscv_vfrec7(one_minus_x, vlen), vlen);
    n = __riscv_vadd(__riscv_vsrl(F_AS_U(ratio), MAN_LEN - 8, vlen), 0x96,
                     vlen);
    n = __riscv_vsub(__riscv_vsrl(n, 8, vlen), EXP_BIAS, vlen);

    VINT X = __riscv_vfcvt_x(__riscv_vfmul(vx, 0x1.0p60, vlen), vlen);
    VINT Numer, Denom;
    // no overflow, so it does not matter if we use the saturating add or not
    VINT One_plus_X = __riscv_vadd(X, ONE_Q60, vlen);
    VINT One_minus_X = __riscv_vrsub(X, ONE_Q60, vlen);
    One_minus_X = __riscv_vsll(One_minus_X, n, vlen);
    Numer = __riscv_vsub(One_plus_X, One_minus_X, vlen);
    Denom = __riscv_vadd(One_plus_X, One_minus_X, vlen);
    VFLOAT numer, delta_numer, denom, delta_denom;
    numer = __riscv_vfcvt_f(Numer, vlen);
    VINT Tail = __riscv_vsub(Numer, __riscv_vfcvt_x(numer, vlen), vlen);
    delta_numer = __riscv_vfcvt_f(Tail, vlen);
    denom = __riscv_vfcvt_f(Denom, vlen);
    Tail = __riscv_vsub(Denom, __riscv_vfcvt_x(denom, vlen), vlen);
    delta_denom = __riscv_vfcvt_f(Tail, vlen);

    VFLOAT r_hi, r_lo, r;
    DIV2_N2D2(numer, delta_numer, denom, delta_denom, r_hi, r_lo, vlen);
    VBOOL x_in_range = __riscv_vmflt(vx, 0x1.fcp-3, vlen);
    r_hi = __riscv_vmerge(r_hi, vx, x_in_range, vlen);
    r_lo = __riscv_vfmerge(r_lo, fp_posZero, x_in_range, vlen);
    n = __riscv_vmerge(n, 0, x_in_range, vlen);

    r = __riscv_vfadd(r_hi, r_lo, vlen);
    VFLOAT rsq = __riscv_vfmul(r, r, vlen);
    VFLOAT rcube = __riscv_vfmul(rsq, r, vlen);
    VFLOAT r4 = __riscv_vfmul(rsq, rsq, vlen);
    VFLOAT r8 = __riscv_vfmul(r4, r4, vlen);
    VFLOAT poly_right = PSTEP(
        0x1.745841007bbc5p-4, rsq,
        PSTEP(0x1.3b99547086d02p-4, rsq,
              PSTEP(0x1.08c7af78d89d3p-4, 0x1.35f7ae00ccaf9p-4, rsq, vlen),
              vlen),
        vlen);
    VFLOAT poly_left = PSTEP(
        0x1.555555555547dp-2, rsq,
        PSTEP(0x1.99999999d0d12p-3, rsq,
              PSTEP(0x1.249248fe05e8dp-3, 0x1.c71c8bc60dde3p-4, rsq, vlen),
              vlen),
        vlen);
    VFLOAT poly = __riscv_vfmadd(poly_right, r8, poly_left, vlen);
    poly = __riscv_vfmadd(poly, rcube, r_lo, vlen);
    // At this point r_hi + poly approximates atanh(r)

    // Compose the final answer  (n/2)*log(2) + atanh(r)
    VFLOAT n_flt = __riscv_vfcvt_f(n, vlen);
    VFLOAT A = __riscv_vfmul(n_flt, LOG2_BY2_HI, vlen);
    VFLOAT S, s;
    FAST2SUM(A, r_hi, S, s, vlen);
    s = __riscv_vfmacc(s, LOG2_BY2_LO, n_flt, vlen);
    s = __riscv_vfadd(s, poly, vlen);
    vy = __riscv_vfadd(S, s, vlen);

    vy = __riscv_vfsgnj(vy, vx_orig, vlen);

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
