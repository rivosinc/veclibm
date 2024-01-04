// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#define LOG2_HI 0x1.62e42fefa4000p-1
#define LOG2_LO -0x1.8432a1b0e2634p-43
#define LOG2_BY2_HI 0x1.62e42fefa4000p-2
#define LOG2_BY2_LO -0x1.8432a1b0e2634p-44
#define ONE_Q60 0x1000000000000000

#if defined(COMPILE_FOR_ACOSH)
#define PLUS_MINUS_ONE -0x1.0p0
#else
#define PLUS_MINUS_ONE 0x1.0p0
#endif

#define EXCEPTION_HANDLING_ACOSH(vx, special_args, vy_special, vlen)           \
  do {                                                                         \
    VFLOAT vxm1 = __riscv_vfsub((vx), fp_posOne, (vlen));                      \
    VUINT vclass = __riscv_vfclass(vxm1, (vlen));                              \
    IDENTIFY(vclass, class_negative, (special_args), (vlen));                  \
    vxm1 = __riscv_vfmerge(vxm1, fp_sNaN, (special_args), (vlen));             \
    IDENTIFY(vclass, class_NaN | class_Inf | class_negative | class_Zero,      \
             (special_args), (vlen));                                          \
    if (__riscv_vcpop((special_args), (vlen)) > 0) {                           \
      (vy_special) = __riscv_vfmul((special_args), vxm1, vxm1, (vlen));        \
      (vx) = __riscv_vfmerge((vx), fp_posOne, (special_args), (vlen));         \
    }                                                                          \
  } while (0)

#define EXCEPTION_HANDLING_ASINH(vx, special_args, vy_special, vlen)           \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    VBOOL Inf_or_NaN_or_pm0;                                                   \
    IDENTIFY(vclass, class_NaN | class_Inf | class_Zero, Inf_or_NaN_or_pm0,    \
             (vlen));                                                          \
    VUINT expo_x = __riscv_vsrl(F_AS_U(vx), MAN_LEN, (vlen));                  \
    expo_x = __riscv_vand(expo_x, 0x7FF, (vlen));                              \
    VBOOL x_small = __riscv_vmsltu(expo_x, EXP_BIAS - 30, (vlen));             \
    (special_args) = __riscv_vmor(Inf_or_NaN_or_pm0, x_small, (vlen));         \
    if (__riscv_vcpop((special_args), (vlen)) > 0) {                           \
      VFLOAT tmp = __riscv_vfmadd(x_small, (vx), -0x1.0p-60, (vx), (vlen));    \
      (vy_special) = __riscv_vmerge((vy_special), tmp, x_small, (vlen));       \
      tmp = __riscv_vfadd(Inf_or_NaN_or_pm0, (vx), (vx), (vlen));              \
      (vy_special) =                                                           \
          __riscv_vmerge((vy_special), tmp, Inf_or_NaN_or_pm0, (vlen));        \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)

