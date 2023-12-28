// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <fenv.h>

#if defined(COMPILE_FOR_ATAN2)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ATAN2D_FIXEDPT
#else
#define F_VER1 RVVLM_ATAN2DI_FIXEDPT
#endif
#elif defined(COMPILE_FOR_ATAN2PI)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ATAN2PID_FIXEDPT
#else
#define F_VER1 RVVLM_ATAN2PIDI_FIXEDPT
#endif
#else
static_assert(false, "Must specify atan2 or atan2pi" __FILE__);
#endif

#define PI_075_HI 0x1.2d97c7f3321d2p+1
#define PI_075_LO 0x1.a79394c9e8a0ap-54
#define THREE_OV_4 0x1.8p-1

#define PIBY4_HI 0x1.921fb54442d18p-1
#define PIBY4_LO 0x1.1a62633145c07p-55
#define ONE_OV_4 0x1.0p-2

#define PIBY2_HI 0x1.921fb54442d18p+0
#define PIBY2_LO 0x1.1a62633145c07p-54
#define HALF 0x1.0p-1

#define PI_HI 0x1.921fb54442d18p+1
#define PI_LO 0x1.1a62633145c07p-53
#define ONE 0x1.0p0

#define PIBY2_Q61 0x3243f6a8885a308d
#define PI_Q61 0x6487ed5110b4611a
#define HALF_Q61 0x1000000000000000
#define ONE_Q61 0x2000000000000000

#define ONE_OV_PI_HI 0x1.45f306dc9c883p-2
#define ONE_OV_PI_LO -0x1.6b01ec5417056p-56

#define PIBY2_Q60 0x1921fb54442d1847
#define PI_Q60 0x3243f6a8885a308d
#define PIBY2_Q61 0x3243f6a8885a308d
#define ONE_OV_PI_Q63 0x28be60db9391054a

