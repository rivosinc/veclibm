// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_LGAMMAD_STD
#else
#define F_VER1 RVVLM_LGAMMADI_STD
#endif

// Gamma(x) ~ (x-1)(x-2) * P(t)/Q(t), t = x - (3/2 - 1/8) x in [1-1/4, 2+1/4]
// Coefficients P in ascending order:
// P coeffs in ascending order
#define P_left_0 0x717ad571ef5dc61  // Q_69
#define P_left_1 0x13b685202b6fd6a4 // Q_69
#define P_left_2 0x16296a1296488970 // Q_69
#define P_left_3 0x19ca4fa8bc759bde // Q_70
#define P_left_4 0x1079dccb79c3089c // Q_71
#define P_left_5 0x16847f57936dc8fb // Q_74
#define P_left_6 0x1d546ec89ba5d14e // Q_78
#define P_left_7 0x1ad0cdf5663cfacf // Q_83
#define P_left_8 0x1f1f571e9999b6c7 // Q_92

#define Q_left_0 0x3877618277fdb07  // Q_67
#define Q_left_1 0x15e97ae7797617d9 // Q_68
#define Q_left_2 0x1c1a630c4311e499 // Q_68
#define Q_left_3 0x133731ff844280b6 // Q_68
#define Q_left_4 0x1e248b8ebb59488b // Q_70
#define Q_left_5 0x1b1f236d04a448b5 // Q_72
#define Q_left_6 0x1a6052955f6252ac // Q_75
#define Q_left_7 0x17e477b664d95b52 // Q_79
#define Q_left_8 0x1b97bd09b9b48410 // Q_85

//---Approximate log(x) by w + w^3 poly(w^2)
//   w = 2(x-1)/(x+1), x roughly in [1/rt(2), rt(2)]
#define P_log_0 0x5555555555555090 // Q_66
#define P_log_1 0x666666666686863a // Q_69
#define P_log_2 0x49249248fc99ba4b // Q_71
#define P_log_3 0x71c71ca402e164fa // Q_74
#define P_log_4 0x5d1733e3ae94dde0 // Q_76
#define P_log_5 0x4ec8b69784234032 // Q_78
#define P_log_6 0x43cc44a056dc3c93 // Q_80
#define P_log_7 0x4432439bb76e7d74 // Q_82

#define LOG2_HI 0x1.62e42fefa4000p-1
#define LOG2_LO -0x1.8432a1b0e2634p-43

// Correction to log_stirling formula
// logGamma(x) - log_sterling(x) ~ P(t)/Q(t), t = 1/x
// x in [2.25 x_max], t in (0, 1/2.25)
// Coefficients P in ascending order:
#define P_LS_corr_0 0x13eb19ce38760e4  // Q_82
#define P_LS_corr_1 0x54ebdd91a33a236  // Q_82
#define P_LS_corr_2 0xf5302c2f3171924  // Q_82
#define P_LS_corr_3 0x17e6ca6c67d42c45 // Q_82
#define P_LS_corr_4 0x18e683b7eb793968 // Q_82
#define P_LS_corr_5 0xe6a7d68df697b37  // Q_82
#define P_LS_corr_6 0x48f07444527e095  // Q_82
#define P_LS_corr_7 0x5ac3ca10d36d7d   // Q_82
#define P_LS_corr_8 -0x115718edb07d53  // Q_82

#define Q_LS_corr_0 0x2f8b7a297052f62  // Q_82
#define Q_LS_corr_1 0xc13fa37f8190cf5  // Q_82
#define Q_LS_corr_2 0x222d203fd991122d // Q_82
#define Q_LS_corr_3 0x32462b6d38e0bfd3 // Q_82
#define Q_LS_corr_4 0x31844bff55d6561a // Q_82
#define Q_LS_corr_5 0x18c83406788ab40e // Q_82
#define Q_LS_corr_6 0x643329f595fac69  // Q_82
#define Q_LS_corr_7 -0x21b0b1bff373cd  // Q_82
#define Q_LS_corr_8 -0xc9c05b696db07   // Q_82

//---Approximate log(sin(pi x)/(pi x)) as x^2 poly(x^2)
#define P_logsin_0 -0x34a34cc4a60fa863  // Q_61
#define P_logsin_1 -0x1151322ac7d51d2e  // Q_61
#define P_logsin_2 -0xada0658820c4e34   // Q_61
#define P_logsin_3 -0x80859b50a7b1918   // Q_61
#define P_logsin_4 -0x66807a019daf246   // Q_61
#define P_logsin_5 -0x555a97e7d8482c8   // Q_61
#define P_logsin_6 -0x4927ceefdc18f62   // Q_61
#define P_logsin_7 -0x3fe5862d4e702a2   // Q_61
#define P_logsin_8 -0x39da522c5099734   // Q_61
#define P_logsin_9 -0x2cbb6825e3efaad   // Q_61
#define P_logsin_10 -0x4df815d2f21e674  // Q_61
#define P_logsin_11 0x41cf7e791cb446c   // Q_61
#define P_logsin_12 -0x126ea0159b1a7052 // Q_61
#define P_logsin_13 0x155103f2634da2c6  // Q_61
#define P_logsin_14 -0x13e497482ec6dff4 // Q_61

