// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <fenv.h>

#if defined(COMPILE_FOR_ASIN)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ASIND_FIXEDPT
#else
#define F_VER1 RVVLM_ASINDI_FIXEDPT
#endif
#elif defined(COMPILE_FOR_ASINPI)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ASINPID_FIXEDPT
#else
#define F_VER1 RVVLM_ASINPIDI_FIXEDPT
#endif
#elif defined(COMPILE_FOR_ACOS)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ACOSD_FIXEDPT
#else
#define F_VER1 RVVLM_ACOSDI_FIXEDPT
#endif
#elif defined(COMPILE_FOR_ACOSPI)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ACOSPID_FIXEDPT
#else
#define F_VER1 RVVLM_ACOSPIDI_FIXEDPT
#endif
#else
static_assert(false, "Must specify asin, acos, asinpi or acospi" __FILE__);
#endif

#define PIBY2_HI 0x1.921fb54442d18p+0
#define PIBY2_LO 0x1.1a62633145c07p-54

#define PI_HI 0x1.921fb54442d18p+1
#define PI_LO 0x1.1a62633145c07p-53

#define ONE_OV_PI_HI 0x1.45f306dc9c883p-2
#define ONE_OV_PI_LO -0x1.6b01ec5417056p-56

#define PIBY2_Q60 0x1921fb54442d1847
#define PI_Q60 0x3243f6a8885a308d
#define ONE_OV_PI_Q63 0x28be60db9391054a

#if defined(COMPILE_FOR_ASIN)
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfmadd((small_x), (vx), 0x1.0p-60, (vx), (vlen))
#elif defined(COMPILE_FOR_ASINPI)
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfmadd((small_x), (vx), ONE_OV_PI_HI,                                \
                 __riscv_vfmul((small_x), (vx), ONE_OV_PI_LO, (vlen)), (vlen))
#elif defined(COMPILE_FOR_ACOS)
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfadd((small_x), __riscv_vfrsub((small_x), (vx), PIBY2_LO, (vlen)),  \
                PIBY2_HI, (vlen))
#else
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfnmsub((small_x), (vx), ONE_OV_PI_HI, VFMV_VF(0x1.0p-1, (vlen)),    \
                  (vlen))
#endif

#if defined(COMPILE_FOR_ASIN)
#define FUNC_AT_ONE(abs_x_1, vx, vlen)                                         \
  __riscv_vfsgnj((abs_x_1), VFMV_VF(PIBY2_HI, (vlen)), (vx), (vlen))
#elif defined(COMPILE_FOR_ASINPI)
#define FUNC_AT_ONE(abs_x_1, vx, vlen)                                         \
  __riscv_vfsgnj((abs_x_1), VFMV_VF(0x1.0p-1, (vlen)), (vx), (vlen))
#elif defined(COMPILE_FOR_ACOS)
#define FUNC_AT_ONE(abs_x_1, vx, vlen)                                         \
  __riscv_vfadd(                                                               \
      (abs_x_1),                                                               \
      __riscv_vfsgnjn((abs_x_1), VFMV_VF(PIBY2_HI, (vlen)), (vx), (vlen)),     \
      PIBY2_HI, (vlen))
#else
#define FUNC_AT_ONE(abs_x_1, vx, vlen)                                         \
  __riscv_vfadd(                                                               \
      (abs_x_1),                                                               \
      __riscv_vfsgnjn((abs_x_1), VFMV_VF(0x1.0p-1, (vlen)), (vx), (vlen)),     \
      0x1.0p-1, (vlen))
#endif