void F_VER1(API) {
  size_t vlen;
  VFLOAT vy, vx, vw, vw_special;
  VUINT vclass_y, vclass_x;
  UINT stencil, class_of_interest;
  VBOOL special_y, special_x, special_args, id_mask;
  UINT nb_special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  stencil = class_NaN | class_Inf | class_Zero;
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vy = VFLOAD_INARG1(vlen);
    vx = VFLOAD_INARG2(vlen);

    // Exception handling is more involved than other functions
    VFLOAT result_tmp;

    vclass_y = __riscv_vfclass(vy, vlen);
    IDENTIFY(vclass_y, stencil, special_y, vlen);
    vclass_x = __riscv_vfclass(vx, vlen);
    IDENTIFY(vclass_x, stencil, special_x, vlen);
    special_args = __riscv_vmor(special_y, special_x, vlen);
    nb_special_args = __riscv_vcpop(special_args, vlen);

    if (nb_special_args > 0) {
      // y or x is one of {NaN, +-Inf, +-0}
      class_of_interest = class_NaN;
      IDENTIFY(vclass_y, class_of_interest, special_y, vlen)
      IDENTIFY(vclass_x, class_of_interest, special_x, vlen)
      VBOOL y_notNaN = __riscv_vmnot(special_y, vlen);
      id_mask = __riscv_vmor(special_y, special_x, vlen);
      result_tmp = __riscv_vfadd(id_mask, vy, vx, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      VBOOL x_0, x_neg0, x_pos0;
      class_of_interest = class_posZero;
      IDENTIFY(vclass_x, class_of_interest, x_pos0, vlen);
      class_of_interest = class_negZero;
      IDENTIFY(vclass_x, class_of_interest, x_neg0, vlen);
      x_0 = __riscv_vmor(x_pos0, x_neg0, vlen);

      VBOOL y_0, y_not0;
      class_of_interest = class_Zero;
      IDENTIFY(vclass_y, class_of_interest, y_0, vlen);
      y_not0 = __riscv_vmnot(y_0, vlen);
      y_not0 = __riscv_vmand(y_not0, y_notNaN, vlen);
      id_mask = __riscv_vmand(x_0, y_not0, vlen);
#if defined(COMPILE_FOR_ATAN2)
      result_tmp = VFMV_VF(PIBY2_HI, vlen);
#else
      result_tmp = VFMV_VF(HALF, vlen);
#endif
      result_tmp = __riscv_vfsgnj(result_tmp, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      id_mask = __riscv_vmand(y_0, x_pos0, vlen);
      vw_special = __riscv_vmerge(vw_special, vy, id_mask, vlen);

      id_mask = __riscv_vmand(y_0, x_neg0, vlen);
#if defined(COMPILE_FOR_ATAN2)
      result_tmp = VFMV_VF(PI_HI, vlen);
#else
      result_tmp = VFMV_VF(ONE, vlen);
#endif
      result_tmp = __riscv_vfsgnj(result_tmp, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      VBOOL x_posInf, x_negInf, y_Inf, y_finite;
      class_of_interest = class_Inf;
      IDENTIFY(vclass_y, class_of_interest, y_Inf, vlen);
      y_finite = __riscv_vmandn(y_notNaN, y_Inf, vlen);
      x_posInf = __riscv_vmfeq(vx, fp_posInf, vlen);
      x_negInf = __riscv_vmfeq(vx, fp_negInf, vlen);

      id_mask = __riscv_vmand(x_posInf, y_Inf, vlen);
#if defined(COMPILE_FOR_ATAN2)
      result_tmp = VFMV_VF(PIBY4_HI, vlen);
#else
      result_tmp = VFMV_VF(ONE_OV_4, vlen);
#endif
      result_tmp = __riscv_vfsgnj(result_tmp, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      id_mask = __riscv_vmand(x_posInf, y_finite, vlen);
      result_tmp = VFMV_VF(fp_posZero, vlen);
      result_tmp = __riscv_vfsgnj(result_tmp, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      id_mask = __riscv_vmand(x_negInf, y_Inf, vlen);
#if defined(COMPILE_FOR_ATAN2)
      result_tmp = VFMV_VF(PI_075_HI, vlen);
#else
      result_tmp = VFMV_VF(THREE_OV_4, vlen);
#endif
      result_tmp = __riscv_vfsgnj(result_tmp, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      id_mask = __riscv_vmand(x_negInf, y_finite, vlen);
#if defined(COMPILE_FOR_ATAN2)
      result_tmp = VFMV_VF(PI_HI, vlen);
#else
      result_tmp = VFMV_VF(ONE, vlen);
#endif
      result_tmp = __riscv_vfsgnj(result_tmp, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      class_of_interest = class_finite_pos;
      VBOOL x_finite_pos;
      IDENTIFY(vclass_x, class_of_interest, x_finite_pos, vlen);
      id_mask = __riscv_vmand(x_finite_pos, y_0, vlen);
      vw_special = __riscv_vmerge(vw_special, vy, id_mask, vlen);

      class_of_interest = class_finite_neg;
      VBOOL x_finite_neg;
      IDENTIFY(vclass_x, class_of_interest, x_finite_neg, vlen);
      id_mask = __riscv_vmand(x_finite_neg, y_0, vlen);
#if defined(COMPILE_FOR_ATAN2)
      result_tmp = VFMV_VF(PI_HI, vlen);
#else
      result_tmp = VFMV_VF(ONE, vlen);
#endif
      result_tmp = __riscv_vfsgnj(result_tmp, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      id_mask = __riscv_vmor(x_finite_pos, x_finite_neg, vlen);
      id_mask = __riscv_vmand(id_mask, y_Inf, vlen);
#if defined(COMPILE_FOR_ATAN2)
      result_tmp = VFMV_VF(PIBY2_HI, vlen);
#else
      result_tmp = VFMV_VF(HALF, vlen);
#endif
      result_tmp = __riscv_vfsgnj(result_tmp, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, result_tmp, id_mask, vlen);

      vy = __riscv_vfmerge(vy, 0x1.0p-1, special_args, vlen);
      vx = __riscv_vfmerge(vx, 0x1.0p0, special_args, vlen);
    }

    // Other than the obvious exceptional cases that have been handled,
    // we filter out large differences in the exponents of x and y
    // to avoid spurious underflow being raised
    VUINT expo_y =
        __riscv_vand(__riscv_vsrl(F_AS_U(vy), MAN_LEN, vlen), 0x7FF, vlen);
    VUINT expo_x =
        __riscv_vand(__riscv_vsrl(F_AS_U(vx), MAN_LEN, vlen), 0x7FF, vlen);
    VINT exp_diff = __riscv_vsub(U_AS_I(expo_y), U_AS_I(expo_x), vlen);
    VBOOL exp_diff_large = __riscv_vmsge(exp_diff, 60, vlen);
    exp_diff_large =
        __riscv_vmor(exp_diff_large, __riscv_vmsle(exp_diff, -60, vlen), vlen);

    nb_special_args = __riscv_vcpop(exp_diff_large, vlen);
    special_args = __riscv_vmor(special_args, exp_diff_large, vlen);

    if (nb_special_args > 0) {
      VBOOL swap_yx = __riscv_vmsgtu(expo_y, expo_x, vlen);
      VBOOL x_neg = __riscv_vmslt(F_AS_I(vx), 0, vlen);

      VBOOL no_divide = __riscv_vmor(swap_yx, x_neg, vlen);
      no_divide = __riscv_vmand(no_divide, exp_diff_large, vlen);

      VBOOL divide = __riscv_vmnot(swap_yx, vlen);
      divide = __riscv_vmandn(divide, x_neg, vlen);
      divide = __riscv_vmand(divide, exp_diff_large, vlen);

      VFLOAT abs_y = __riscv_vfsgnj(vy, fp_posOne, vlen);
#if defined(COMPILE_FOR_ATAN2)
      VFLOAT tmp1 = __riscv_vfdiv(divide, abs_y, vx, vlen);
      tmp1 = __riscv_vfmerge(tmp1, 0x1.0p-60, no_divide, vlen);
      tmp1 = __riscv_vfsgnj(tmp1, vx, vlen);

      VFLOAT tmp2 = __riscv_vfsgnj(divide, tmp1, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, tmp2, divide, vlen);

      VBOOL use_piby2 = __riscv_vmand(swap_yx, exp_diff_large, vlen);
      tmp2 = __riscv_vfrsub(use_piby2, tmp1, PIBY2_HI, vlen);
      tmp2 = __riscv_vfsgnj(use_piby2, tmp2, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, tmp2, use_piby2, vlen);

      VBOOL use_pi = __riscv_vmandn(x_neg, swap_yx, vlen);
      use_pi = __riscv_vmand(use_pi, exp_diff_large, vlen);
      tmp2 = __riscv_vfadd(use_pi, tmp1, PI_HI, vlen);
      tmp2 = __riscv_vfsgnj(use_pi, tmp2, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, tmp2, use_pi, vlen);
#else
      // computing (1/pi) * (y/x) close to correctly rounded requires
      // careful error compensation. 1/(pi * x) can underflow and 1/y may
      // overflow, etc. Because exponent difference is large, so x is at least
      // 2^60 above denormal.
      VFLOAT tmp1 = __riscv_vfmul(divide, vx, 0x1.0p-55, vlen);
      VFLOAT tmp2 = __riscv_vfmul(divide, tmp1, PI_HI, vlen);
      VFLOAT tmp3 = __riscv_vfmsac(divide, tmp2, PI_HI, tmp1, vlen);
      tmp3 = __riscv_vfmacc(divide, tmp3, PI_LO, tmp1, vlen);
      // tmp2 + tmp3 is pi * (scaled_x) to extra precision
      VFLOAT R = __riscv_vfrdiv(divide, tmp2, fp_posOne, vlen);
      VFLOAT r = VFMV_VF(fp_posOne, vlen);
      r = __riscv_vfnmsac(divide, r, R, tmp2, vlen);
      r = __riscv_vfnmsac(divide, r, R, tmp3, vlen);
      r = __riscv_vfmul(divide, r, R, vlen);
      // R + r is 1/(pi * scaled_x) to extra precision
      // now compute vy * (R + r)
      tmp1 = __riscv_vfmul(divide, vy, 0x1.0p-55, vlen);
      tmp2 = __riscv_vfmul(divide, tmp1, r, vlen);
      tmp1 = __riscv_vfmadd(divide, tmp1, R, tmp2, vlen);
      tmp1 = __riscv_vfmerge(tmp1, 0x1.0p-60, no_divide, vlen);
      tmp1 = __riscv_vfsgnj(tmp1, vx, vlen);

      tmp2 = __riscv_vfsgnj(divide, tmp1, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, tmp2, divide, vlen);

      VBOOL use_half = __riscv_vmand(swap_yx, exp_diff_large, vlen);
      tmp2 = __riscv_vfrsub(use_half, tmp1, fp_posHalf, vlen);
      tmp2 = __riscv_vfsgnj(use_half, tmp2, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, tmp2, use_half, vlen);

      VBOOL use_one = __riscv_vmandn(x_neg, swap_yx, vlen);
      use_one = __riscv_vmand(use_one, exp_diff_large, vlen);
      tmp2 = __riscv_vfadd(use_one, tmp1, fp_posOne, vlen);
      tmp2 = __riscv_vfsgnj(use_one, tmp2, vy, vlen);
      vw_special = __riscv_vmerge(vw_special, tmp2, use_one, vlen);
#endif
      vy = __riscv_vfmerge(vy, fp_posZero, special_args, vlen);
      vx = __riscv_vfmerge(vx, 0x1.0p0, special_args, vlen);
    }

    // atan2(y, x) = sgn(y) * atan2(|y|, x)
    // Let z = min(|y|, |x|) / max(|y|, |x|)
    // If |y| >= |x|, then atan2(|y|, x) = pi/2 - sgn(x)*atan(z)
    // If |y| <  |x|, then atan2(|y|, x) = sgn(x)*atan(z) if x > 0;
    // otherwise it is pi + sgn(x)*atan(z).
    // And atan2pi(y, x) = atan2(y, x) / pi

    VFLOAT abs_y = __riscv_vfsgnj(vy, fp_posOne, vlen);
    VFLOAT abs_x = __riscv_vfsgnj(vx, fp_posOne, vlen);
    VBOOL swap_yx = __riscv_vmfge(abs_y, abs_x, vlen);
    VFLOAT numer = abs_y;
    VFLOAT denom = abs_x;
    numer = __riscv_vmerge(numer, abs_x, swap_yx, vlen);
    denom = __riscv_vmerge(denom, abs_y, swap_yx, vlen);
    numer = __riscv_vfsgnj(numer, vx, vlen);

    // Here |numer| <= denom and the exponent difference is within 60
    // We normalize them so that 1/denom will not overflow
    VUINT exp_normalize = __riscv_vsrl(F_AS_U(denom), 52, vlen);
    exp_normalize = __riscv_vmaxu(exp_normalize, 10, vlen);
    exp_normalize = __riscv_vminu(exp_normalize, 2036, vlen);
    exp_normalize = __riscv_vrsub(exp_normalize, 2046, vlen);
    VFLOAT scale_normalize = U_AS_F(__riscv_vsll(exp_normalize, 52, vlen));
    numer = __riscv_vfmul(numer, scale_normalize, vlen);
    denom = __riscv_vfmul(denom, scale_normalize, vlen);

    VFLOAT z = __riscv_vfdiv(numer, denom, vlen);
    VFLOAT delta = numer;
    delta = __riscv_vfnmsac(delta, z, denom, vlen);
    delta = __riscv_vfmul(delta, __riscv_vfrec7(numer, vlen), vlen);
    delta = __riscv_vfmul(delta, z, vlen);
    // z + delta is extra precise z.

    // Now convert z to fixed point.
    // We scale z by 61+m where 2^(-m) <= a < 2^(-m+1)
    // noting that m >= 0
    VUINT expo_61pm = __riscv_vsrl(F_AS_U(z), MAN_LEN, vlen);
    expo_61pm = __riscv_vand(expo_61pm, 0x7FF, vlen);
    expo_61pm = __riscv_vmaxu(expo_61pm, EXP_BIAS - 60, vlen);
    expo_61pm = __riscv_vrsub(expo_61pm, 2 * EXP_BIAS + 61, vlen); // BIAS+61+m

    VFLOAT scale_61pm = U_AS_F(__riscv_vsll(expo_61pm, MAN_LEN, vlen));
    VINT Z = __riscv_vfcvt_x(__riscv_vfmul(z, scale_61pm, vlen), vlen);
    VINT Delta = __riscv_vfcvt_x(__riscv_vfmul(delta, scale_61pm, vlen), vlen);
    Z = __riscv_vsadd(Z, Delta, vlen);

    VINT V = __riscv_vsmul(Z, __riscv_vsll(Z, 1, vlen), 1, vlen);
    // V is z*z with scale 60 + 2m
    VINT VV = __riscv_vrsub(V, 0, vlen);

    VUINT m = __riscv_vsub(expo_61pm, EXP_BIAS + 61, vlen);
    VUINT two_m = __riscv_vsll(m, 1, vlen);
    VBOOL left_shift = __riscv_vmsltu(two_m, 3, vlen);
    VBOOL right_shift = __riscv_vmnot(left_shift, vlen);

    VINT I_tmp =
        __riscv_vsll(left_shift, V, __riscv_vrsub(two_m, 2, vlen), vlen);
    V = __riscv_vmerge(V, I_tmp, left_shift, vlen);
    I_tmp = __riscv_vsll(left_shift, VV, __riscv_vrsub(two_m, 3, vlen), vlen);
    VV = __riscv_vmerge(VV, I_tmp, left_shift, vlen);

    I_tmp = __riscv_vsra(right_shift, V, __riscv_vsub(two_m, 2, vlen), vlen);
    V = __riscv_vmerge(V, I_tmp, right_shift, vlen);
    I_tmp = __riscv_vsra(right_shift, VV, __riscv_vsub(two_m, 3, vlen), vlen);
    VV = __riscv_vmerge(VV, I_tmp, right_shift, vlen);

    // V is z*z in scale 62, VV is -z*z in scale 63
    VINT WW = __riscv_vsll(__riscv_vsmul(V, VV, 1, vlen), 1, vlen);
    // WW is -z^4 in scale 63.

    VINT P_even =
        PSTEPN_I(-0x56629d839b68685, WW,
                 PSTEPN_I(-0x3d2984d0a6f836a, WW,
                          PSTEPN_I(-0x1c5e8b5228f9fe4, WW,
                                   PSTEPN_I(-0x05deca0ae3a1a5d,
                                            -0x004efe42fda24d7, WW, vlen),
                                   vlen),
                          vlen),
                 vlen);

    P_even = PSTEPN_I(-0x2aaaaaaaaaaa49d3, WW,
                      PSTEPN_I(-0x12492492378aaf69, WW,
                               PSTEPN_I(-0xba2e88c805cbaf8, WW,
                                        PSTEPN_I(-0x888722719d1260a, WW,
                                                 PSTEPN_I(-0x6b96ef57ce79cc3,
                                                          WW, P_even, vlen),
                                                 vlen),
                                        vlen),
                               vlen),
                      vlen);

    VINT P_odd =
        PSTEPN_I(0x04afe3b1345b489b, WW,
                 PSTEPN_I(0x02cec355111c7439, WW,
                          PSTEPN_I(0x00eaa9acebf3963e, WW,
                                   PSTEPN_I(0x001b053368ecfa14,
                                            0x00006da7bb4399dd, WW, vlen),
                                   vlen),
                          vlen),
                 vlen);

    P_odd = PSTEPN_I(0x1999999999540349, WW,
                     PSTEPN_I(0x0e38e38bf1671f42, WW,
                              PSTEPN_I(0x09d89b293ef5f4d9, WW,
                                       PSTEPN_I(0x0786ec3df324db61, WW,
                                                PSTEPN_I(0x060b457b3c56e750, WW,
                                                         P_odd, vlen),
                                                vlen),
                                       vlen),
                              vlen),
                     vlen);

    P_odd = __riscv_vsmul(VV, P_odd, 1, vlen);
    VINT P = __riscv_vsub(P_even, P_odd, vlen);

    P = __riscv_vsmul(VV, P, 1, vlen); // Q_63
    P = __riscv_vsmul(Z, P, 1, vlen);  // Q_61pm
    P = __riscv_vsub(Z, P, vlen);      // Q_61pm

    VBOOL xneg = __riscv_vmslt(F_AS_I(vx), 0, vlen);
    VBOOL xneg_or_swap = __riscv_vmor(xneg, swap_yx, vlen);
    VBOOL xneg_and_noswap = __riscv_vmandn(xneg, swap_yx, vlen);

    VINT P_tmp = __riscv_vsra(xneg_or_swap, P, m, vlen);
    P = __riscv_vmerge(P, P_tmp, xneg_or_swap, vlen);

#if defined(COMPILE_FOR_ATAN2PI)
    P = __riscv_vsmul(P, ONE_OV_PI_Q63, 1, vlen);
#endif

#if defined(COMPILE_FOR_ATAN2)
    P_tmp = __riscv_vrsub(swap_yx, P, PIBY2_Q61, vlen);
#else
    P_tmp = __riscv_vrsub(swap_yx, P, HALF_Q61, vlen);
#endif
    P = __riscv_vmerge(P, P_tmp, swap_yx, vlen);

#if defined(COMPILE_FOR_ATAN2)
    P_tmp = __riscv_vadd(xneg_and_noswap, P, PI_Q61, vlen);
#else
    P_tmp = __riscv_vadd(xneg_and_noswap, P, ONE_Q61, vlen);
#endif
    P = __riscv_vmerge(P, P_tmp, xneg_and_noswap, vlen);

    // we need to scale P by 2^(-(61+m)) or 2^(-61)
    VUINT expo_scale =
        __riscv_vrsub(expo_61pm, 2 * EXP_BIAS, vlen); // EXP_BIAS - (61+m)
    expo_scale = __riscv_vmerge(expo_scale, EXP_BIAS - 61, xneg_or_swap, vlen);
    VFLOAT scale_result = U_AS_F(__riscv_vsll(expo_scale, MAN_LEN, vlen));
    vw = __riscv_vfcvt_f(P, vlen);
    vw = __riscv_vfmul(vw, scale_result, vlen);
    vw = __riscv_vfsgnj(vw, vy, vlen);
    vw = __riscv_vmerge(vw, vw_special, special_args, vlen);

    // copy vw into outarg1 and increment addr pointers
    VFSTORE_OUTARG1(vw, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_INARG2(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
