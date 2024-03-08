// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#define RTPI_BY2_HI 0x1.c5bf891b4ef6ap-1
#define RTPI_BY2_LO 0x1.4f38760a41abbp-54
#define NEG_LOG2_HI -0x1.62e42fefa4000p-1
#define NEG_LOG2_LO 0x1.8432a1b0e2634p-43

// P_coefficients in asending order, in varying scales. p0_delta is in floating
// point
#define P_tiny_0 -0x8593442e139d           // scale 66
#define P_tiny_1 -0x1fcf7055ac5f03         // scale 64
#define P_tiny_2 -0x106dde33d8dc179        // scale 61
#define P_tiny_3 -0xc31d6e09935118a        // scale 60
#define P_tiny_4 -0x3560de73cb5bbcc0       // scale 59
#define P_tiny_5 -0x1eb4c7e14b254de8       // scale 57
#define P_tiny_6 0x1fdf5a9d23430bd7        // scale 56
#define P_tiny_7 0x62c4020631de121b        // scale 56
#define P_tiny_8 0x4be1ed5d031773f1        // scale 58
#define P_tiny_9 0x55a9f8b9538981a1        // scale 60
#define DELTA_P0_tiny 0x1.ba4d0b79d16e6p-2 // scale 66

// Q_coefficients in asending order, in varying scales. q0_delta is in floating
// point
#define Q_tiny_0 -0x85933cda2d6d           // sacle 66
#define Q_tiny_1 -0x1fcf792da7d51d         // sacle 64
#define Q_tiny_2 -0x106ec6e0ed13ae1        // sacle 61
#define Q_tiny_3 -0x61b925a39a461aa        // sacle 59
#define Q_tiny_4 -0x35ebf9dc72fab062       // sacle 59
#define Q_tiny_5 -0x2131cf7760e82873       // sacle 57
#define Q_tiny_6 0x1860ae67db2a6609        // sacle 56
#define Q_tiny_7 0x5e9a123701d89289        // sacle 56
#define Q_tiny_8 0x417b35aab14ac49d        // sacle 56
#define Q_tiny_9 0x5e4a26a7c1415755        // sacle 57
#define DELTA_Q0_tiny 0x1.8a7adad44d65ap-4 // scale 66

// erfinv(+-1) = +-Inf with divide by zero
// erfinv(x) |x| > 1, real is NaN with invalid
// erfinv(NaN) is NaN, invalid if input is signalling NaN
// erfinv(x) is (2/rt(pi)) x  for |x| < 2^-30
#define EXCEPTION_HANDLING_ERFINV(vx, special_args, vy_special, vlen)          \
  do {                                                                         \
    VUINT expo_x = __riscv_vand(__riscv_vsrl(F_AS_U((vx)), MAN_LEN, (vlen)),   \
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
      tmp = __riscv_vfmul(x_small, (vx), RTPI_BY2_LO, (vlen));                 \
      tmp = __riscv_vfmacc(x_small, tmp, RTPI_BY2_HI, (vx), (vlen));           \
      (vy_special) = __riscv_vmerge((vy_special), tmp, x_small, (vlen));       \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)

// erfcinv(0) = Inf, erfcinv(2) = -Inf with divide by zero
// erfcinv(x) x outside [0, 2], real is NaN with invalid
// erfcinv(NaN) is NaN, invalid if input is signalling NaN
#define EXCEPTION_HANDLING_ERFCINV(vx, special_args, vy_special, vlen)         \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    IDENTIFY(vclass, 0x39F, (special_args), (vlen));                           \
    VBOOL x_ge_2 = __riscv_vmfge((vx), 0x1.0p1, (vlen));                       \
    (special_args) = __riscv_vmor((special_args), x_ge_2, (vlen));             \
    if (__riscv_vcpop((special_args), (vlen)) > 0) {                           \
      VBOOL x_gt_2 = __riscv_vmfgt((vx), 0x1.0p1, (vlen));                     \
      VBOOL x_lt_0 = __riscv_vmflt((vx), fp_posZero, (vlen));                  \
      /* substitute x > 2 or x < 0 with sNaN */                                \
      (vx) = __riscv_vfmerge((vx), fp_sNaN, x_gt_2, (vlen));                   \
      (vx) = __riscv_vfmerge((vx), fp_sNaN, x_lt_0, (vlen));                   \
      /* substitute x = 0 or 2 with +/-Inf and generate div-by-zero signal */  \
      VFLOAT tmp = VFMV_VF(fp_posZero, (vlen));                                \
      VFLOAT x_tmp = __riscv_vfrsub((vx), fp_posOne, (vlen));                  \
      tmp = __riscv_vfsgnj(tmp, x_tmp, (vlen));                                \
      VBOOL x_eq_2 = __riscv_vmfeq((vx), 0x1.0p1, (vlen));                     \
      VBOOL x_eq_0 = __riscv_vmfeq((vx), fp_posZero, (vlen));                  \
      VBOOL pm_Inf = __riscv_vmor(x_eq_2, x_eq_0, (vlen));                     \
      tmp = __riscv_vfrec7(pm_Inf, tmp, (vlen));                               \
      (vy_special) = __riscv_vfsub((special_args), (vx), (vx), (vlen));        \
      (vy_special) = __riscv_vmerge((vy_special), tmp, pm_Inf, (vlen));        \
      (vx) = __riscv_vfmerge((vx), fp_posOne, (special_args), (vlen));         \
    }                                                                          \
  } while (0)

