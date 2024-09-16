// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "rvvlm_invhyperD.h"

#if defined(COMPILE_FOR_ACOSH)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ACOSHD_STD
#else
#define F_VER1 RVVLM_ACOSHDI_STD
#endif
#else
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ASINHD_STD
#else
#define F_VER1 RVVLM_ASINHDI_STD
#endif
#endif

// Acosh(x)  is defined for x >= 1 by the formula log(x + sqrt(x*x - 1))
// Asinh(x) is defined for all finite x by the formula log(x + sqrt(x*x + 1))
// Acosh is always positive, and Asinh(-x) = -Asinh(x). Thus we in general
// work with |x| and restore the sign (if necessary) in the end.
// For the log function log(2^n z), we uses the expansion in terms of atanh:
// n log(2) + 2 atanh((z-1)/(z+1))
// The algorithm here first scales down x by 2^(-550) when |x| >= 2^500.
// And for such large x, both acosh and asinh equals log(2x) to very high
// precision. We safely ignore the +/- 1 when this is the case.
//
// A power 2^n is determined by the value of x + sqrt(x*x +/- 1) so that
// scaling the expression by 2^(-n) transforms it to the range [0.71, 1.42].
// Log(t) for t in this region is computed by 2 atanh((t-1)/(t+1))
// More precisely, we use s = 2(t-1)/(t+1) and approximate the function
// 2 atanh(s/2) by  s + s^3 * polynomial(s^2).
// The final result is n * log(2) + s + s^3 * polynomial(s^2)
// which is computed with care.
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);
#if defined(COMPILE_FOR_ASINH)
    VFLOAT vx_orig = vx;
#endif

#if defined(COMPILE_FOR_ACOSH)
    // Handle Inf and NaN and input <= 1.0
    EXCEPTION_HANDLING_ACOSH(vx, special_args, vy_special, vlen);
#else
    // Handle Inf and NaN and |input| < 2^(-30)
    EXCEPTION_HANDLING_ASINH(vx, special_args, vy_special, vlen);
    vx = __riscv_vfsgnj(vx, fp_posOne, vlen);
#endif

    // Need to scale x so that x + sqrt(x*x +/- 1) doesn't overflow
    // We scale x down by 2^(-550) if x >= 2^500 and set the "+/- 1" to 0
    VINT n;
    VFLOAT u;
    SCALE_X(vx, n, u, vlen);
    // n is 0 or 500; and u is +/-1.0 or 0.0

    // sqrt(x*x + u) extra precisely
    VFLOAT A, a;
#if defined(COMPILE_FOR_ACOSH)
    XSQ_PLUS_U_ACOSH(vx, u, A, a, vlen);
#else
    XSQ_PLUS_U_ASINH(vx, u, A, a, vlen);
#endif
    // A + a is x*x + u

    VFLOAT B, b;
#if defined(COMPILE_FOR_ACOSH)
    SQRT2_X2(A, a, B, b, vlen);
    // B + b is sqrt(x*x + u) to about 7 extra bits
#else
    // For asinh, we need the sqrt to double-double precision
    VFLOAT recip = __riscv_vfrdiv(A, fp_posOne, vlen);
    B = __riscv_vfsqrt(A, vlen);
    b = __riscv_vfnmsub(B, B, A, vlen);
    b = __riscv_vfadd(b, a, vlen);
    VFLOAT B_recip = __riscv_vfmul(B, recip, vlen);
    b = __riscv_vfmul(b, 0x1.0p-1, vlen);
    b = __riscv_vfmul(b, B_recip, vlen);
#endif

    VFLOAT S, s;
#if defined(COMPILE_FOR_ACOSH)
    // x dominantes B for acosh
    FAST2SUM(vx, B, S, s, vlen);
    s = __riscv_vfadd(s, b, vlen);
#else
    // B dominates x for asinh
    FAST2SUM(B, vx, S, s, vlen);
    s = __riscv_vfadd(s, b, vlen);
#endif

    // x + sqrt(x*x + u) is accurately represented as S + s
    // We first scale S, s by 2^(-n) so that the scaled value
    // falls roughly in [1/rt2, rt2]
    SCALE_4_LOG(S, s, n, vlen);

    // log(x + sqrt(x*x + u)) = n * log(2) + log(y); y = S + s
    // We use log(y) = 2 atanh( (y-1)/(y+1) ) and approximate the latter
    // by t + t^3 * poly(t^2), t = 2 (y-1)/(y+1)

    // We now compute the numerator 2(y-1) and denominator y+1 and their
    // quotient to extra precision
    VFLOAT numer, delta_numer, denom, delta_denom;
    TRANSFORM_2_ATANH(S, s, numer, delta_numer, denom, delta_denom, vlen);

    VFLOAT r_hi, r_lo, r;
    DIV2_N2D2(numer, delta_numer, denom, delta_denom, r_hi, r_lo, vlen);
    r = __riscv_vfadd(r_hi, r_lo, vlen);

    VFLOAT poly;
    LOG_POLY(r, r_lo, poly, vlen);
    // At this point r_hi + poly approximates log(X)

    // Compose the final result: n * log(2) + r_hi + poly
    VFLOAT n_flt = __riscv_vfcvt_f(n, vlen);
    A = __riscv_vfmul(n_flt, LOG2_HI, vlen);
    FAST2SUM(A, r_hi, S, s, vlen);
    s = __riscv_vfmacc(s, LOG2_LO, n_flt, vlen);
    s = __riscv_vfadd(s, poly, vlen);

    vy = __riscv_vfadd(S, s, vlen);
#if defined(COMPILE_FOR_ASINH)
    vy = __riscv_vfsgnj(vy, vx_orig, vlen);
#endif
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
