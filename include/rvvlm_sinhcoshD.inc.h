// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "rvvlm_hyperbolicsD.h"

#include <fenv.h>

#if defined(COMPILE_FOR_SINH)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_SINHD_STD
#define F_VER2 RVVLM_SINHD_MIXED
#else
#define F_VER1 RVVLM_SINHDI_STD
#define F_VER2 RVVLM_SINHDI_MIXED
#endif
#elif defined(COMPILE_FOR_COSH)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_COSHD_STD
#define F_VER2 RVVLM_COSHD_MIXED
#else
#define F_VER1 RVVLM_COSHDI_STD
#define F_VER2 RVVLM_COSHDI_MIXED
#endif
#else
static_assert(false, "Must specify sinh or cosh" __FILE__);
#endif

#include <fenv.h>

// This versions reduces argument to [-log2/2, log2/2]
// Exploit common expressions exp(R) and exp(-R), and uses purely
// floating point method to preserve precision
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
    expo_x =
        __riscv_vand(__riscv_vsrl(F_AS_U(vx_orig), MAN_LEN, vlen), 0x7FF, vlen);

    // Set results for input of NaN and Inf and also for |x| very small
    EXCEPTION_HANDLING_HYPER(vx_orig, expo_x, special_args, vy_special, vlen);

    // Both sinh and cosh have sign symmetry; suffices to work on |x|.
    // For sinh(x) = sign(x) * sinh(|x|) and cosh(x) = cosh(|x|).
    vx = __riscv_vfsgnj(vx_orig, fp_posOne, vlen);

    // Suffices to clip |x| to 714.0, which is bigger than 1030 log(2)
    vx = __riscv_vfmin(vx, 0x1.65p9, vlen);
    VINT n;
    VFLOAT r, r_delta;
    ARGUMENT_REDUCTION(vx, n, r, r_delta, vlen);

    // At this point exp(x) = 2^n exp(r'), where r' = r + delta_r
    // sinh(x) or cosh(x) is 2^(n-1) ( exp(r') -/+ 2^(-2n) exp(-r') )
    // Note that n >= 0. Moreover, the factor 2^(-2n) can be replaced by
    // s = 2^(-m), m = min(2n, 60)
    // sinh(x) / cosh(x) = 2^(n-1)(exp(r') -/+ s exp(-r'))

    // exp(r') and exp(-r') will be computed purely in floating point
    // using extra-precision simulation when needed
    // Note exp(t) is approximated by
    //       1 + t + t^2/2 + t^3(p_even(t^2) + t*p_odd(t^2))
    // and thus exp(-t) is approximated
    //       1 - t + t^2/2 - t^3(p_even(t^2) - t*p_odd(t^2))
    // So we compute the common expressions p_even and p_odd separately.
    // Moreover, they can be evaluated as r*r alone, not needing r_delta
    // because they are at least a factor of (log(2)/2)^2/6 smaller than the
    // final result of interest.

    VFLOAT rsq = __riscv_vfmul(r, r, vlen);
    VFLOAT rcube = __riscv_vfmul(rsq, r, vlen);

    VFLOAT p_even = PSTEP(0x1.555555555555ap-3, rsq,
                          PSTEP(0x1.111111110ef6ap-7, rsq,
                                PSTEP(0x1.a01a01b32b633p-13, rsq,
                                      PSTEP(0x1.71ddef82f4beep-19,
                                            0x1.af6eacd796f0bp-26, rsq, vlen),
                                      vlen),
                                vlen),
                          vlen);

    VFLOAT p_odd = PSTEP(
        0x1.5555555553aefp-5, rsq,
        PSTEP(0x1.6c16c17a09506p-10, rsq,
              PSTEP(0x1.a019b37a2b3dfp-16, 0x1.289788d8bdadfp-22, rsq, vlen),
              vlen),
        vlen);

    VFLOAT p_pos = __riscv_vfmadd(p_odd, r, p_even, vlen);
    VFLOAT p_neg = __riscv_vfnmsub(p_odd, r, p_even, vlen);
    p_pos = __riscv_vfmul(p_pos, rcube, vlen);
    p_neg = __riscv_vfmul(p_neg, rcube, vlen);

    // exp( r') is approximated by 1 + r' + (r')^2/2 + p_pos
    // exp(-r') is approximated by 1 - r' + (r')^2/2 - p_neg

    VINT m = __riscv_vmin(__riscv_vadd(n, n, vlen), 60, vlen);
    VFLOAT s = U_AS_F(
        __riscv_vsll(I_AS_U(__riscv_vrsub(m, EXP_BIAS, vlen)), MAN_LEN, vlen));
