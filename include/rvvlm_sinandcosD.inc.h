// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "rvvlm_trigD.h"

#if defined(COMPILE_FOR_SINCOS)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_SINCOSD_STD
#else
#define F_VER1 RVVLM_SINCOSDI_STD
#endif
#elif defined(COMPILE_FOR_SINCOSPI)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_SINCOSPID_STD
#else
#define F_VER1 RVVLM_SINCOSPIDI_STD
#endif
#else
static_assert(false, "Must specify sincos or sincospi" __FILE__);
#endif

// This versions reduces argument to [-pi/4, pi/4] and computes sin(r) and
// cos(r)
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx_orig, vx, vy, vy_special, vz, vz_special;
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
    EXCEPTION_HANDLING_SINCOS(vx_orig, expo_x, special_args, vy_special,
                              vz_special, vlen);

#if defined(COMPILE_FOR_SINCOS)
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
    VBOOL sin_use_c = __riscv_vmsne(n_lsb, 0, vlen);

#if defined(COMPILE_FOR_SINCOSPI)
    VBOOL sinX_zero = __riscv_vmandn(x_is_n_piby2, sin_use_c, vlen);

    VBOOL cosX_zero = __riscv_vmand(x_is_n_piby2, sin_use_c, vlen);
#endif

    // We compute both sin(r) and cos(r)
    // Note that sin(t) = t + t^3(s_poly(t^2))
    // and       cos(t) = 1 - t^2/2 + t^4(c_poly(t^2))
    // where s_poly and c_poly are of the same degree.
    //
    // sin(r+r_delta) ~=~ sin(r) + r_delta(1 - r^2/2)
    // sin(r) is approximated by 7 terms, starting from x, x^3, ..., x^13
    // cos(r+r_delta) ~=~ cos(r) - r * r_delta
    //
    VFLOAT rsq, rcube, s_corr, c_corr, r_prime, One, C;
    One = VFMV_VF(fp_posOne, vlen);
    rsq = __riscv_vfmul(r, r, vlen);
    rcube = __riscv_vfmul(rsq, r, vlen);

    r_prime = __riscv_vfmul(r, -0x1.0p-1, vlen);
    C = __riscv_vfmacc(One, r_prime, r, vlen);
    s_corr = __riscv_vfmul(r_delta, C, vlen);

    c_corr = __riscv_vfsub(One, C, vlen);
    c_corr = __riscv_vfmacc(c_corr, r, r_prime, vlen);
    c_corr = __riscv_vfnmsac(c_corr, r, r_delta, vlen);

    VFLOAT poly_sin =
        PSTEP(-0x1.5555555555548p-3, rsq,
              PSTEP(0x1.111111110f730p-7, rsq,
                    PSTEP(-0x1.a01a019be932ap-13, rsq,
                          PSTEP(0x1.71de35553ddb6p-19, rsq,
                                PSTEP(-0x1.ae5e4b94836f8p-26,
                                      0x1.5d8b5ae12066ap-33, rsq, vlen),
                                vlen),
                          vlen),
                    vlen),
              vlen);

    VFLOAT poly_cos =
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

    VFLOAT r_4 = __riscv_vfmul(rsq, rsq, vlen);

    poly_sin = __riscv_vfmadd(poly_sin, rcube, s_corr, vlen);
    poly_sin = __riscv_vfadd(r, poly_sin, vlen);

    poly_cos = __riscv_vfmadd(poly_cos, r_4, c_corr, vlen);
    poly_cos = __riscv_vfadd(C, poly_cos, vlen);

    vy = __riscv_vmerge(poly_sin, poly_cos, sin_use_c, vlen);
    vz = __riscv_vmerge(poly_cos, poly_sin, sin_use_c, vlen);

    n = __riscv_vsll(n, BIT_WIDTH - 2, vlen);
    vy = __riscv_vfsgnjx(vy, I_AS_F(n), vlen);
    vz = __riscv_vfsgnjx(vz, I_AS_F(n), vlen);

    n_lsb = __riscv_vsll(n_lsb, 63, vlen);
    vz = __riscv_vfsgnjx(vz, U_AS_F(n_lsb), vlen);

#if defined(COMPILE_FOR_SINCOSPI)
    vy = __riscv_vmerge(vy, VFMV_VF(fp_posZero, vlen), sinX_zero, vlen);
    vz = __riscv_vmerge(vz, VFMV_VF(fp_posZero, vlen), cosX_zero, vlen);
#endif
    vy = __riscv_vfsgnjx(vy, vx_orig, vlen);
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);
    vz = __riscv_vmerge(vz, vz_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);
    VFSTORE_OUTARG2(vz, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
    INCREMENT_OUTARG2(vlen);
  }
  RESTORE_FRM;
}
