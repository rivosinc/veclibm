// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if defined(COMPILE_FOR_ATAN)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ATAND_FIXEDPT
#else
#define F_VER1 RVVLM_ATANDI_FIXEDPT
#endif
#elif defined(COMPILE_FOR_ATANPI)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ATANPID_FIXEDPT
#else
#define F_VER1 RVVLM_ATANPIDI_FIXEDPT
#endif
#else
static_assert(false, "Must specify atan or atanpi" __FILE__);
#endif

#define PIBY2_HI 0x1.921fb54442d18p+0
#define PIBY2_LO 0x1.1a62633145c07p-54

#define ONE_OV_PI_HI 0x1.45f306dc9c883p-2
#define ONE_OV_PI_LO -0x1.6b01ec5417056p-56

#define PIBY2_Q60 0x1921fb54442d1847
#define PI_Q60 0x3243f6a8885a308d
#define PIBY2_Q61 0x3243f6a8885a308d
#define ONE_OV_PI_Q63 0x28be60db9391054a

#if defined(COMPILE_FOR_ATAN)
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfnmsub((small_x), (vx), 0x1.0p-60, (vx), (vlen))
#else
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfmadd((small_x), (vx), ONE_OV_PI_HI,                                \
                 __riscv_vfmul((small_x), (vx), ONE_OV_PI_LO, (vlen)), (vlen))
#endif

#if defined(COMPILE_FOR_ATAN)
#define FUNC_EXPO_LARGE(expo_x_large, vx, vlen)                                \
  __riscv_vfsgnj(__riscv_vfadd((expo_x_large), VFMV_VF(PIBY2_HI, (vlen)),      \
                               PIBY2_LO, (vlen)),                              \
                 (vx), (vlen))
#else
#define FUNC_EXPO_LARGE(expo_x_large, vx, vlen)                                \
  __riscv_vfsub((expo_x_large),                                                \
                __riscv_vfsgnj(VFMV_VF(0x1.0p-1, (vlen)), (vx), (vlen)),       \
                __riscv_vfrec7((expo_x_large), (vx), (vlen)), (vlen))
#endif

#define EXCEPTION_HANDLING_ATAN(vx, special_args, vy_special, vlen)            \
  do {                                                                         \
    VUINT expo_x = __riscv_vsrl(F_AS_U(vx), MAN_LEN, (vlen));                  \
    expo_x = __riscv_vand(expo_x, 0x7FF, (vlen));                              \
    /* filter out |x| >= 2^60, Infs and NaNs */                                \
    VBOOL expo_x_large = __riscv_vmsgeu(expo_x, EXP_BIAS + 60, (vlen));        \
    /* filter out |x| < 2^(-30) */                                             \
    VBOOL x_small = __riscv_vmsleu(expo_x, EXP_BIAS - 31, (vlen));             \
    (special_args) = __riscv_vmor(expo_x_large, x_small, (vlen));              \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      VFLOAT x_tmp = FUNC_NEAR_ZERO(x_small, (vx), (vlen));                    \
      (vy_special) = __riscv_vmerge((vy_special), x_tmp, x_small, vlen);       \
      x_tmp = FUNC_EXPO_LARGE(expo_x_large, (vx), (vlen));                     \
      (vy_special) = __riscv_vmerge((vy_special), x_tmp, expo_x_large, vlen);  \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)

// For atan, atan(x) ~=~ r + r*s*poly(s), r = x and s = r*r for |x| < 1
// and atan(x) = pi/2 - atan(1/x) for |x| >= 1
// Thus atan(x) = (pi/2 or 0) +/- (r + r*s*poly(s)), where r is x or 1/x, s is
// r*r This version computes this entire expression in fixed point by converting
// r and s into fixed point.
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx_orig, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx_orig = VFLOAD_INARG1(vlen);

    EXCEPTION_HANDLING_ATAN(vx_orig, special_args, vy_special, vlen);

    // atan(-x) = -atan(x) and so we compute sign(x)*atan(|x|) to preserve
    // symmetry. For 0 <= t < 1, atan(t) is approximated by t + t^3*poly(t^2)
    // For 1 <= t < Inf, atan(t) = pi/2 - atan(1/t).
    // So the generic form of core is z + z^3 poly(z^2).
    // Because the series decays slowly and that the argument can be
    // as big as 1 in magnitude, rounding errors accumulation is significant
    // This version uses fixed point computation for the entire polynomial.

    VFLOAT a = __riscv_vfsgnj(vx_orig, fp_posOne, vlen);
    VBOOL a_ge_one = __riscv_vmfge(a, fp_posOne, vlen);
    VBOOL a_lt_one = __riscv_vmnot(a_ge_one, vlen);
    VFLOAT z = __riscv_vfrdiv(a_ge_one, a, fp_posOne, vlen);
    z = __riscv_vmerge(z, a, a_lt_one, vlen);
    // We need 1/a to extra precision.
    VFLOAT delta = VFMV_VF(fp_posOne, vlen);
    delta = __riscv_vfnmsac(a_ge_one, delta, z, a, vlen);
    delta = __riscv_vfmul(a_ge_one, delta, z, vlen);
    delta = __riscv_vfmerge(delta, fp_posZero, a_lt_one, vlen);
    // z + delta is extra precise z.

    // Now convert z to fixed point.
    // We scale z by 61+m where 2^(-m) <= a < 2^(-m+1)
    // noting that m >= 0
    VUINT expo_61pm = __riscv_vsrl(F_AS_U(z), MAN_LEN, vlen);
    expo_61pm = __riscv_vmaxu(expo_61pm, EXP_BIAS - 60, vlen);
    expo_61pm = __riscv_vrsub(expo_61pm, 2 * EXP_BIAS + 61, vlen); // BIAS+61+m
    VFLOAT scale_61pm = U_AS_F(__riscv_vsll(expo_61pm, MAN_LEN, vlen));
    VINT Z = __riscv_vfcvt_x(__riscv_vfmul(z, scale_61pm, vlen), vlen);
    VINT Delta = __riscv_vfcvt_x(__riscv_vfmul(delta, scale_61pm, vlen), vlen);
    Delta = __riscv_vsadd(a_ge_one, Delta, Z, vlen);
    Z = __riscv_vmerge(Z, Delta, a_ge_one, vlen);

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

    VINT P_a_ge_one = __riscv_vsra(a_ge_one, P, m, vlen);
    P_a_ge_one = __riscv_vrsub(P_a_ge_one, PIBY2_Q61, vlen);
    P = __riscv_vmerge(P, P_a_ge_one, a_ge_one, vlen);

#if defined(COMPILE_FOR_ATANPI)
    P = __riscv_vsmul(P, ONE_OV_PI_Q63, 1, vlen);
#endif

    // we need to scale P by 2^(-(61+m)) or 2^(-61)
    VUINT expo_scale =
        __riscv_vrsub(expo_61pm, 2 * EXP_BIAS, vlen); // EXP_BIAS - (61+m)
    expo_scale = __riscv_vmerge(expo_scale, EXP_BIAS - 61, a_ge_one, vlen);
    VFLOAT scale_result = U_AS_F(__riscv_vsll(expo_scale, MAN_LEN, vlen));
    vy = __riscv_vfcvt_f(P, vlen);
    vy = __riscv_vfmul(vy, scale_result, vlen);
    vy = __riscv_vfsgnj(vy, vx_orig, vlen);
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