#if defined(COMPILE_FOR_SINH)
    VFLOAT poly = __riscv_vfmacc(p_pos, s, p_neg, vlen);
#else
    VFLOAT poly = __riscv_vfnmsac(p_pos, s, p_neg, vlen);
#endif
    // sinh / cosh = (1 -/+ s) + ([r' + (r'2)^2/2] +/- s [r' - (r')^2/2]) + poly
    // We need r' +/- (r')^2/2 and their sum/diff to high precision
    // and 1 -/+ s to high precision
    VFLOAT r_half = __riscv_vfmul(r, 0x1.0p-1, vlen);
    VFLOAT B_plus = __riscv_vfmadd(r, r_half, r, vlen);
    VFLOAT b_plus =
        __riscv_vfmacc(__riscv_vfsub(r, B_plus, vlen), r, r_half, vlen);
    VFLOAT delta_b_plus = __riscv_vfmadd(r, r_delta, r_delta, vlen);
    b_plus = __riscv_vfadd(b_plus, delta_b_plus, vlen);
    VFLOAT B_minus = __riscv_vfnmsub(r, r_half, r, vlen);
    VFLOAT b_minus =
        __riscv_vfnmsac(__riscv_vfsub(r, B_minus, vlen), r, r_half, vlen);
    VFLOAT delta_b_minus = __riscv_vfnmsub(r, r_delta, r_delta, vlen);
    b_minus = __riscv_vfadd(b_minus, delta_b_minus, vlen);
#if defined(COMPILE_FOR_SINH)
    VFLOAT B = __riscv_vfmadd(B_minus, s, B_plus, vlen);
    VFLOAT b = __riscv_vfmacc(__riscv_vfsub(B_plus, B, vlen), s, B_minus, vlen);
    b = __riscv_vfadd(b, __riscv_vfmadd(b_minus, s, b_plus, vlen), vlen);
#else
    VFLOAT B = __riscv_vfnmsub(B_minus, s, B_plus, vlen);
    VFLOAT b =
        __riscv_vfnmsac(__riscv_vfsub(B_plus, B, vlen), s, B_minus, vlen);
    b = __riscv_vfadd(b, __riscv_vfnmsub(b_minus, s, b_plus, vlen), vlen);
#endif
    VBOOL n_large = __riscv_vmsge(n, 50, vlen);
    VFLOAT s_hi = s;
    VFLOAT s_lo;
    s_lo = U_AS_F(__riscv_vxor(F_AS_U(s_lo), F_AS_U(s_lo), vlen));
    s_hi = __riscv_vfmerge(s_hi, fp_posZero, n_large, vlen);
    s_lo = __riscv_vmerge(s_lo, s, n_large, vlen);
#if defined(COMPILE_FOR_SINH)
    VFLOAT A = __riscv_vfrsub(s_hi, fp_posOne, vlen);
    s_lo = __riscv_vfsgnjn(s_lo, s_lo, vlen);
#else
    VFLOAT A = __riscv_vfadd(s_hi, fp_posOne, vlen);
#endif
    b = __riscv_vfadd(b, s_lo, vlen);
    VFLOAT Z_hi, Z_lo;
    FAST2SUM(B, poly, Z_hi, Z_lo, vlen);
    b = __riscv_vfadd(b, Z_lo, vlen);
    B = Z_hi;
    FAST2SUM(A, B, Z_hi, Z_lo, vlen);
    b = __riscv_vfadd(b, Z_lo, vlen);
    vy = __riscv_vfadd(Z_hi, b, vlen);

    // scale vy by 2^(n-1)
    n = __riscv_vsub(n, 1, vlen);
    FAST_LDEXP(vy, n, vlen);

#if defined(COMPILE_FOR_SINH)
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