//---Approximate exp(R) by 1 + R + R^2*poly(R)
#define P_exp_0 0x400000000000004e // Q_63
#define P_exp_1 0x1555555555555b6e // Q_63
#define P_exp_2 0x555555555553378  // Q_63
#define P_exp_3 0x1111111110ec10d  // Q_63
#define P_exp_4 0x2d82d82d87a9b5   // Q_63
#define P_exp_5 0x6806806ce6d6f    // Q_63
#define P_exp_6 0xd00d00841fcf     // Q_63
#define P_exp_7 0x171ddefda54b     // Q_63
#define P_exp_8 0x24fcc01d627      // Q_63
#define P_exp_9 0x35ed8bbd24       // Q_63
#define P_exp_10 0x477745b6c       // Q_63

//---Approximate Stirling correction by P(t)/Q(t)
//  Gamma(x) = (x/e)^(x-1/2) * P(t)/Q(t), t = 1/x, x in [2, 180]
#define P_corr_0 0x599ecf7a9368327  // Q_78
#define P_corr_1 0x120a4be8e3d8673d // Q_78
#define P_corr_2 0x2ab73aec63e90213 // Q_78
#define P_corr_3 0x32f903e18454e088 // Q_78
#define P_corr_4 0x29f463d533d0a4b5 // Q_78
#define P_corr_5 0x1212989fdf61f6c1 // Q_78
#define P_corr_6 0x48706d4f75a0491  // Q_78
#define P_corr_7 0x5591439d2d51a6   // Q_78

#define Q_corr_0 0x75e5053ce715a76  // Q_79
#define Q_corr_1 0x171e2068d3ef7453 // Q_79
#define Q_corr_2 0x363d736690f2373f // Q_79
#define Q_corr_3 0x3e793a1cc19bbc32 // Q_79
#define Q_corr_4 0x31dc2fbf92ec978c // Q_79
#define Q_corr_5 0x138c2244d1c1e0b1 // Q_79
#define Q_corr_6 0x450a7392d81c20f  // Q_79
#define Q_corr_7 0x1ed9c605221435   // Q_79

//---Approximate sin(pi x)/pi as x + x^3 poly(x^2)
#define P_sin_0 -0x694699894c1f4ae7 // Q_62
#define P_sin_1 0x33f396805788034f  // Q_62
#define P_sin_2 -0xc3547239048c220  // Q_62
#define P_sin_3 0x1ac6805cc1cecf4   // Q_62
#define P_sin_4 -0x26702d2fd5a3e6   // Q_62
#define P_sin_5 0x26e8d360232c6     // Q_62
#define P_sin_6 -0x1d3e4d9787ba     // Q_62
#define P_sin_7 0x107298fc107       // Q_62

// lgamma(qNaN/sNaN) is qNaN, invalid if input is sNaN
// lgamma(+-Inf) is +Inf
// lgamma(+-0) is +Inf and divide by zero
// lgamma(x) = -log(|x|) when |x| < 2^(-60)
#define EXCEPTION_HANDLING_LGAMMA(vx, special_args, vy_special, vlen)          \
  do {                                                                         \
    VUINT expo_x = __riscv_vand(__riscv_vsrl(F_AS_U((vx)), MAN_LEN, (vlen)),   \
                                0x7FF, (vlen));                                \
    VBOOL x_small = __riscv_vmsltu(expo_x, EXP_BIAS - 60, (vlen));             \
    VBOOL x_InfNaN = __riscv_vmseq(expo_x, 0x7FF, (vlen));                     \
    (special_args) = __riscv_vmor(x_small, x_InfNaN, (vlen));                  \
    if (__riscv_vcpop((special_args), (vlen)) > 0) {                           \
      VUINT vclass = __riscv_vfclass((vx), (vlen));                            \
      VBOOL x_Inf;                                                             \
      IDENTIFY(vclass, class_Inf, x_Inf, (vlen));                              \
      (vx) = __riscv_vfmerge((vx), fp_posInf, x_Inf, (vlen));                  \
      VBOOL x_Zero;                                                            \
      IDENTIFY(vclass, 0x18, x_Zero, (vlen));                                  \
      (vx) = __riscv_vfmerge((vx), fp_posZero, x_Zero, (vlen));                \
      VFLOAT y_tmp = (vx);                                                     \
      VFLOAT y_0 = __riscv_vfrec7(x_Zero, (vx), (vlen));                       \
      y_tmp = __riscv_vmerge(y_tmp, y_0, x_Zero, (vlen));                      \
      (vy_special) = __riscv_vfadd((special_args), (vx), y_tmp, (vlen));       \
      x_small = __riscv_vmandn(                                                \
          x_small, __riscv_vmfeq((vx), fp_posZero, (vlen)), (vlen));           \
      if (__riscv_vcpop(x_small, (vlen)) > 0) {                                \
        VFLOAT x_tmp = VFMV_VF(fp_posOne, (vlen));                             \
        x_tmp = __riscv_vmerge(x_tmp, (vx), x_small, (vlen));                  \
        x_tmp = __riscv_vfsgnj(x_tmp, fp_posOne, (vlen));                      \
        VFLOAT zero = VFMV_VF(fp_posZero, (vlen));                             \
        VFLOAT y_hi, y_lo;                                                     \
        LGAMMA_LOG(x_tmp, zero, zero, y_hi, y_lo, (vlen));                     \
        y_hi = __riscv_vfadd(y_hi, y_lo, (vlen));                              \
        y_hi = __riscv_vfsgnj(y_hi, fp_posOne, (vlen));                        \
        (vy_special) = __riscv_vmerge((vy_special), y_hi, x_small, (vlen));    \
      }                                                                        \
      (vx) = __riscv_vfmerge((vx), 0x1.0p2, (special_args), (vlen));           \
    }                                                                          \
  } while (0)