#define EXCEPTION_HANDLING_ASINCOS(vx, special_args, vy_special, vlen)         \
  do {                                                                         \
    VUINT expo_x = __riscv_vsrl(F_AS_U(vx), MAN_LEN, (vlen));                  \
    expo_x = __riscv_vand(expo_x, 0x7FF, (vlen));                              \
    /* filter out |x| >= 1, Infs and NaNs */                                   \
    VBOOL expo_ge_BIAS = __riscv_vmsgeu(expo_x, EXP_BIAS, (vlen));             \
    /* filter out |x| < 2^(-30) */                                             \
    VBOOL expo_le_BIASm31 = __riscv_vmsleu(expo_x, EXP_BIAS - 31, (vlen));     \
    (special_args) = __riscv_vmor(expo_ge_BIAS, expo_le_BIASm31, (vlen));      \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      VFLOAT x_tmp = __riscv_vfsgnj((vx), fp_posOne, (vlen));                  \
      VBOOL abs_x_1 = __riscv_vmand(                                           \
          (special_args), __riscv_vmfeq(x_tmp, fp_posOne, (vlen)), (vlen));    \
      VBOOL abs_x_gt1 = __riscv_vmand(                                         \
          (special_args), __riscv_vmfgt(x_tmp, fp_posOne, (vlen)), (vlen));    \
      (vy_special) = vx;                                                       \
      /* Only replace extended real numbers x, |x| > 1; abs_x_gt1 is not true  \
       * if x is NaN */                                                        \
      x_tmp = __riscv_vfmerge(x_tmp, fp_sNaN, abs_x_gt1, (vlen));              \
      /* Here we add x to itself for all "special args" including NaNs,        \
       * generating the necessary signal */                                    \
      x_tmp = __riscv_vfadd((special_args), x_tmp, x_tmp, (vlen));             \
      (vy_special) =                                                           \
          __riscv_vmerge((vy_special), x_tmp, (special_args), (vlen));         \
      x_tmp = FUNC_AT_ONE(abs_x_1, (vx), (vlen));                              \
      (vy_special) = __riscv_vmerge((vy_special), x_tmp, abs_x_1, (vlen));     \
      x_tmp = FUNC_NEAR_ZERO(expo_le_BIASm31, vx, vlen);                       \
      (vy_special) =                                                           \
          __riscv_vmerge((vy_special), x_tmp, expo_le_BIASm31, (vlen));        \
      (vx) = __riscv_vfmerge((vx), fp_posZero, special_args, (vlen));          \
    }                                                                          \
  } while (0)

#include <fenv.h>

// For asin/acos, the computation is of the form  Const +/- (r + r*s*poly(s))
// This version computes this entire expression in fixed point by converting
// r and s into fixed point.
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx_orig, vx, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx_orig = VFLOAD_INARG1(vlen);

    EXCEPTION_HANDLING_ASINCOS(vx_orig, special_args, vy_special, vlen);
    vx = __riscv_vfsgnj(vx_orig, fp_posOne, vlen);

    // asin(x) ~=~ r + r*s*poly(s); r=x; s=r*r when |x| <= 1/2;
    // asin(x) = pi/2 - 2 * asin(sqrt((1-x)/2)) for x > 1/2;
    // acos(x) = pi/2 - asin(x) for |x| <= 1.
    // These expressions allow us to compute asin or acos with the essential
    // approximation of asin(x) for |x| <= 1/2

    VBOOL x_le_half = __riscv_vmfle(vx, 0x1.0p-1, vlen);
    VBOOL x_gt_half = __riscv_vmnot(x_le_half, vlen);
#if defined(COMPILE_FOR_ACOS) || defined(COMPILE_FOR_ACOSPI)
    VBOOL x_orig_le_half = __riscv_vmfle(vx_orig, 0x1.0p-1, vlen);
    VBOOL x_orig_lt_neghalf = __riscv_vmflt(vx_orig, 0x1.0p-1, vlen);