// Compute -log(2^(-n_adjust) * x), where x < 1
#define NEG_LOGX_4_TRANSFORM(vx, n_adjust, y_hi, y_lo, vlen)                   \
  do {                                                                         \
    /* work on entire vector register */                                       \
    VFLOAT vx_in = (vx);                                                       \
    VINT n = __riscv_vadd(__riscv_vsra(F_AS_I(vx_in), MAN_LEN - 8, (vlen)),    \
                          0x96, vlen);                                         \
    n = __riscv_vsub(__riscv_vsra(n, 8, vlen), EXP_BIAS, vlen);                \
    VFLOAT scale = I_AS_F(                                                     \
        __riscv_vsll(__riscv_vrsub(n, EXP_BIAS, (vlen)), MAN_LEN, (vlen)));    \
    vx_in = __riscv_vfmul(vx_in, scale, (vlen));                               \
    /* x is scaled, and -log(x) is 2 atanh(w/2); w = 2(1-x)/(1+x) */           \
    n = __riscv_vsub(n, (n_adjust), (vlen));                                   \
    VFLOAT n_flt = __riscv_vfcvt_f(n, (vlen));                                 \
    VFLOAT numer = __riscv_vfrsub(vx_in, fp_posOne, (vlen));                   \
    /* note that 1-x is exact as 1/2 < x < 2 */                                \
    numer = __riscv_vfadd(numer, numer, (vlen));                               \
    VFLOAT denom = __riscv_vfadd(vx_in, fp_posOne, (vlen));                    \
    VFLOAT delta_denom = __riscv_vfadd(                                        \
        __riscv_vfrsub(denom, fp_posOne, (vlen)), vx_in, (vlen));              \
    /* note that 1 - denom is exact even if denom > 2 */                       \
    /* becase 1 has many trailing zeros */                                     \
    VFLOAT r_hi, r_lo, r;                                                      \
    DIV_N1D2(numer, denom, delta_denom, r_hi, r_lo, (vlen));                   \
    r = __riscv_vfadd(r_hi, r_lo, (vlen));                                     \
    /* for the original unscaled x, we have */                                 \
    /* -log(x) = -n * log(2) + 2 atanh(-w/2) */                                \
    /* where w = 2(1-x)/(1+x); -w = 2(x-1)/(x+1) */                            \
    VFLOAT A, B;                                                               \
    A = __riscv_vfmadd(n_flt, NEG_LOG2_HI, r_hi, (vlen));                      \
    B = __riscv_vfmsub(n_flt, NEG_LOG2_HI, A, (vlen));                         \
    B = __riscv_vfadd(B, r_hi, (vlen));                                        \
    B = __riscv_vfadd(r_lo, B, (vlen));                                        \
    VFLOAT rsq = __riscv_vfmul(r, r, (vlen));                                  \
    VFLOAT rcube = __riscv_vfmul(rsq, r, (vlen));                              \
    VFLOAT r6 = __riscv_vfmul(rcube, rcube, (vlen));                           \
    VFLOAT poly_right = PSTEP(                                                 \
        0x1.74681ff881228p-14, rsq,                                            \
        PSTEP(0x1.39751be23e4a3p-16, 0x1.30a893993e73dp-18, rsq, vlen), vlen); \
    VFLOAT poly_left = PSTEP(                                                  \
        0x1.999999996ce82p-7, rsq,                                             \
        PSTEP(0x1.249249501b1adp-9, 0x1.c71c47e7189f6p-12, rsq, vlen), vlen);  \
    poly_left = __riscv_vfmacc(poly_left, r6, poly_right, (vlen));             \
    poly_left = PSTEP(0x1.55555555555dbp-4, rsq, poly_left, (vlen));           \
    B = __riscv_vfmacc(B, NEG_LOG2_LO, n_flt, (vlen));                         \
    B = __riscv_vfmacc(B, rcube, poly_left, (vlen));                           \
    FAST2SUM(A, B, (y_hi), (y_lo), (vlen));                                    \
    /* A + B is -log(x) with extra precision, |B| \le ulp(A)/2 */              \
  } while (0)

