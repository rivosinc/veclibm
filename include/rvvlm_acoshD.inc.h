// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "rvvlm_invhyperD.h"

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ACOSHD_STD
#else
#define F_VER1 RVVLM_ACOSHDI_STD
#endif

#include <fenv.h>

// Acosh(x)  is defined for x >= 1 by the formula log(x + sqrt(x*x - 1))
// and for the log function log(2^n z), we uses the expansion in terms of atanh
// n log(2) + 2 atanh((z-1)/(z+1))
// This algorithm obtains this scale factor 2^n from the input x, and computes
// the expression x' + sqrt(x'*x' - 2^(-2n)) thus avoiding possible overflow or
// excess computation such as computing sqrt(x*x - 1) by x * sqrt(1 - 1/(x*x))
// which needs a division on top of a sqrt.
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vx_orig, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);
#if defined(COMPILE_FOR_ASINH)
    vx_orig = vx;
#endif

#if defined(COMPILE_FOR_ACOSH)
    // Handle Inf and NaN and input <= 1.0
    EXCEPTION_HANDLING_ACOSH(vx, special_args, vy_special, vlen);
#else
    // Handle Inf and NaN and |input} < 2^(-30)
    EXCEPTION_HANDLING_ASINH(vx, special_args, vy_special, vlen);
    vx = __riscv_vfsgnj(vx, fp_posOne, vlen);
#endif

    // Need to scale x so that x + sqrt(x*x +/- 1) doesn't overflow
    // Since x >= 1, we scale x down by 2^(-550) if x >= 2^500 and set 1 to 0
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
    SQRT2_X2(A, a, B, b, vlen);
    // B + b is sqrt(x*x + u) to about 7 extra bits

    // x dominants B for acosh
    VFLOAT S, s;
#if defined(COMPILE_FOR_ACOSH)
    FAST2SUM(vx, B, S, s, vlen);
    s = __riscv_vfadd(s, b, vlen);
#else
    FAST2SUM(B, vx, S, s, vlen);
    s = __riscv_vfadd(s, b, vlen);
#endif

    // x + sqrt(x*x + u) is accurately represented as S + s
    // We first scale S, s so that it falls roughly in [1/rt2, rt2]
    SCALE_4_LOG(S, s, n, vlen);

    // log(x + sqrt(x*x + u)) = n * log(2) + log(y); y = S + s
    // since log(y) = 2 atanh( (y-1)/(y+1) ) to be approximated
    // by t + t^3 * poly(t^2), t = 2 (y-1)/(y+1)
    // We now compute the numerator and denominator and its quotient
    // to extra precision
    VFLOAT numer, delta_numer, denom, delta_denom;
    TRANSFORM_2_ATANH(S, s, numer, delta_numer, denom, delta_denom, vlen);

    VFLOAT r_hi, r_lo, r;
    DIV2_N2D2(numer, delta_numer, denom, delta_denom, r_hi, r_lo, vlen);
    r = __riscv_vfadd(r_hi, r_lo, vlen);

    VFLOAT n_flt = __riscv_vfcvt_f(n, vlen);

    VFLOAT poly;
    LOG_POLY(r, r_lo, poly, vlen);
    // At this point r_hi + poly approximates log(X)

    // Reconstruction: logB(in_arg) = n logB(2) + log(X) * logB(e), computed as
    // n*(logB_2_hi + logB_2_lo) + r * (logB_e_hi + logB_e_lo) + poly *
    // logB_e_hi It is best to compute n * logB_2_hi + r * logB_e_hi in extra
    // precision

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
