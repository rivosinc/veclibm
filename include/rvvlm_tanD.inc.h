// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "rvvlm_trigD.h"

#include <fenv.h>

#if defined(COMPILE_FOR_TAN)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_TAND_MERGED
#else
#define F_VER1 RVVLM_TANDI_MERGED
#endif
#elif defined(COMPILE_FOR_TANPI)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_TANPID_MERGED
#else
#define F_VER1 RVVLM_TANPIDI_MERGED
#endif
#else
static_assert(false, "Must specify tan or tanpi" __FILE__);
#endif

#include <fenv.h>

// This versions reduces argument to [-pi/4, pi/4] and computes sin(r) or cos(r)
// tan(x) is either sin(r)/cos(r) or -cos(r)/sin(r)
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

#if defined(COMPILE_FOR_TAN)
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
    // tanpi can be exactly 0 or Inf when x_is_n_piby2
    // Furthermore, the signs of these 0 and Inf are as follows.
    // tanpi(-X) = -tanpi(X). Thus only consider X >= 0.
    // tanpi(n * pi/2) = (-1)^floor(n/2) {0 if n even; Inf otherwise}
    if (__riscv_vcpop(x_is_n_piby2, vlen) > 0) {
      VBOOL n_even = __riscv_vmseq(__riscv_vand(n, 0x1, vlen), 0, vlen);
      VBOOL set_inf = __riscv_vmandn(x_is_n_piby2, n_even, vlen);
      VFLOAT Zero_or_Inf = VFMV_VF(fp_posZero, vlen);
      Zero_or_Inf = __riscv_vmerge(Zero_or_Inf,
                                   __riscv_vfrec7(set_inf, Zero_or_Inf, vlen),
                                   set_inf, vlen);
      Zero_or_Inf = __riscv_vfsgnj(
          Zero_or_Inf, U_AS_F(__riscv_vsll(I_AS_U(n), BIT_WIDTH - 2, vlen)),
          vlen);
      Zero_or_Inf = __riscv_vfsgnjx(Zero_or_Inf, vx_orig, vlen);
      vy_special = __riscv_vmerge(vy_special, Zero_or_Inf, x_is_n_piby2, vlen);
      special_args = __riscv_vmor(special_args, x_is_n_piby2, vlen);
      n = __riscv_vmerge(n, 0, x_is_n_piby2, vlen);
    }
    VFLOAT r = __riscv_vfmul(rem, PIBY2_HI, vlen);
    VFLOAT r_delta = __riscv_vfmsac(r, PIBY2_HI, rem, vlen);
    r_delta = __riscv_vfmacc(r_delta, PIBY2_MID, rem, vlen);
    // At this point, r + r_delta is an accurate reduced argument PROVIDED