// This macro computes loggamma(x) for 0 < x <= 2.25
// It uses a rational approximation for x in [0.75, 2.25]
// For x < 0.75, it uses the relation gamma(x) = gamma(x+1)/x
#define LGAMMA_LE_225(x, logx_hi, logx_lo, y_hi, y_lo, vlen)                   \
  do {                                                                         \
    VBOOL x_lt_75 = __riscv_vmflt((x), 0x1.8p-1, (vlen));                      \
    VFLOAT c = VFMV_VF(0x1.6p63, (vlen));                                      \
    c = __riscv_vfmerge(c, 0x1.8p61, x_lt_75, (vlen));                         \
    VFLOAT rt1 = VFMV_VF(0x1.0p0, (vlen));                                     \
    VFLOAT rt2 = VFMV_VF(0x1.0p1, (vlen));                                     \
    rt1 = __riscv_vfmerge(rt1, fp_posZero, x_lt_75, (vlen));                   \
    rt2 = __riscv_vfmerge(rt2, fp_posOne, x_lt_75, (vlen));                    \
    VFLOAT t_hi = __riscv_vfmsub((x), 0x1.0p63, c, (vlen));                    \
    VFLOAT t_lo = __riscv_vfadd(t_hi, c, (vlen));                              \
    t_lo = __riscv_vfmsub((x), 0x1.0p63, t_lo, (vlen));                        \
    VFLOAT fact1 = __riscv_vfsub((x), rt1, (vlen));                            \
    VINT T = __riscv_vfcvt_x(t_hi, (vlen));                                    \
    T = __riscv_vadd(T, __riscv_vfcvt_x(t_lo, (vlen)), (vlen));                \
    VFLOAT fact2_hi = __riscv_vfsub((x), rt2, (vlen));                         \
    VFLOAT fact2_lo = __riscv_vfadd(fact2_hi, rt2, (vlen));                    \
    fact2_lo = __riscv_vfsub((x), fact2_lo, (vlen));                           \
    VFLOAT fact_hi, fact_lo;                                                   \
    PROD_X1Y2(fact1, fact2_hi, fact2_lo, fact_hi, fact_lo, (vlen));            \
    VINT P = PSTEP_I_SRA(P_left_7, P_left_8, 9, T, (vlen));                    \
    P = PSTEP_I_SRA(P_left_6, T, 5, P, (vlen));                                \
    P = PSTEP_I_SRA(P_left_5, T, 4, P, (vlen));                                \
    P = PSTEP_I_SRA(P_left_4, T, 3, P, (vlen));                                \
    P = PSTEP_I_SRA(P_left_3, T, 1, P, (vlen));                                \
    P = PSTEP_I_SRA(P_left_2, T, 1, P, (vlen));                                \
    P = PSTEP_I(P_left_1, T, P, (vlen));                                       \
    P = PSTEP_I(P_left_0, T, P, (vlen));                                       \
    VINT Q = PSTEP_I_SRA(Q_left_7, Q_left_8, 6, T, (vlen));                    \
    Q = PSTEP_I_SRA(Q_left_6, T, 4, Q, (vlen));                                \
    Q = PSTEP_I_SRA(Q_left_5, T, 3, Q, (vlen));                                \
    Q = PSTEP_I_SRA(Q_left_4, T, 2, Q, (vlen));                                \
    Q = PSTEP_I_SRA(Q_left_3, T, 2, Q, (vlen));                                \
    Q = PSTEP_I(Q_left_1, T, PSTEP_I(Q_left_2, T, Q, (vlen)), (vlen));         \
    Q = PSTEP_I_SRA(Q_left_0, T, 1, Q, (vlen));                                \
    /* P is in Q69 and Q is in Q67 */                                          \
    VFLOAT p_hi = __riscv_vfcvt_f(P, (vlen));                                  \
    VFLOAT p_lo = __riscv_vfcvt_f(                                             \
        __riscv_vsub(P, __riscv_vfcvt_x(p_hi, (vlen)), (vlen)), (vlen));       \
    VFLOAT q_hi = __riscv_vfcvt_f(Q, (vlen));                                  \
    VFLOAT q_lo = __riscv_vfcvt_f(                                             \
        __riscv_vsub(Q, __riscv_vfcvt_x(q_hi, (vlen)), (vlen)), (vlen));       \
    VFLOAT z_hi, z_lo;                                                         \
    DIV2_N2D2(p_hi, p_lo, q_hi, q_lo, z_hi, z_lo, (vlen));                     \
    z_hi = __riscv_vfmul(z_hi, 0x1.0p-2, (vlen));                              \
    z_lo = __riscv_vfmul(z_lo, 0x1.0p-2, (vlen));                              \
    PROD_X2Y2(z_hi, z_lo, fact_hi, fact_lo, (y_hi), (y_lo), (vlen));           \
    /* if original input is in (0, 3/4), need to add -log(x) */                \
    VFLOAT A, a;                                                               \
    A = I_AS_F(__riscv_vxor(F_AS_I(A), F_AS_I(A), (vlen)));                    \
    a = I_AS_F(__riscv_vxor(F_AS_I(a), F_AS_I(a), (vlen)));                    \
    A = __riscv_vmerge(A, (logx_hi), x_lt_75, (vlen));                         \
    a = __riscv_vmerge(a, (logx_lo), x_lt_75, (vlen));                         \
    /* y_hi + y_lo - (A + a), A is either 0 or dominates */                    \
    z_hi = __riscv_vfsub((y_hi), A, (vlen));                                   \
    z_lo = __riscv_vfadd(z_hi, A, (vlen));                                     \
    z_lo = __riscv_vfsub((y_hi), z_lo, (vlen));                                \
    (y_lo) = __riscv_vfadd((y_lo), z_lo, (vlen));                              \
    (y_lo) = __riscv_vfsub((y_lo), a, (vlen));                                 \
    (y_hi) = z_hi;                                                             \
  } while (0)