// This versions reduces argument to [-log2/2, log2/2]
// Exploit common expressions exp(R) and exp(-R), and uses both
// floating point and fixed point for core approximation and reconstruction
void F_VER2(API) {
  size_t vlen;
  VFLOAT vx_orig, vx, vy, vy_special;
  VBOOL special_args;
  VUINT expo_x;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx_orig = VFLOAD_INARG1(vlen);
    expo_x =
        __riscv_vand(__riscv_vsrl(F_AS_U(vx_orig), MAN_LEN, vlen), 0x7FF, vlen);

    // Set results for input of NaN and Inf and also for |x| very small
    EXCEPTION_HANDLING_HYPER(vx_orig, expo_x, special_args, vy_special, vlen);

    // Both sinh and cosh have sign symmetry; suffices to work on |x|.
    // For sinh(x) = sign(x) * sinh(|x|) and cosh(x) = cosh(|x|).
    vx = __riscv_vfsgnj(vx_orig, fp_posOne, vlen);

    // Suffices to clip |x| to 714.0, which is bigger than 1030 log(2)
    vx = __riscv_vfmin(vx, 0x1.65p9, vlen);
    VINT n;
    VFLOAT r, r_delta;
    ARGUMENT_REDUCTION(vx, n, r, r_delta, vlen);

    // At this point exp(x) = 2^n exp(r'), where r' = r + delta_r
    // sinh(x) or cosh(x) is 2^(n-1) ( exp(r') -/+ 2^(-2n) exp(-r') )
    // Note that n >= 0. Moreover, the factor 2^(-2n) can be replaced by
    // s = 2^(-m), m = min(2n, 60)
    // sinh(x) / cosh(x) = 2^(n-1)(exp(r') -/+ s exp(-r'))

    // exp(r') and exp(r') will be computed with a mixture of floating-point
    // and fixed-point arithmetic.

    // Here we first create the factor s = 2^(-m) and r' in fixed point.
    // The factor s is scaled by 2^61; while r' is scaled differently for sinh
    // and cosh
    VINT m = __riscv_vmin(__riscv_vsll(n, 1, vlen), 60, vlen);
    VINT s_fix = VMVI_VX(1, vlen);
    s_fix = __riscv_vsll(s_fix, I_AS_U(__riscv_vrsub(m, 61, vlen)), vlen);
    // s_fix is s in fixed point, scale factor 61, aka Q_61

#if defined(COMPILE_FOR_SINH)
    // For 2^(-k) <= |r| < 2^(-k+1), we create fixed point of r in Q_(60+k)
    // format
    VINT j = __riscv_vand(__riscv_vsra(F_AS_I(r), MAN_LEN, vlen), 0x7FF, vlen);
    j = __riscv_vmax(j, EXP_BIAS - 60, vlen);
    VINT k = __riscv_vrsub(j, EXP_BIAS, vlen); // k >= 2
    VFLOAT r_scale = I_AS_F(
        __riscv_vsll(__riscv_vrsub(j, 2 * EXP_BIAS + 60, vlen), MAN_LEN, vlen));
#else
    VFLOAT r_scale = I_AS_F(VMVI_VX(1023 + 61, vlen));
    r_scale = I_AS_F(__riscv_vsll(F_AS_I(r_scale), 52, vlen));
#endif
    VINT R = __riscv_vfcvt_x(__riscv_vfmul(r, r_scale, vlen), vlen);
    R = __riscv_vsadd(
        R, __riscv_vfcvt_x(__riscv_vfmul(r_delta, r_scale, vlen), vlen), vlen);

    // Note exp(t) is approximated by
    //       1 + t + t^2/2 + t^3(p_even(t^2) + t*p_odd(t^2))
    // and thus exp(-t) is approximated
    //       1 - t + t^2/2 - t^3(p_even(t^2) - t*p_odd(t^2))
    // So we compute the common expressions p_even and p_odd separately.
    // Moreover, they can be evaluated as r*r alone, not needing r_delta
    // because they are at least a factor of (log(2)/2)^2/6 smaller than the
    // final result of interest.

    VFLOAT rsq = __riscv_vfmul(r, r, vlen);
    VFLOAT rcube = __riscv_vfmul(rsq, r, vlen);

    VFLOAT p_even = PSTEP(0x1.555555555555ap-3, rsq,
                          PSTEP(0x1.111111110ef6ap-7, rsq,
                                PSTEP(0x1.a01a01b32b633p-13, rsq,
                                      PSTEP(0x1.71ddef82f4beep-19,
                                            0x1.af6eacd796f0bp-26, rsq, vlen),
                                      vlen),
                                vlen),
                          vlen);

    VFLOAT p_odd = PSTEP(
        0x1.5555555553aefp-5, rsq,
        PSTEP(0x1.6c16c17a09506p-10, rsq,
              PSTEP(0x1.a019b37a2b3dfp-16, 0x1.289788d8bdadfp-22, rsq, vlen),
              vlen),
        vlen);

    VFLOAT p_pos = __riscv_vfmadd(p_odd, r, p_even, vlen);
    VFLOAT p_neg = __riscv_vfnmsub(p_odd, r, p_even, vlen);
    p_pos = __riscv_vfmul(p_pos, rcube, vlen);
    p_neg = __riscv_vfmul(p_neg, rcube, vlen);

    // exp( r') is approximated by 1 + r' + (r')^2/2 + p_pos
    // exp(-r') is approximated by 1 - r' + (r')^2/2 - p_neg
    // convert p_pos and p_neg into fixed point, scale by r_scale

    VINT P_pos = __riscv_vfcvt_x(__riscv_vfmul(p_pos, r_scale, vlen), vlen);
    VINT P_neg = __riscv_vfcvt_x(__riscv_vfmul(p_neg, r_scale, vlen), vlen);
    P_neg = __riscv_vsra(P_neg, I_AS_U(m), vlen);
#if defined(COMPILE_FOR_SINH)
    VINT P = __riscv_vsadd(P_pos, P_neg, vlen);
#else
    VINT P = __riscv_vssub(P_pos, P_neg, vlen);
#endif

    VINT Rsq = __riscv_vsmul(R, R, 1, vlen);
#if defined(COMPILE_FOR_SINH)
    // R is Q_60+k; so Rsq is r^2 Q_60+2k-3, where k >= 2
    // or r^2/2 in Q_60+2k-2; so we right shift by k-2 to make it Q_60+k
    // Note that k >= 2, so k - 2 >= 0
    VINT Rsq_by_2 = __riscv_vsra(Rsq, I_AS_U(__riscv_vsub(k, 2, vlen)), vlen);
    // Rsq_by_2 is now r^2/2 in Q_61+k
    VINT Q_pos = __riscv_vsadd(R, Rsq_by_2, vlen);
    VINT Q_neg = __riscv_vssub(R, Rsq_by_2, vlen);
    VINT B = __riscv_vsra(Q_neg, I_AS_U(m), vlen);
    B = __riscv_vsadd(B, Q_pos, vlen);
    B = __riscv_vsadd(B, P, vlen);

    VINT A = __riscv_vsll(VMVI_VX(1, vlen), 61, vlen);
    A = __riscv_vsub(A, s_fix, vlen);
    VBOOL n_is_0 = __riscv_vmseq(n, 0, vlen);
    VBOOL n_not_0 = __riscv_vmnot(n_is_0, vlen);
    VUINT shift_B = I_AS_U(__riscv_vsub(k, 1, vlen));
    shift_B = __riscv_vmerge(shift_B, 0, n_is_0, vlen);
    B = __riscv_vsra(B, shift_B, vlen);
    A = __riscv_vsadd(A, B, vlen);
    vy = __riscv_vfcvt_f(A, vlen);

    // result is 2^(-(n-1)) sinh(x) * scale
    // Scale of A is either 61 or 60+k
    VINT scale_result = __riscv_vadd(k, 60, vlen); // 60+k
    scale_result =
        __riscv_vmerge(scale_result, 61, n_not_0, vlen); // 61 or 60+k
    scale_result = __riscv_vsub(n, scale_result, vlen);
    scale_result = __riscv_vsub(scale_result, 1, vlen);
#else
    // R is Q_61; so Rsq is r^2 Q_59 or r^2/2 in Q_60,
    // so we left shift by 1 to make it Q_61
    VINT Rsq_by_2 = __riscv_vsll(Rsq, 1, vlen);
    // Rsq_by_2 is now r^2/2 in Q_61
    VINT Q_pos = __riscv_vsadd(Rsq_by_2, R, vlen);
    VINT Q_neg = __riscv_vssub(Rsq_by_2, R, vlen);
    VINT B = __riscv_vsra(Q_neg, I_AS_U(m), vlen);
    B = __riscv_vsadd(B, Q_pos, vlen);
    B = __riscv_vsadd(B, P, vlen);

    VINT A = __riscv_vsll(VMVI_VX(1, vlen), 61, vlen);
    A = __riscv_vsadd(A, s_fix, vlen);
    A = __riscv_vsadd(A, B, vlen);
    vy = __riscv_vfcvt_f(A, vlen);

    // result is 2^(-(n-1)) cosh(x_clip) * scale
    // Scale of A is 61
    // So we need to scale A by n-62
    VINT scale_result = __riscv_vsub(n, 62, vlen);
#endif

    // scale vy by 2^scale_result
    FAST_LDEXP(vy, scale_result, vlen);

#if defined(COMPILE_FOR_SINH)
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