#endif

    VUINT n_lsb = __riscv_vand(I_AS_U(n), 0x1, vlen);
    VBOOL numer_pick_c = __riscv_vmsne(n_lsb, 0, vlen);
    VBOOL denom_pick_c = __riscv_vmnot(numer_pick_c, vlen);

    //
    // sin(r) is approximated by 8 terms corresponding to x, x^3, ..., x^15
    // cos(r) is approximated by 8 terms corresponding to 1, x^2, ..., x^14
    // This "r" is more precise than FP64; it suffices to use the FP64-precise
    // value for the last 6 terms for sin and cos. We only need to use the
    // extra precise values for the first two terms for each of the above.
    // Our strategy here is to use extra precision simulation with
    // floating-point computation
    //
    // For sin(r), the first 2 terms are r + p r^3 where p is basically -1/6
    // We decompose r into r = r_head + t_tail where r_head is r with the lower
    // 36 bits set to 0. This way, r_head^3 can be computed exactly. r + p r^3 =
    // r + r_head^3 * p  + (r^3 - r_head^3) * p r + r_head^3 * p can be computed
    // by sin_hi := r + r_head^3 * p  (FMA) sin_corr := (r - sin_hi) + r_head^3
    // * p (subtract and FMA) sin_hi + sin_corr is is r + r_head^3 * p to
    // doubled FP64 precision (way more than needed) Next we need to add (r^3 -
    // r_head^3) * p which is r_tail * (r^2 + r * r_head + r_head^2) * p because
    // r_tail is small, rounding error in computing this is immaterial to the
    // final result Finally, we need also to add r_delta * (1 - r^2/2) to
    // sin_corr because sin(r + r_delta) ~=~ sin(r) + r_delta * cos(r) ~=~
    // sin(r) + r_delta * (1 - r^2/2). Note that the term 1 - r^2/2 will be
    // computed in the course of our computation of cos(r), discussed next.
    //
    // For cos(r), the first 2 terms are 1 - r^2/2. This can be easily computed
    // to high precision. r_prime := r * 1/2; cos_hi := 1 - r * r_prime (FMA);
    // cos_corr := (1 - cos_hi) - r * r_prime cos_hi can be used above to
    // compute r_delta * (1 - r^2/2). Because cos(r + r_delta) ~=~ cos(r) -
    // r_delta * sin(r) ~=~ cos(r) - r_delta * r we add the term -r_delta * r to
    // cos_corr
    //
    // So in a nutshell sin(r) is approximated by sin_hi + sin_lo,
    // sin_lo is the sum of sin_corr and a polynomial starting at r^5
    //
    // And cos(r) is approximated by cos_hi + cos_lo,
    // cos_lo is the sum of cos_corr and a polynomial starting at r^4
    //
    // By suitably merging the two, we have numer_hi, numer_lo  and denom_hi,
    // denom_lo
    //

    VFLOAT rsq = __riscv_vfmul(r, r, vlen);

    UINT mask_r_head = 1;
    mask_r_head = ~((mask_r_head << 36) - 1);
    VFLOAT r_head = U_AS_F(__riscv_vand(F_AS_U(r), mask_r_head, vlen));
    VFLOAT r_tail = __riscv_vfsub(r, r_head, vlen);

    UINT exp_m1 = 1;
    exp_m1 = (exp_m1 << 52);
    VFLOAT r_prime = U_AS_F(__riscv_vsub(F_AS_U(r), exp_m1, vlen));
    // |r| is never too small, so subtracting 1 from exponent is division by 2

    VFLOAT ONE = VFMV_VF(fp_posOne, vlen);
    VFLOAT cos_hi = __riscv_vfnmsac(ONE, r, r_prime, vlen);
    VFLOAT cos_corr = __riscv_vfsub(ONE, cos_hi, vlen);
    cos_corr = __riscv_vfnmsac(cos_corr, r, r_prime, vlen);
    cos_corr = __riscv_vfnmsac(cos_corr, r_delta, r, vlen);

    double coeff = -0x1.5555555555555p-3;
    VFLOAT r_head_cube = __riscv_vfmul(r_head, r_head, vlen);
    r_head_cube = __riscv_vfmul(r_head_cube, r_head, vlen);
    VFLOAT sin_hi = __riscv_vfmadd(r_head_cube, coeff, r, vlen);
    VFLOAT sin_corr = __riscv_vfsub(r, sin_hi, vlen);
    sin_corr = __riscv_vfmacc(sin_corr, coeff, r_head_cube, vlen);
    VFLOAT tmp = __riscv_vfmadd(r_head, r_head, rsq, vlen);
    VFLOAT tmp2 = __riscv_vfmul(r_tail, coeff, vlen);
    tmp = __riscv_vfmacc(tmp, r_head, r, vlen);
    sin_corr = __riscv_vfmacc(sin_corr, tmp, tmp2, vlen);
    sin_corr = __riscv_vfmacc(sin_corr, r_delta, cos_hi, vlen);

    VFLOAT poly_s =
        PSTEP(0x1.1111111111069p-7, rsq,
              PSTEP(-0x1.a01a019ffe527p-13, rsq,
                    PSTEP(0x1.71de3a33a62c6p-19, rsq,
                          PSTEP(-0x1.ae642c52fc493p-26, rsq,
                                PSTEP(0x1.6109be886e15cp-33,
                                      -0x1.9ffe1dd295e78p-41, rsq, vlen),
                                vlen),
                          vlen),
                    vlen),
              vlen);

    VFLOAT poly_c =
        PSTEP(0x1.5555555555546p-5, rsq,
              PSTEP(-0x1.6c16c16c1450cp-10, rsq,
                    PSTEP(0x1.a01a019b77545p-16, rsq,
                          PSTEP(-0x1.27e4f72551e3dp-22, rsq,
                                PSTEP(0x1.1ee950032f74cp-29,
                                      -0x1.8f5dd75850673p-37, rsq, vlen),
                                vlen),
                          vlen),
                    vlen),
              vlen);

    VFLOAT r_to_4 = __riscv_vfmul(rsq, rsq, vlen);
    VFLOAT r_to_5 = __riscv_vfmul(r_to_4, r, vlen);

    poly_c = __riscv_vfmadd(poly_c, r_to_4, cos_corr, vlen);
    poly_s = __riscv_vfmadd(poly_s, r_to_5, sin_corr, vlen);

    VFLOAT S, s, C, c;
    FAST2SUM(sin_hi, poly_s, S, s, vlen);
    FAST2SUM(cos_hi, poly_c, C, c, vlen);

    VFLOAT numer_hi, numer_lo, denom_hi, denom_lo;
    numer_hi = S;
    numer_hi = __riscv_vmerge(numer_hi, C, numer_pick_c, vlen);
    numer_lo = s;
    numer_lo = __riscv_vmerge(numer_lo, c, numer_pick_c, vlen);

    denom_hi = S;
    denom_hi = __riscv_vmerge(denom_hi, C, denom_pick_c, vlen);
    denom_lo = s;
    denom_lo = __riscv_vmerge(denom_lo, c, denom_pick_c, vlen);

    DIV_N2D2(numer_hi, numer_lo, denom_hi, denom_lo, vy, vlen);

    // need to put the correct sign
    n = __riscv_vsll(n, BIT_WIDTH - 1, vlen);
    vy = __riscv_vfsgnjx(vy, I_AS_F(n), vlen);
    vy = __riscv_vfsgnjx(vy, vx_orig, vlen);

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
