// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "rvvlm_trigD.h"

#if defined(COMPILE_FOR_SIN)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_SIND_MERGED
#else
#define F_VER1 RVVLM_SINDI_MERGED
#endif
#elif defined(COMPILE_FOR_SINPI)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_SINPID_MERGED
#else
#define F_VER1 RVVLM_SINPIDI_MERGED
#endif
#elif defined(COMPILE_FOR_COS)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_COSD_MERGED
#else
#define F_VER1 RVVLM_COSDI_MERGED
#endif
#elif defined(COMPILE_FOR_COSPI)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_COSPID_MERGED
#else
#define F_VER1 RVVLM_COSPIDI_MERGED
#endif
#else
static_assert(false, "Must specify sin, sinpi, cos or cospi" __FILE__);
#endif

// This versions reduces argument to [-pi/4, pi/4] and computes sin(r) or cos(r)
// by merging the appropriate coefficients into a vector register
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx_orig, vx, vy, vy_special;
  VBOOL special_args;
  VUINT expo_x;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx_orig = VFLOAD_INARG1(vlen);
    vx = __riscv_vfsgnj(vx_orig, fp_posOne, vlen);
    expo_x = __riscv_vsrl(F_AS_U(vx), MAN_LEN, vlen);

    // Set results for input of NaN and Inf and also for |x| very small
    EXCEPTION_HANDLING_TRIG(vx_orig, expo_x, special_args, vy_special, vlen);

#if defined(COMPILE_FOR_COS) || defined(COMPILE_FOR_SIN)
    VBOOL x_large =
        __riscv_vmsgeu(expo_x, EXP_BIAS + 24, vlen); // |x| >= 2^(24)
    VFLOAT vx_copy = vx;
    vx = __riscv_vfmerge(vx, fp_posZero, x_large, vlen);

    VFLOAT n_flt = __riscv_vfmul(vx, PIBY2_INV, vlen);
    VINT n = __riscv_vfcvt_x(n_flt, vlen);
    n_flt = __riscv_vfcvt_f(n, vlen);
    VFLOAT r_hi = __riscv_vfnmsac(vx, PIBY2_HI, n_flt, vlen);
    VUINT expo_r = __riscv_vsrl(F_AS_U(r_hi), MAN_LEN, vlen);
    expo_r = __riscv_vand(expo_r, 0x7FF, vlen);
    VBOOL r_small =
        __riscv_vmsleu(expo_r, EXP_BIAS - 16, vlen); // |r_hi| < 2^(-15)
    UINT nb_r_small = __riscv_vcpop(r_small, vlen);
    VFLOAT r = __riscv_vfnmsac(r_hi, PIBY2_MID, n_flt, vlen);
    VFLOAT r_delta = __riscv_vfsub(r_hi, r, vlen);
    r_delta = __riscv_vfnmsac(r_delta, PIBY2_MID, n_flt, vlen);
    // At this point, r + r_delta is an accurate reduced argument PROVIDED
    // |r_hi| >= 2^(-15)
    if (nb_r_small > 0) {
      VFLOAT A = __riscv_vfmul(n_flt, PIBY2_MID, vlen);
      VFLOAT a = __riscv_vfmsub(n_flt, PIBY2_MID, A, vlen);
      // A + a is n * piby2_mid exactly
      VFLOAT S = __riscv_vfsub(r_hi, A, vlen);
      VFLOAT s = __riscv_vfsub(r_hi, S, vlen);
      s = __riscv_vfsub(s, A, vlen);
      s = __riscv_vfnmsac(s, PIBY2_LO, n_flt, vlen);
      r = __riscv_vmerge(r, S, r_small, vlen);
      r_delta = __riscv_vmerge(r_delta, s, r_small, vlen);
    }

    if (__riscv_vcpop(x_large, vlen) > 0) {
      VFLOAT r_xlarge, r_delta_xlarge;
      VINT n_xlarge;
      LARGE_ARGUMENT_REDUCTION_Piby2(vx_copy, vlen, x_large, n_xlarge, r_xlarge,
                                     r_delta_xlarge);
      r = __riscv_vmerge(r, r_xlarge, x_large, vlen);
      r_delta = __riscv_vmerge(r_delta, r_delta_xlarge, x_large, vlen);
      n = __riscv_vmerge(n, n_xlarge, x_large, vlen);
    }
#else
    VBOOL x_large =
        __riscv_vmsgeu(expo_x, EXP_BIAS + 53, vlen); // |x| >= 2^(53)
    vx = __riscv_vfmerge(vx, fp_posZero, x_large, vlen);

    // Usual argument reduction
    // N = rint(2x); rem := 2x - N, |rem| <= 1/2 and x = (N/2) + (rem/2);
    // x pi = N (pi/2) + rem * (pi/2)
    VFLOAT two_x = __riscv_vfadd(vx, vx, vlen);
    VINT n = __riscv_vfcvt_x(two_x, vlen);
    VFLOAT n_flt = __riscv_vfcvt_f(n, vlen);
    VFLOAT rem = __riscv_vfsub(two_x, n_flt, vlen);
    VBOOL x_is_n_piby2 = __riscv_vmseq(F_AS_U(rem), 0, vlen);
    // Now rem * pi_by_2 as r + r_delta
    VFLOAT r = __riscv_vfmul(rem, PIBY2_HI, vlen);
    VFLOAT r_delta = __riscv_vfmsac(r, PIBY2_HI, rem, vlen);
    r_delta = __riscv_vfmacc(r_delta, PIBY2_MID, rem, vlen);
    // At this point, r + r_delta is an accurate reduced argument PROVIDED