//---Compute log(x/e) or log(x) to 2^(-65) absolute accuracy
//   log(x) - c, c is 1 or 0; x > 0
#define LGAMMA_LOG(x_hi, x_lo, c, y_hi, y_lo, vlen)                            \
  do {                                                                         \
    /* need x_hi, x_lo as input */                                             \
    VFLOAT x_in_hi = (x_hi);                                                   \
    VFLOAT x_in_lo = (x_lo);                                                   \
    VINT n_adjust;                                                             \
    n_adjust = __riscv_vxor(n_adjust, n_adjust, (vlen));                       \
    VBOOL x_tiny = __riscv_vmflt(x_in_hi, 0x1.0p-1020, (vlen));                \
    if (__riscv_vcpop(x_tiny, (vlen)) > 0) {                                   \
      VFLOAT x_adjust = __riscv_vfmul(x_tiny, x_in_hi, 0x1.0p60, (vlen));      \
      x_in_hi = __riscv_vmerge(x_in_hi, x_adjust, x_tiny, (vlen));             \
      x_adjust = __riscv_vfmul(x_tiny, x_in_lo, 0x1.0p60, (vlen));             \
      x_in_lo = __riscv_vmerge(x_in_lo, x_adjust, x_tiny, (vlen));             \
      n_adjust = __riscv_vmerge(n_adjust, 60, x_tiny, (vlen));                 \
    }                                                                          \
    VINT n = __riscv_vadd(__riscv_vsra(F_AS_I(x_in_hi), MAN_LEN - 8, (vlen)),  \
                          0x96, vlen);                                         \
    n = __riscv_vsub(__riscv_vsra(n, 8, vlen), EXP_BIAS, vlen);                \
    VFLOAT scale = I_AS_F(                                                     \
        __riscv_vsll(__riscv_vrsub(n, EXP_BIAS, (vlen)), MAN_LEN, (vlen)));    \
    x_in_hi = __riscv_vfmul(x_in_hi, scale, (vlen));                           \
    x_in_lo = __riscv_vfmul(x_in_lo, scale, (vlen));                           \
    n = __riscv_vsub(n, n_adjust, (vlen));                                     \
    /* x is scaled, and log(x) is 2 atanh(w/2); w = 2(x-1)/(x+1) */            \
                                                                               \
    VFLOAT numer, denom, denom_delta;                                          \
    numer = __riscv_vfsub(x_in_hi, fp_posOne, (vlen)); /* exact */             \
    denom = __riscv_vfadd(x_in_hi, fp_posOne, (vlen));                         \
    denom_delta = __riscv_vfadd(__riscv_vfrsub(denom, fp_posOne, (vlen)),      \
                                x_in_hi, (vlen));                              \
    denom_delta = __riscv_vfadd(denom_delta, x_in_lo, (vlen));                 \
    VFLOAT w_hi, w_lo;                                                         \
    ACC_DIV2_N2D2(numer, x_in_lo, denom, denom_delta, w_hi, w_lo, vlen);       \
    /* w_hi + w_lo is at this point (x-1)/(x+1) */                             \
    /* Next get 2(x-1)/(x+1) in Q64 fixed point */                             \
    VINT W = __riscv_vfcvt_x(__riscv_vfmul(w_hi, 0x1.0p65, (vlen)), (vlen));   \
    W = __riscv_vadd(                                                          \
        W, __riscv_vfcvt_x(__riscv_vfmul(w_lo, 0x1.0p65, (vlen)), (vlen)),     \
        (vlen));                                                               \
    /* W is in Q64 because W is 2(x-1)/(x+1) */                                \
                                                                               \
    VFLOAT n_flt = __riscv_vfcvt_f(n, (vlen));                                 \
    VINT W2 = __riscv_vsmul(W, W, 1, (vlen)); /* Q65 */                        \
                                                                               \
    VINT P_right, P_left, W8;                                                  \
    P_right = PSTEP_I_SRA(P_log_6, P_log_7, 4, W2, (vlen));                    \
    P_right = PSTEP_I_SRA(P_log_5, W2, 4, P_right, (vlen));                    \
    P_right = PSTEP_I_SRA(P_log_4, W2, 4, P_right, (vlen));                    \
    /* P_right in Q76 */                                                       \
    P_left = PSTEP_I_SRA(P_log_2, P_log_3, 5, W2, (vlen));                     \
    P_left = PSTEP_I_SRA(P_log_1, W2, 4, P_left, (vlen));                      \
    P_left = PSTEP_I_SRA(P_log_0, W2, 5, P_left, (vlen));                      \
    /* P_left in Q66 */                                                        \
    W8 = __riscv_vsmul(W2, W2, 1, (vlen));           /* Q67 */                 \
    W8 = __riscv_vsmul(W8, W8, 1, (vlen));           /* Q71 */                 \
    P_right = __riscv_vsmul(P_right, W8, 1, (vlen)); /* Q84 */                 \
    P_right = __riscv_vsra(P_right, 18, (vlen));     /* Q66 */                 \
    P_left = __riscv_vadd(P_left, P_right, (vlen));  /* Q66 */                 \
                                                                               \
    VINT W3 = __riscv_vsmul(W2, W, 1, (vlen));     /* Q66 */                   \
    P_left = __riscv_vsmul(P_left, W3, 1, (vlen)); /* Q69 */                   \
    VFLOAT poly_hi = __riscv_vfcvt_f(P_left, (vlen));                          \
    P_left = __riscv_vsub(P_left, __riscv_vfcvt_x(poly_hi, (vlen)), (vlen));   \
    VFLOAT poly_lo = __riscv_vfcvt_f(P_left, (vlen));                          \
    poly_hi = __riscv_vfmul(poly_hi, 0x1.0p-69, (vlen));                       \
    poly_lo = __riscv_vfmul(poly_lo, 0x1.0p-69, (vlen));                       \
                                                                               \
    /* n*log(2) - c + w + poly is the desired result */                        \
    VFLOAT A, B;                                                               \
    A = __riscv_vfmul(n_flt, LOG2_HI, (vlen)); /* exact */                     \
    A = __riscv_vfsub(A, (c), (vlen));         /* exact due to A's range */    \
    w_hi = __riscv_vfadd(w_hi, w_hi, (vlen));                                  \
    w_lo = __riscv_vfadd(w_lo, w_lo, (vlen));                                  \
    FAST2SUM(A, w_hi, B, (y_lo), (vlen));                                      \
    w_lo = __riscv_vfadd((y_lo), w_lo, (vlen));                                \
    w_lo = __riscv_vfmacc(w_lo, LOG2_LO, n_flt, (vlen));                       \
    poly_lo = __riscv_vfadd(poly_lo, w_lo, (vlen));                            \
    FAST2SUM(B, poly_hi, (y_hi), (y_lo), (vlen));                              \
    (y_lo) = __riscv_vfadd((y_lo), poly_lo, (vlen));                           \
  } while (0)