#endif
    VFLOAT alpha, beta;
    alpha = vx;
    beta = U_AS_F(__riscv_vxor(F_AS_U(beta), F_AS_U(beta), vlen));
    alpha = __riscv_vfmerge(alpha, -0x1.0p-1, x_gt_half, vlen);
    beta = __riscv_vfmerge(beta, 0x1.0p-1, x_gt_half, vlen);
    VFLOAT s = __riscv_vfmadd(alpha, vx, beta, vlen);
    // s is x*x or (1-x)/2
    double two_to_63 = 0x1.0p63;
    VINT S = __riscv_vfcvt_x(__riscv_vfmul(s, two_to_63, vlen), vlen);
    VINT Ssq = __riscv_vsmul(S, S, 1, vlen);

    // For x > 1/2, we need to compute sqrt(s) to be used later
    // where s = (1-x)/2. Note that s > 0 as we have handled |x| = 1 as special
    // arguments
    VFLOAT sqrt_s = __riscv_vfsqrt(x_gt_half, s, vlen);
    VFLOAT delta = __riscv_vfnmsub(x_gt_half, sqrt_s, sqrt_s, s, vlen);
    delta = __riscv_vfmul(x_gt_half, delta, __riscv_vfrec7(s, vlen), vlen);
    delta = __riscv_vfmul(x_gt_half, delta, sqrt_s, vlen);
    delta = __riscv_vfmul(x_gt_half, delta, 0x1.0p-1, vlen);

    VINT P_EVEN =
        PSTEP_I(0x15555555555390dd, Ssq,
                PSTEP_I(0x5b6db6d09b27a82, Ssq,
                        PSTEP_I(0x2dd13e6dd791f29, Ssq,
                                PSTEP_I(0x1c6fc7fedf424bb, Ssq,
                                        PSTEP_I(0xd5bd98b325786c,
                                                -0x21470ca28feec71, Ssq, vlen),
                                        vlen),
                                vlen),
                        vlen),
                vlen);

    VINT P_ODD =
        PSTEP_I(0x99999999b7428ad, Ssq,
                PSTEP_I(0x3e38e587fad54b2, Ssq,
                        PSTEP_I(0x238d7e0436a1c30, Ssq,
                                PSTEP_I(0x18ecc06c5a390e3, Ssq,
                                        PSTEP_I(0x28063c8b4b6a072,
                                                0x41646ebd6edd35e, Ssq, vlen),
                                        vlen),
                                vlen),
                        vlen),
                vlen);

    P_ODD = __riscv_vsmul(P_ODD, S, 1, vlen);
    VINT POLY = __riscv_vsadd(P_ODD, P_EVEN, vlen);
    POLY = __riscv_vsmul(POLY, S, 1, vlen);

    VFLOAT r = vx;
    r = __riscv_vmerge(r, sqrt_s, x_gt_half, vlen);
    delta = __riscv_vfmerge(delta, fp_posZero, x_le_half, vlen);

    VINT m = U_AS_I(
        __riscv_vrsub(__riscv_vsrl(F_AS_U(r), MAN_LEN, vlen), EXP_BIAS, vlen));
    m = __riscv_vmin(m, 60, vlen); // in case r is 0.0
    VINT q = __riscv_vadd(m, 60, vlen);
#if defined(COMPILE_FOR_ASIN) || defined(COMPILE_FOR_ASINPI)
    q = __riscv_vmerge(q, 60, x_gt_half, vlen);
#else
    q = __riscv_vmerge(q, 60, x_orig_le_half, vlen);
    r = __riscv_vfsgnjx(r, vx_orig, vlen);
    delta = __riscv_vfsgnjx(delta, vx_orig, vlen);
#endif
    VFLOAT scale_r = U_AS_F(
        __riscv_vsll(I_AS_U(__riscv_vadd(q, EXP_BIAS, vlen)), MAN_LEN, vlen));
    VINT R = __riscv_vfcvt_x(__riscv_vfmul(r, scale_r, vlen), vlen);
    R = __riscv_vsadd(
        __riscv_vfcvt_x(__riscv_vfmul(delta, scale_r, vlen), vlen), R, vlen);
    POLY = __riscv_vsadd(R, __riscv_vsmul(POLY, R, 1, vlen), vlen);
    VINT POLY_prime = __riscv_vsadd(x_gt_half, POLY, POLY, vlen);

#if defined(COMPILE_FOR_ASIN) || defined(COMPILE_FOR_ASINPI)
    POLY_prime = __riscv_vrsub(x_gt_half, POLY_prime, PIBY2_Q60, vlen);
#else
    POLY = __riscv_vrsub(x_le_half, POLY, 0, vlen);
#endif

    POLY = __riscv_vmerge(POLY, POLY_prime, x_gt_half, vlen);

#if defined(COMPILE_FOR_ACOS) || defined(COMPILE_FOR_ACOSPI)
    VINT C;
    C = __riscv_vxor(C, C, vlen);
    C = __riscv_vmerge(C, PI_Q60, x_orig_lt_neghalf, vlen);
    C = __riscv_vmerge(C, PIBY2_Q60, x_le_half, vlen);
    POLY = __riscv_vsadd(C, POLY, vlen);
#endif

#if defined(COMPILE_FOR_ASINPI) || defined(COMPILE_FOR_ACOSPI)
    POLY = __riscv_vsmul(POLY, ONE_OV_PI_Q63, 1, vlen);
#endif

    VFLOAT inv_scale_r = U_AS_F(
        __riscv_vsll(I_AS_U(__riscv_vrsub(q, EXP_BIAS, vlen)), MAN_LEN, vlen));
    vy = __riscv_vfmul(inv_scale_r, __riscv_vfcvt_f(POLY, vlen), vlen);

#if defined(COMPILE_FOR_ASIN) || defined(COMPILE_FOR_ASINPI)
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