#endif

    VUINT n_lsb = __riscv_vand(I_AS_U(n), 0x1, vlen);
#if defined(COMPILE_FOR_SIN) || defined(COMPILE_FOR_SINPI)
    VBOOL pick_c = __riscv_vmsne(n_lsb, 0, vlen);
#else
    VBOOL pick_c = __riscv_vmseq(n_lsb, 0, vlen);
#endif

#if defined(COMPILE_FOR_SINPI) || defined(COMPILE_FOR_COSPI)
    VBOOL exact_zero = __riscv_vmandn(x_is_n_piby2, pick_c, vlen);
#endif

    // Instead of always computing both sin(r) and cos(r) for |r| <= pi/4
    // We merge the sin and cos case together in picking the correct
    // polynomial coefficients. This way we save on the bulk of the poly
    // computation except for a couple of terms.

    // This standard algorithm either computes sin(r+r_delta) or
    // cos(r+r_delta), depending on the parity of n
    // Note that sin(t) = t + t^3(s_poly(t^2))
    // and       cos(t) = 1 - t^2/2 + t^4(c_poly(t^2))
    // where s_poly and c_poly are of the same degree. Hence
    // it suffices to load the coefficient vector with the correct
    // coefficients for s_poly or c_poly. We compute the needed s_poly or c_poly
    // without wasteful operations. (That is, computing s_poly for all r
    // and c_poly for all r and in general discarding half of these results.)
    //

    // sin(r+r_delta) ~=~ sin(r) + r_delta(1 - r^2/2)
    // sin(r) is approximated by 7 terms, starting from x, x^3, ..., x^13
    // cos(r+r_delta) ~=~ cos(r) - r * r_delta
    //
    VFLOAT rsq, rcube, r_to_6, s_corr, c_corr, r_prime, One, C;
    One = VFMV_VF(fp_posOne, vlen);
    rsq = __riscv_vfmul(r, r, vlen);
    rcube = __riscv_vfmul(rsq, r, vlen);
    r_to_6 = __riscv_vfmul(rcube, rcube, vlen);

    r_prime = __riscv_vfmul(r, -0x1.0p-1, vlen);
    C = __riscv_vfmacc(One, r_prime, r, vlen);
    s_corr = __riscv_vfmul(r_delta, C, vlen);

    c_corr = __riscv_vfsub(One, C, vlen);
    c_corr = __riscv_vfmacc(c_corr, r, r_prime, vlen);
    c_corr = __riscv_vfnmsac(c_corr, r, r_delta, vlen);

    VFLOAT poly_right = VFMV_VF(0x1.5d8b5ae12066ap-33, vlen);
    poly_right =
        __riscv_vfmerge(poly_right, -0x1.8f5dd75850673p-37, pick_c, vlen);
    poly_right =
        PSTEP_ab(pick_c, -0x1.27e4f72551e3dp-22, 0x1.71de35553ddb6p-19, rsq,
                 PSTEP_ab(pick_c, 0x1.1ee950032f74cp-29, -0x1.ae5e4b94836f8p-26,
                          rsq, poly_right, vlen),
                 vlen);

    VFLOAT poly_left = VFMV_VF(-0x1.a01a019be932ap-13, vlen);
    poly_left = __riscv_vfmerge(poly_left, 0x1.a01a019b77545p-16, pick_c, vlen);
    poly_left =
        PSTEP_ab(pick_c, 0x1.5555555555546p-5, -0x1.5555555555548p-3, rsq,
                 PSTEP_ab(pick_c, -0x1.6c16c16c1450cp-10, 0x1.111111110f730p-7,
                          rsq, poly_left, vlen),
                 vlen);

    poly_right = __riscv_vfmadd(poly_right, r_to_6, poly_left, vlen);

    VFLOAT t = __riscv_vfmul(rsq, rsq, vlen);
    t = __riscv_vmerge(rcube, t, pick_c, vlen);
    // t is r^3 for sin(r) and r^4 for cos(r)

    VFLOAT A = __riscv_vmerge(r, C, pick_c, vlen);
    VFLOAT a = __riscv_vmerge(s_corr, c_corr, pick_c, vlen);
    vy = __riscv_vfmadd(poly_right, t, a, vlen);
    vy = __riscv_vfadd(A, vy, vlen);

    n = __riscv_vsll(n, BIT_WIDTH - 2, vlen);
    vy = __riscv_vfsgnjx(vy, I_AS_F(n), vlen);
#if defined(COMPILE_FOR_COS) || defined(COMPILE_FOR_COSPI)
    n_lsb = __riscv_vsll(n_lsb, 63, vlen);
    vy = __riscv_vfsgnjx(vy, U_AS_F(n_lsb), vlen);
#endif

#if defined(COMPILE_FOR_SINPI) || defined(COMPILE_FOR_COSPI)
    vy = __riscv_vmerge(vy, VFMV_VF(fp_posZero, vlen), exact_zero, vlen);
#endif

#if defined(COMPILE_FOR_SIN) || defined(COMPILE_FOR_SINPI)
    vy = __riscv_vfsgnjx(vy, vx_orig, vlen);
#endif

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