#define EXCEPTION_HANDLING_ATANH(vx, special_args, vy_special, vlen)           \
  do {                                                                         \
    VUINT expo_x = __riscv_vand(__riscv_vsrl(F_AS_U(vx), MAN_LEN, (vlen)),     \
                                0x7FF, (vlen));                                \
    VBOOL x_large = __riscv_vmsgeu(expo_x, EXP_BIAS, (vlen));                  \
    VBOOL x_small = __riscv_vmsltu(expo_x, EXP_BIAS - 30, (vlen));             \
    (special_args) = __riscv_vmor(x_large, x_small, (vlen));                   \
    if (__riscv_vcpop((special_args), (vlen)) > 0) {                           \
      VFLOAT abs_x = __riscv_vfsgnj((vx), fp_posOne, (vlen));                  \
      VBOOL x_gt_1 = __riscv_vmfgt(abs_x, fp_posOne, (vlen));                  \
      VBOOL x_eq_1 = __riscv_vmfeq(abs_x, fp_posOne, (vlen));                  \
      /* substitute |x| > 1 with sNaN */                                       \
      (vx) = __riscv_vfmerge((vx), fp_sNaN, x_gt_1, (vlen));                   \
      /* substitute |x| = 1 with +/-Inf and generate div-by-zero signal */     \
      VFLOAT tmp = VFMV_VF(fp_posZero, (vlen));                                \
      tmp = __riscv_vfsgnj(tmp, (vx), (vlen));                                 \
      tmp = __riscv_vfrec7(x_eq_1, tmp, (vlen));                               \
      (vy_special) = __riscv_vfadd((special_args), (vx), (vx), (vlen));        \
      (vy_special) = __riscv_vmerge((vy_special), tmp, x_eq_1, (vlen));        \
      tmp = __riscv_vfmadd(x_small, (vx), 0x1.0p-60, (vx), (vlen));            \
      (vy_special) = __riscv_vmerge((vy_special), tmp, x_small, (vlen));       \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)

// scale x down by 2^(-550) and set u to 0 if x >= 2^500
#define SCALE_X(vx, n, u, vlen)                                                \
  do {                                                                         \
    VUINT expo_x = __riscv_vsrl(F_AS_U((vx)), MAN_LEN, (vlen));                \
    VBOOL x_large = __riscv_vmsgeu(expo_x, EXP_BIAS + 500, (vlen));            \
    (n) = __riscv_vxor((n), (n), (vlen));                                      \
    (n) = __riscv_vmerge((n), 550, x_large, (vlen));                           \
    (u) = VFMV_VF(PLUS_MINUS_ONE, (vlen));                                     \
    (u) = __riscv_vfmerge((u), fp_posZero, x_large, (vlen));                   \
    (vx) = I_AS_F(                                                             \
        __riscv_vsub(F_AS_I(vx), __riscv_vsll((n), MAN_LEN, (vlen)), (vlen))); \
  } while (0)

// 2^(-50) <= X <= 2^500, u is -1 or 0
// If u is -1, 1 <= X < 2^500
#define XSQ_PLUS_U_ACOSH(vx, u, A, a, vlen)                                    \
  do {                                                                         \
    VFLOAT P, p;                                                               \
    PROD_X1Y1((vx), (vx), P, p, (vlen));                                       \
    VFLOAT tmp1, tmp2;                                                         \
    FAST2SUM(P, (u), tmp1, tmp2, (vlen));                                      \
    tmp2 = __riscv_vfadd(tmp2, p, (vlen));                                     \
    FAST2SUM(tmp1, tmp2, (A), (a), (vlen));                                    \
  } while (0)

#define XSQ_PLUS_U_ASINH(vx, u, A, a, vlen)                                    \
  do {                                                                         \
    VFLOAT P, p;                                                               \
    PROD_X1Y1((vx), (vx), P, p, (vlen));                                       \
    VFLOAT tmp1, tmp2;                                                         \
    POS2SUM(P, (u), tmp1, tmp2, (vlen));                                       \
    tmp2 = __riscv_vfadd(tmp2, p, (vlen));                                     \
    POS2SUM(tmp1, tmp2, (A), (a), (vlen));                                     \
  } while (0)

// scale x down by 2^(-550) and set u to 0 if x >= 2^500
#define SCALE_4_LOG(S, s, n, vlen)                                             \
  do {                                                                         \
    VINT expo_x = __riscv_vsra(F_AS_I((S)), MAN_LEN - 8, (vlen));              \
    expo_x = __riscv_vadd(expo_x, 0x96, (vlen));                               \
    expo_x = __riscv_vsra(expo_x, 8, (vlen));                                  \
    VINT n_adjust = __riscv_vsub(expo_x, EXP_BIAS, (vlen));                    \
    (n) = __riscv_vadd((n), n_adjust, (vlen));                                 \
    expo_x = __riscv_vsll(__riscv_vrsub(expo_x, 2 * EXP_BIAS, (vlen)),         \
                          MAN_LEN, (vlen));                                    \
    (S) = I_AS_F(__riscv_vsub(                                                 \
        F_AS_I((S)), __riscv_vsll(n_adjust, MAN_LEN, (vlen)), (vlen)));        \
    (s) = __riscv_vfmul((s), I_AS_F(expo_x), (vlen));                          \
  } while (0)

#define TRANSFORM_2_ATANH(S, s, numer, delta_numer, denom, delta_denom, vlen)  \
  do {                                                                         \
    VFLOAT S_tmp = __riscv_vfsub((S), fp_posOne, (vlen));                      \
    FAST2SUM(S_tmp, (s), (numer), (delta_numer), (vlen));                      \
    (numer) = __riscv_vfadd((numer), (numer), (vlen));                         \
    (delta_numer) = __riscv_vfadd((delta_numer), (delta_numer), (vlen));       \
    S_tmp = VFMV_VF(fp_posOne, (vlen));                                        \
    FAST2SUM(S_tmp, (S), (denom), (delta_denom), (vlen));                      \
    (delta_denom) = __riscv_vfadd((delta_denom), (s), (vlen));                 \
  } while (0)

#define LOG_POLY(r, r_lo, poly, vlen)                                          \
  do {                                                                         \
    VFLOAT rsq = __riscv_vfmul((r), (r), (vlen));                              \
    VFLOAT rcube = __riscv_vfmul(rsq, (r), (vlen));                            \
    VFLOAT r6 = __riscv_vfmul(rcube, rcube, (vlen));                           \
    VFLOAT poly_right = PSTEP(0x1.c71c543983a27p-12, rsq,                      \
                              PSTEP(0x1.7465c27ee47d0p-14, rsq,                \
                                    PSTEP(0x1.39af2e90a6554p-16,               \
                                          0x1.2e74f2255e096p-18, rsq, (vlen)), \
                                    (vlen)),                                   \
                              (vlen));                                         \
    VFLOAT poly_left =                                                         \
        PSTEP(0x1.555555555558cp-4, rsq,                                       \
              PSTEP(0x1.9999999982550p-7, 0x1.2492493f7cc71p-9, rsq, (vlen)),  \
              (vlen));                                                         \
    (poly) = __riscv_vfmadd(poly_right, r6, poly_left, (vlen));                \
    (poly) = __riscv_vfmadd(poly, rcube, (r_lo), (vlen));                      \
  } while (0)