// Use Stirling approximation with correction when x >= 9/4
// on input logx_hi, logx_lo is log(x)-1
// result is returned in y_hi, y_lo
#define LGAMMA_LOG_STIRLING(x, logx_hi, logx_lo, y_hi, y_lo, expo_adj, vlen)   \
  do {                                                                         \
    VFLOAT x_in = (x);                                                         \
    VBOOL adjust_x = __riscv_vmfge(x_in, 0x1.0p+200, (vlen));                  \
    (expo_adj) = __riscv_vmerge((expo_adj), 100, adjust_x, (vlen));            \
    VINT m = __riscv_vrsub((expo_adj), EXP_BIAS, (vlen));                      \
    VFLOAT scale = I_AS_F(__riscv_vsll(m, MAN_LEN, (vlen)));                   \
    x_in = __riscv_vfmul(x_in, scale, (vlen));                                 \
    VFLOAT w_hi, w_lo;                                                         \
    w_hi = __riscv_vfsub(x_in, 0x1.0p-1, (vlen));                              \
    w_lo = __riscv_vfsub(x_in, w_hi, (vlen));                                  \
    w_lo = __riscv_vfsub(w_lo, 0x1.0p-1, (vlen));                              \
    PROD_X2Y2(w_hi, w_lo, (logx_hi), (logx_lo), (y_hi), (y_lo), (vlen));       \
  } while (0)