// This macro computes w_hi + w_lo = sqrt(y_hi + y_lo) in floating point
// and 1/(w_hi + w_lo) as a Q63 fixed-point T
// y_hi, y_lo is normalized on input; that is y_hi has
// full working precision of the sum y_hi + y_lo
// and 2 log(2) < y_hi < 1100 log(2)
#define SQRTX_4_TRANSFORM(y_hi, y_lo, w_hi, w_lo, T, t_sc, t_sc_inv, vlen)     \
  do {                                                                         \
    (w_hi) = __riscv_vfsqrt((y_hi), (vlen));                                   \
    (w_lo) = __riscv_vfnmsub((w_hi), (w_hi), (y_hi), (vlen));                  \
    (w_lo) = __riscv_vfadd((w_lo), (y_lo), (vlen));                            \
    VFLOAT recip;                                                              \
    recip = __riscv_vfadd((y_hi), (y_hi), (vlen));                             \
    recip = __riscv_vfrec7(recip, (vlen));                                     \
    recip = __riscv_vfmul(recip, (w_hi), (vlen));                              \
    (w_lo) = __riscv_vfmul((w_lo), recip, (vlen));                             \
    /* w_hi + w_lo is sqrt(y_hi + y_lo) to extra precision */                  \
    /* now compute T = t_sc/(w_hi + w_lo) as fixed point */                    \
    VFLOAT t_lo = VFMV_VF((t_sc), (vlen));                                     \
    VFLOAT t_hi = __riscv_vfdiv(t_lo, (w_hi), (vlen));                         \
    (T) = __riscv_vfcvt_x(t_hi, (vlen));                                       \
    t_lo = __riscv_vfnmsac(t_lo, (w_hi), t_hi, (vlen));                        \
    t_lo = __riscv_vfnmsac(t_lo, (w_lo), t_hi, (vlen));                        \
    t_lo = __riscv_vfmul(t_lo, t_hi, (vlen));                                  \
    t_lo = __riscv_vfmul(t_lo, (t_sc_inv), vlen);                              \
    (T) = __riscv_vadd((T), __riscv_vfcvt_x(t_lo, (vlen)), (vlen));            \
  } while (0)

#define ERFCINV_PQ_TINY(T, p_hi_tiny, p_lo_tiny, q_hi_tiny, q_lo_tiny, vlen)   \
  do {                                                                         \
    /* T is in scale of 65 */                                                  \
    VINT P, Q;                                                                 \
    P = PSTEP_I_SRA(P_tiny_7, T, 4,                                            \
                    PSTEP_I_SRA(P_tiny_8, P_tiny_9, 4, T, (vlen)), (vlen));    \
    /* P in Q_56 */                                                            \
    P = PSTEP_I_SRA(P_tiny_5, T, 1, PSTEP_I_SRA(P_tiny_6, P, 2, T, (vlen)),    \
                    (vlen));                                                   \
    /* P in Q_57 */                                                            \
    P = PSTEP_I_SRA(P_tiny_3, T, 1, PSTEP_I(P_tiny_4, P, T, (vlen)), (vlen));  \
    /* P in Q_60 */                                                            \
    P = PSTEP_I_SLL(P_tiny_1, T, 1, PSTEP_I_SRA(P_tiny_2, P, 1, T, (vlen)),    \
                    (vlen));                                                   \
    /* P in Q_64 */                                                            \
    P = PSTEP_I(P_tiny_0, T, P, (vlen));                                       \
    /* P in Q_66 */                                                            \
                                                                               \
    Q = PSTEP_I_SRA(Q_tiny_7, T, 2,                                            \
                    PSTEP_I_SRA(Q_tiny_8, Q_tiny_9, 3, T, (vlen)), (vlen));    \
    /* Q in Q_56 */                                                            \
    Q = PSTEP_I_SRA(Q_tiny_5, T, 1, PSTEP_I_SRA(Q_tiny_6, Q, 2, T, (vlen)),    \
                    (vlen));                                                   \
    /* Q in Q_57 */                                                            \
    Q = PSTEP_I_SRA(Q_tiny_3, T, 2, PSTEP_I(Q_tiny_4, Q, T, (vlen)), (vlen));  \
    /* P in Q_59 */                                                            \
    Q = PSTEP_I_SLL(Q_tiny_1, T, 1, PSTEP_I(Q_tiny_2, Q, T, (vlen)), (vlen));  \
    /* Q in Q_64 */                                                            \
    Q = PSTEP_I(Q_tiny_0, T, Q, (vlen));                                       \
    /* Q in Q_66 */                                                            \
                                                                               \
    p_hi_tiny = __riscv_vfcvt_f(P, (vlen));                                    \
    p_lo_tiny = __riscv_vfcvt_f(                                               \
        __riscv_vsub(P, __riscv_vfcvt_x(p_hi_tiny, (vlen)), (vlen)), (vlen));  \
    p_lo_tiny = __riscv_vfadd(p_lo_tiny, DELTA_P0_tiny, (vlen));               \
    q_hi_tiny = __riscv_vfcvt_f(Q, vlen);                                      \
    q_lo_tiny = __riscv_vfcvt_f(                                               \
        __riscv_vsub(Q, __riscv_vfcvt_x(q_hi_tiny, (vlen)), (vlen)), (vlen));  \
    q_lo_tiny = __riscv_vfadd(q_lo, DELTA_Q0_tiny, (vlen));                    \
  } while (0)