// Compute log(x*r) + log(|sin(pi r)/(pi r)|) where x = N + r, |r| <= 1/2
// This is for handling of gamma at negative arguments where
// we have a denominator of x sin(pi x)/pi.
// Since taking the log of |sin(pi x)/pi|, same as log |sin(pi r)/pi|
// is more easily done with doing log(|r|) + log|sin(pi r)/(pi r)|
// as the latter can be approximated by r^2 poly(r^2).
// The term log(|r|) is combined with log(|x|) by log(|r * x|)
// This macro also sets special arguments when x is of integral value
// The macro assumes x > 0 and it suffices to clip it to 2^52 as x will be
// of integral value at and beyond 2^52.
#define LGAMMA_LOGSIN(x, y_hi, y_lo, vy_special, special_args, vlen)           \
  do {                                                                         \
    VFLOAT x_in = __riscv_vfmin((x), 0x1.0p+52, (vlen));                       \
    VFLOAT n_flt;                                                              \
    VINT n = __riscv_vfcvt_x(x_in, (vlen));                                    \
    n_flt = __riscv_vfcvt_f(n, (vlen));                                        \
    VFLOAT r = __riscv_vfsub(x_in, n_flt, (vlen));                             \
    VBOOL pole = __riscv_vmfeq(r, fp_posZero, (vlen));                         \
    if (__riscv_vcpop(pole, (vlen)) > 0) {                                     \
      r = __riscv_vfmerge(r, 0x1.0p-1, pole, (vlen));                          \
      (special_args) = __riscv_vmor((special_args), pole, (vlen));             \
      (vy_special) = __riscv_vfmerge((vy_special), fp_posInf, pole, (vlen));   \
    }                                                                          \
    VFLOAT rsq = __riscv_vfmul(r, r, (vlen));                                  \
    VFLOAT rsq_lo = __riscv_vfmsub(r, r, rsq, (vlen));                         \
    VINT Rsq = __riscv_vfcvt_x(__riscv_vfmul(rsq, 0x1.0p63, (vlen)), (vlen));  \
    Rsq = __riscv_vadd(                                                        \
        Rsq, __riscv_vfcvt_x(__riscv_vfmul(rsq_lo, 0x1.0p63, (vlen)), (vlen)), \
        (vlen));                                                               \
    VINT P_right = PSTEP_I(                                                    \
        P_logsin_8, Rsq,                                                       \
        PSTEP_I(P_logsin_9, Rsq,                                               \
                PSTEP_I(P_logsin_10, Rsq,                                      \
                        PSTEP_I(P_logsin_11, Rsq,                              \
                                PSTEP_I(P_logsin_12, Rsq,                      \
                                        PSTEP_I(P_logsin_13, P_logsin_14, Rsq, \
                                                (vlen)),                       \
                                        (vlen)),                               \
                                (vlen)),                                       \
                        (vlen)),                                               \
                (vlen)),                                                       \
        (vlen));                                                               \
                                                                               \
    VINT P_left = PSTEP_I(                                                     \
        P_logsin_0, Rsq,                                                       \
        PSTEP_I(                                                               \
            P_logsin_1, Rsq,                                                   \
            PSTEP_I(P_logsin_2, Rsq,                                           \
                    PSTEP_I(P_logsin_3, Rsq,                                   \
                            PSTEP_I(P_logsin_4, Rsq,                           \
                                    PSTEP_I(P_logsin_5, Rsq,                   \
                                            PSTEP_I(P_logsin_6, P_logsin_7,    \
                                                    Rsq, (vlen)),              \
                                            (vlen)),                           \
                                    (vlen)),                                   \
                            (vlen)),                                           \
                    (vlen)),                                                   \
            (vlen)),                                                           \
        (vlen));                                                               \
    VINT R16 = __riscv_vsmul(Rsq, Rsq, 1, (vlen));                             \
    R16 = __riscv_vsmul(R16, R16, 1, (vlen));                                  \
    R16 = __riscv_vsmul(R16, R16, 1, (vlen));                                  \
    P_right = __riscv_vsmul(R16, P_right, 1, (vlen));                          \
    P_left = __riscv_vadd(P_left, P_right, (vlen));                            \
    VFLOAT z_hi = __riscv_vfcvt_f(P_left, (vlen));                             \
    P_right = __riscv_vfcvt_x(z_hi, (vlen));                                   \
    VFLOAT z_lo =                                                              \
        __riscv_vfcvt_f(__riscv_vsub(P_left, P_right, (vlen)), (vlen));        \
    z_hi = __riscv_vfmul(z_hi, 0x1.0p-61, (vlen));                             \
    z_lo = __riscv_vfmul(z_lo, 0x1.0p-61, (vlen));                             \
    VFLOAT ls_hi, ls_lo;                                                       \
    PROD_X2Y2(z_hi, z_lo, rsq, rsq_lo, ls_hi, ls_lo, (vlen));                  \
    /* At this point we have log|sin(pi r)/(pi r)| */                          \
                                                                               \
    /* we now compute log(|x r|); 2^(-60) <= x <= 2^52 by design */            \
    VFLOAT xr_hi, xr_lo;                                                       \
    r = __riscv_vfsgnj(r, fp_posOne, (vlen));                                  \
    PROD_X1Y1(r, x_in, xr_hi, xr_lo, (vlen));                                  \
    VFLOAT logx_hi, logx_lo, c;                                                \
    c = I_AS_F(__riscv_vxor(F_AS_I(c), F_AS_I(c), (vlen)));                    \
    LGAMMA_LOG(xr_hi, xr_lo, c, logx_hi, logx_lo, (vlen));                     \
    VFLOAT S_hi, S_lo;                                                         \
    KNUTH2SUM(ls_hi, logx_hi, S_hi, S_lo, (vlen));                             \
    logx_lo = __riscv_vfadd(logx_lo, ls_lo, (vlen));                           \
    (y_lo) = __riscv_vfadd(S_lo, logx_lo, (vlen));                             \
    (y_hi) = S_hi;                                                             \
  } while (0)

// LogGamma based on Stirling formula is
// LogGamma(x) ~ (x-1/2)*(log(x)-1) + poly(1/x)
// This poly(1/x) is in essense a correction term
// This form is used when x >= 9/4. We use Q63 to represent 1/x
#define LOG_STIRLING_CORRECTION(x, y_hi, y_lo, vlen)                           \
  do {                                                                         \
    VFLOAT x_in = __riscv_vfmin((x), 0x1.0p80, (vlen));                        \
    VFLOAT z_hi = __riscv_vfrdiv(x_in, fp_posOne, (vlen));                     \
    VFLOAT z_lo = VFMV_VF(fp_posOne, (vlen));                                  \
    z_lo = __riscv_vfnmsub(x_in, z_hi, z_lo, (vlen));                          \
    z_lo = __riscv_vfmul(z_hi, z_lo, (vlen));                                  \
    z_hi = __riscv_vfmul(z_hi, 0x1.0p63, (vlen));                              \
    z_lo = __riscv_vfmul(z_lo, 0x1.0p63, (vlen));                              \
    VINT R = __riscv_vfcvt_x(z_hi, (vlen));                                    \
    R = __riscv_vadd(R, __riscv_vfcvt_x(z_lo, (vlen)), (vlen));                \
    VINT P_SC, Q_SC;                                                           \
    /* R is 1/x in Q63 */                                                      \
    P_SC = PSTEP_I(                                                            \
        P_LS_corr_4, R,                                                        \
        PSTEP_I(P_LS_corr_5, R,                                                \
                PSTEP_I(P_LS_corr_6, R,                                        \
                        PSTEP_I(P_LS_corr_7, P_LS_corr_8, R, (vlen)), (vlen)), \
                (vlen)),                                                       \
        (vlen));                                                               \
    P_SC = PSTEP_I(                                                            \
        P_LS_corr_0, R,                                                        \
        PSTEP_I(P_LS_corr_1, R,                                                \
                PSTEP_I(P_LS_corr_2, R, PSTEP_I(P_LS_corr_3, R, P_SC, (vlen)), \
                        (vlen)),                                               \
                (vlen)),                                                       \
        (vlen));                                                               \
    Q_SC = PSTEP_I(                                                            \
        Q_LS_corr_4, R,                                                        \
        PSTEP_I(Q_LS_corr_5, R,                                                \
                PSTEP_I(Q_LS_corr_6, R,                                        \
                        PSTEP_I(Q_LS_corr_7, Q_LS_corr_8, R, (vlen)), (vlen)), \
                (vlen)),                                                       \
        (vlen));                                                               \
    Q_SC = PSTEP_I(                                                            \
        Q_LS_corr_0, R,                                                        \
        PSTEP_I(Q_LS_corr_1, R,                                                \
                PSTEP_I(Q_LS_corr_2, R, PSTEP_I(Q_LS_corr_3, R, Q_SC, (vlen)), \
                        (vlen)),                                               \
                (vlen)),                                                       \
        (vlen));                                                               \
    VFLOAT p_hi, p_lo, q_hi, q_lo;                                             \
    VINT P_tmp, Q_tmp;                                                         \
    p_hi = __riscv_vfcvt_f(P_SC, (vlen));                                      \
    P_tmp = __riscv_vfcvt_x(p_hi, (vlen));                                     \
    p_lo = __riscv_vfcvt_f(__riscv_vsub(P_SC, P_tmp, (vlen)), (vlen));         \
    q_hi = __riscv_vfcvt_f(Q_SC, (vlen));                                      \
    Q_tmp = __riscv_vfcvt_x(q_hi, (vlen));                                     \
    q_lo = __riscv_vfcvt_f(__riscv_vsub(Q_SC, Q_tmp, (vlen)), (vlen));         \
    ACC_DIV2_N2D2(p_hi, p_lo, q_hi, q_lo, (y_hi), (y_lo), (vlen));             \
  } while (0)

void F_VER1(API) {
  size_t vlen = VSETMAX();
  VFLOAT vx, vx_orig, vy, vy_special;
  VBOOL special_args;
  VFLOAT zero = VFMV_VF(fp_posZero, vlen);

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    // Handle Inf and NaN and Zero
    EXCEPTION_HANDLING_LGAMMA(vx, special_args, vy_special, vlen);
    vx_orig = vx;

    // Work with the absolute value.
    // Modify loggamma(|x|) appropriately later on if x < 0.
    vx = __riscv_vfabs(vx, vlen);
    vx_orig = __riscv_vfsgnj(vx, vx_orig, vlen);

    VBOOL x_lt_225 = __riscv_vmflt(vx, 0x1.2p+1, vlen);
    VFLOAT c = VFMV_VF(fp_posOne, vlen);
    c = __riscv_vfmerge(c, fp_posZero, x_lt_225, vlen);

    VFLOAT logx_hi, logx_lo;
    LGAMMA_LOG(vx, zero, c, logx_hi, logx_lo, vlen);

    VFLOAT y_left_hi, y_left_lo;
    if (__riscv_vcpop(x_lt_225, vlen) > 0) {
      // Consider 0 < x < 2.25 to be rare cases
      VFLOAT vx_tmp;
      vx_tmp = VFMV_VF(0x1.0p0, vlen);
      vx_tmp = __riscv_vmerge(vx_tmp, vx, x_lt_225, vlen);
      LGAMMA_LE_225(vx_tmp, logx_hi, logx_lo, y_left_hi, y_left_lo, vlen);
    }

    VFLOAT stir_hi, stir_lo;
    VFLOAT stir_corr_hi, stir_corr_lo;
    VINT expo_adj;
    expo_adj = __riscv_vxor(expo_adj, expo_adj, vlen);
    LGAMMA_LOG_STIRLING(vx, logx_hi, logx_lo, stir_hi, stir_lo, expo_adj, vlen);
    LOG_STIRLING_CORRECTION(vx, stir_corr_hi, stir_corr_lo, vlen);

    VFLOAT loggamma_hi, loggamma_lo;
    KNUTH2SUM(stir_hi, stir_corr_hi, loggamma_hi, loggamma_lo, vlen);
    loggamma_lo = __riscv_vfadd(loggamma_lo, stir_corr_lo, vlen);
    loggamma_lo = __riscv_vfadd(loggamma_lo, stir_lo, vlen);

    loggamma_hi = __riscv_vmerge(loggamma_hi, y_left_hi, x_lt_225, vlen);
    loggamma_lo = __riscv_vmerge(loggamma_lo, y_left_lo, x_lt_225, vlen);

    VBOOL x_lt_0 = __riscv_vmflt(vx_orig, fp_posZero, vlen);

    if (__riscv_vcpop(x_lt_0, vlen) > 0) {
      // for negative x, the desired result is
      // log(1/gamma(|x|)) + log(1/(|x sin(pi x)/pi|))
      // loggamma(|x|) is in loggamma_{hi,lo}
      // we use the macro to get log(|x sin(pi x)/ pi|)
      VFLOAT vx_for_neg = VFMV_VF(0x1.0p-1, vlen);
      vx_for_neg = __riscv_vmerge(vx_for_neg, vx, x_lt_0, vlen);
      VFLOAT logsin_hi, logsin_lo;
      LGAMMA_LOGSIN(vx_for_neg, logsin_hi, logsin_lo, vy_special, special_args,
                    vlen);

      VFLOAT A, a;
      KNUTH2SUM(loggamma_hi, logsin_hi, A, a, vlen);
      a = __riscv_vfadd(a, logsin_lo, vlen);
      a = __riscv_vfadd(a, loggamma_lo, vlen);
      A = __riscv_vfsgnjx(A, fp_negOne, vlen);
      a = __riscv_vfsgnjx(a, fp_negOne, vlen);
      loggamma_hi = __riscv_vmerge(loggamma_hi, A, x_lt_0, vlen);
      loggamma_lo = __riscv_vmerge(loggamma_lo, a, x_lt_0, vlen);
    }
    loggamma_hi = __riscv_vfadd(loggamma_hi, loggamma_lo, vlen);
    expo_adj = __riscv_vadd(expo_adj, EXP_BIAS, vlen);
    vy = __riscv_vfmul(loggamma_hi,
                       I_AS_F(__riscv_vsll(expo_adj, MAN_LEN, vlen)), vlen);

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
