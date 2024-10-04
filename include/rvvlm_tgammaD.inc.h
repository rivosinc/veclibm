// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_TGAMMAD_STD
#else
#define F_VER1 RVVLM_TGAMMADI_STD
#endif

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

//---Compute log(x/e) to 2^(-65) absolute accuracy
//   for Stirlings formula (x/e)^x sqrt(2pi/x) = (x/e)^(x-1/2) sqrt(2pi/e)
#define TGAMMA_LOG(x_hi, x_lo, y_hi, y_lo, vlen)                               \
  do {                                                                         \
    VFLOAT x_in_hi = (x_hi);                                                   \
    VFLOAT x_in_lo = (x_lo);                                                   \
    VINT n = __riscv_vadd(__riscv_vsra(F_AS_I(x_in_hi), MAN_LEN - 8, (vlen)),  \
                          0x96, vlen);                                         \
    n = __riscv_vsub(__riscv_vsra(n, 8, vlen), EXP_BIAS, vlen);                \
    VFLOAT scale = I_AS_F(                                                     \
        __riscv_vsll(__riscv_vrsub(n, EXP_BIAS, (vlen)), MAN_LEN, (vlen)));    \
    x_in_hi = __riscv_vfmul(x_in_hi, scale, (vlen));                           \
    x_in_lo = __riscv_vfmul(x_in_lo, scale, (vlen));                           \
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
    /* n*log(2) - 1 + w + poly is the desired result */                        \
    VFLOAT A, B;                                                               \
    A = __riscv_vfmul(n_flt, LOG2_HI, (vlen)); /* exact */                     \
    A = __riscv_vfsub(A, fp_posOne, (vlen));   /* exact due to A's range */    \
    w_hi = __riscv_vfadd(w_hi, w_hi, (vlen));                                  \
    w_lo = __riscv_vfadd(w_lo, w_lo, (vlen));                                  \
    FAST2SUM(A, w_hi, B, (y_lo), (vlen));                                      \
    w_lo = __riscv_vfadd((y_lo), w_lo, (vlen));                                \
    w_lo = __riscv_vfmacc(w_lo, LOG2_LO, n_flt, (vlen));                       \
    poly_lo = __riscv_vfadd(poly_lo, w_lo, (vlen));                            \
    FAST2SUM(B, poly_hi, (y_hi), (y_lo), (vlen));                              \
    (y_lo) = __riscv_vfadd((y_lo), poly_lo, (vlen));                           \
  } while (0)

//---Compute exp for Stirlings formula used in tgamma
//   computes exp(x_hi + x_lo) as 2^n * EXP, EXP is fixed-point Q62
#define TGAMMA_EXP(x_hi, x_lo, n, EXP, vlen)                                   \
  do {                                                                         \
    VFLOAT n_flt = __riscv_vfmul((x_hi), 0x1.71547652b82fep+0, (vlen));        \
    (n) = __riscv_vfcvt_x(n_flt, (vlen));                                      \
    n_flt = __riscv_vfcvt_f((n), (vlen));                                      \
    VFLOAT r_hi = __riscv_vfnmsub(n_flt, LOG2_HI, (x_hi), (vlen));             \
    VFLOAT r_lo = __riscv_vfnmsub(n_flt, LOG2_LO, (x_lo), (vlen));             \
    r_hi = __riscv_vfmul(r_hi, 0x1.0p63, (vlen));                              \
    r_lo = __riscv_vfmul(r_lo, 0x1.0p63, (vlen));                              \
    VINT R = __riscv_vfcvt_x(r_hi, (vlen));                                    \
    R = __riscv_vadd(R, __riscv_vfcvt_x(r_lo, (vlen)), (vlen));                \
    /* R is reduced argument in Q63 */                                         \
                                                                               \
    VINT P_right =                                                             \
        PSTEP_I(P_exp_5, R,                                                    \
                PSTEP_I(P_exp_6, R,                                            \
                        PSTEP_I(P_exp_7, R,                                    \
                                PSTEP_I(P_exp_8, R,                            \
                                        PSTEP_I(P_exp_9, P_exp_10, R, (vlen)), \
                                        (vlen)),                               \
                                (vlen)),                                       \
                        (vlen)),                                               \
                (vlen));                                                       \
    VINT R_sq = __riscv_vsmul(R, R, 1, (vlen));                                \
    VINT R_to_5 = __riscv_vsmul(R_sq, R_sq, 1, (vlen));                        \
    R_to_5 = __riscv_vsmul(R_sq, R_sq, 1, (vlen));                             \
    R_to_5 = __riscv_vsmul(R_to_5, R, 1, (vlen));                              \
    VINT P_left =                                                              \
        PSTEP_I(P_exp_0, R,                                                    \
                PSTEP_I(P_exp_1, R,                                            \
                        PSTEP_I(P_exp_2, R,                                    \
                                PSTEP_I(P_exp_3, P_exp_4, R, (vlen)), (vlen)), \
                        (vlen)),                                               \
                (vlen));                                                       \
    P_right = __riscv_vsmul(P_right, R_to_5, 1, (vlen));                       \
    P_left = __riscv_vadd(P_right, P_left, (vlen));                            \
    P_left = __riscv_vsmul(P_left, R_sq, 1, (vlen));                           \
    P_left = __riscv_vadd(P_left, R, (vlen));                                  \
    (EXP) = __riscv_vsra(P_left, 1, (vlen));                                   \
    INT ONE = (1LL) << 62;                                                     \
    (EXP) = __riscv_vadd((EXP), ONE, (vlen));                                  \
  } while (0)

// Compute the term (x/e)^(x-1/2) for 2 <= x <= 180.
// Return integer n and Q62 fixed point EXP, 2^n value_of(EXP) is (x/e)^(x-1/2)
#define STIRLING_POWER(x_hi, x_lo, n, EXP, vlen)                               \
  do {                                                                         \
    VFLOAT y_hi, y_lo;                                                         \
    TGAMMA_LOG((x_hi), (x_lo), y_hi, y_lo, (vlen));                            \
    VFLOAT x_m_half = __riscv_vfsub((x_hi), 0x1.0p-1, (vlen));                 \
    /* compute (x_m_half, x_lo) * (y_hi, y_lo) */                              \
    VFLOAT z_hi, z_lo;                                                         \
    PROD_X1Y1(x_m_half, y_hi, z_hi, z_lo, (vlen));                             \
    z_lo = __riscv_vfmacc(z_lo, x_m_half, y_lo, (vlen));                       \
    z_lo = __riscv_vfmacc(z_lo, (x_lo), y_hi, (vlen));                         \
    TGAMMA_EXP(z_hi, z_lo, (n), (EXP), (vlen));                                \
  } while (0)

// Gamma based on Stirling formula is Gamma(x) ~ (x/e)^x sqrt(2 pi / x)
// poly(1/x) To incoporate the 1/sqrt(x) into the power calculation
// (x/e)^(x-1/2) sqrt(2 pi / e ) poly(1/x)
// This poly(1/x) is in essense a correction term
#define STIRLING_CORRECTION(x_hi, x_lo, P_SC, Q_SC, vlen)                      \
  do {                                                                         \
    /* 2 <= x < 180. Use Q62 to represent 1/x in fixed point */                \
    VFLOAT y_hi = __riscv_vfrdiv((x_hi), fp_posOne, (vlen));                   \
    VFLOAT y_lo = VFMV_VF(fp_posOne, (vlen));                                  \
    y_lo = __riscv_vfnmsub((x_hi), y_hi, y_lo, (vlen));                        \
    y_lo = __riscv_vfnmsac(y_lo, (x_lo), y_hi, (vlen));                        \
    y_lo = __riscv_vfmul(y_hi, y_lo, (vlen));                                  \
    y_hi = __riscv_vfmul(y_hi, 0x1.0p62, (vlen));                              \
    y_lo = __riscv_vfmul(y_lo, 0x1.0p62, (vlen));                              \
    VINT R = __riscv_vfcvt_x(y_hi, (vlen));                                    \
    R = __riscv_vadd(R, __riscv_vfcvt_x(y_lo, (vlen)), (vlen));                \
    /* R is 1/(x_hi+x_lo) in Q62 */                                            \
    (P_SC) = PSTEP_I_SLL(P_corr_6, P_corr_7, 1, R, (vlen));                    \
    (P_SC) = PSTEP_I_SLL(P_corr_5, R, 1, (P_SC), (vlen));                      \
    (P_SC) = PSTEP_I_SLL(P_corr_4, R, 1, (P_SC), (vlen));                      \
    (P_SC) = PSTEP_I_SLL(P_corr_3, R, 1, (P_SC), (vlen));                      \
    (P_SC) = PSTEP_I_SLL(P_corr_2, R, 1, (P_SC), (vlen));                      \
    (P_SC) = PSTEP_I_SLL(P_corr_1, R, 1, (P_SC), (vlen));                      \
    (P_SC) = PSTEP_I_SLL(P_corr_0, R, 1, (P_SC), (vlen));                      \
                                                                               \
    (Q_SC) = PSTEP_I_SLL(Q_corr_6, Q_corr_7, 1, R, (vlen));                    \
    (Q_SC) = PSTEP_I_SLL(Q_corr_5, R, 1, (Q_SC), (vlen));                      \
    (Q_SC) = PSTEP_I_SLL(Q_corr_4, R, 1, (Q_SC), (vlen));                      \
    (Q_SC) = PSTEP_I_SLL(Q_corr_3, R, 1, (Q_SC), (vlen));                      \
    (Q_SC) = PSTEP_I_SLL(Q_corr_2, R, 1, (Q_SC), (vlen));                      \
    (Q_SC) = PSTEP_I_SLL(Q_corr_1, R, 1, (Q_SC), (vlen));                      \
    (Q_SC) = PSTEP_I_SLL(Q_corr_0, R, 1, (Q_SC), (vlen));                      \
  } while (0)

// When input x to gamma(x) is negative, a factor of  sin(pi x)/pi
// is needed. When x is an exact negative integer, we need to return
// +-inf as special values and also raise the divide-by-zero signal
// The input to TGAMMA_SIN is actually |x| clipped to [2^(-60), 179.5]
#define TGAMMA_SIN(x, P_SIN, SIN_scale, n, vy_special, special_args, vlen)     \
  do {                                                                         \
    VFLOAT n_flt;                                                              \
    (n) = __riscv_vfcvt_x((x), (vlen));                                        \
    n_flt = __riscv_vfcvt_f((n), (vlen));                                      \
    VFLOAT r = __riscv_vfsub((x), n_flt, (vlen));                              \
    VINT m = __riscv_vsra(F_AS_I(r), MAN_LEN, (vlen));                         \
    m = __riscv_vrsub(__riscv_vand(m, 0x7FF, (vlen)), EXP_BIAS, (vlen));       \
    /* r = 2^(-m) * val, val in [1, 2). Note that 1 <= m <= 60 */              \
    VFLOAT scale = I_AS_F(__riscv_vsll(__riscv_vadd(m, EXP_BIAS + 61, (vlen)), \
                                       MAN_LEN, (vlen)));                      \
    VINT R = __riscv_vfcvt_x(__riscv_vfmul(r, scale, (vlen)), (vlen));         \
    /* R is fixed point in scale 61+m */                                       \
    VFLOAT rsq = __riscv_vfmul(r, r, (vlen));                                  \
    VFLOAT rsq_lo = __riscv_vfmsub(r, r, rsq, (vlen));                         \
    VINT Rsq = __riscv_vfcvt_x(__riscv_vfmul(rsq, 0x1.0p63, (vlen)), (vlen));  \
    Rsq = __riscv_vadd(                                                        \
        Rsq, __riscv_vfcvt_x(__riscv_vfmul(rsq_lo, 0x1.0p63, (vlen)), (vlen)), \
        (vlen));                                                               \
    VINT P_right = PSTEP_I(                                                    \
        P_sin_4, Rsq,                                                          \
        PSTEP_I(P_sin_5, Rsq, PSTEP_I(P_sin_6, P_sin_7, Rsq, (vlen)), (vlen)), \
        (vlen));                                                               \
    VINT R8 = __riscv_vsmul(Rsq, Rsq, 1, (vlen));                              \
    R8 = __riscv_vsmul(R8, R8, 1, (vlen));                                     \
    VINT P_left = PSTEP_I(                                                     \
        P_sin_0, Rsq,                                                          \
        PSTEP_I(P_sin_1, Rsq, PSTEP_I(P_sin_2, P_sin_3, Rsq, (vlen)), (vlen)), \
        (vlen));                                                               \
    P_right = __riscv_vsmul(P_right, R8, 1, (vlen));                           \
    P_left = __riscv_vadd(P_left, P_right, (vlen));                            \
    P_left = __riscv_vsmul(P_left, Rsq, 1, (vlen));                            \
    P_left = __riscv_vsmul(P_left, R, 1, (vlen));                              \
    (P_SIN) = __riscv_vadd(R, __riscv_vsll(P_left, 1, (vlen)), (vlen));        \
    (SIN_scale) = __riscv_vadd(m, 61, (vlen));                                 \
    VBOOL pole = __riscv_vmseq(R, 0, (vlen));                                  \
    if (__riscv_vcpop(pole, (vlen)) > 0) {                                     \
      VFLOAT pm_inf = __riscv_vfrec7(pole, I_AS_F(R), (vlen));                 \
      pm_inf = __riscv_vfsgnjn(pm_inf, I_AS_F(__riscv_vsll((n), 63, (vlen))),  \
                               (vlen));                                        \
      (vy_special) = __riscv_vmerge((vy_special), pm_inf, pole, (vlen));       \
      (special_args) = __riscv_vmor((special_args), pole, (vlen));             \
      (P_SIN) = __riscv_vmerge((P_SIN), 0x8000, pole, (vlen));                 \
    }                                                                          \
  } while (0)

void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vx_orig, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    // Handle Inf and NaN and |vx| < 2^(-60)
    EXCEPTION_HANDLING_TGAMMA(vx, special_args, vy_special, vlen);
    vx_orig = vx;

    vx = __riscv_vfabs(vx, vlen);
    vx = __riscv_vfmin(vx, 0x1.67p+7, vlen);
    vx_orig = __riscv_vfsgnj(vx, vx_orig, vlen);

    VFLOAT vx_hi = vx;
    VFLOAT vx_lo = VFMV_VF(fp_posZero, vlen);
    VBOOL x_lt_0 = __riscv_vmflt(vx_orig, fp_posZero, vlen);

    // VINT P_SIN, SIN_scale, lsb;
    // TGAMMA_SIN(vx_orig, P_SIN, SIN_scale, lsb, vy_special, special_args,
    // vlen);

    if (__riscv_vcpop(x_lt_0, vlen) > 0) {
      // add 1 to argument
      VFLOAT a_tmp_hi = __riscv_vfadd(vx_hi, fp_posOne, vlen);
      VFLOAT a_tmp_lo = __riscv_vfrsub(a_tmp_hi, fp_posOne, vlen);
      a_tmp_lo = __riscv_vfadd(a_tmp_lo, vx_hi, vlen);
      vx_hi = __riscv_vmerge(vx_hi, a_tmp_hi, x_lt_0, vlen);
      vx_lo = __riscv_vmerge(vx_lo, a_tmp_lo, x_lt_0, vlen);
    }

    VINT n, EXP;

    VINT XF = __riscv_vsll(VMVI_VX(1, vlen), 61, vlen);
    VINT XF_scale = VMVI_VX(61, vlen);

    VBOOL x_lt_2 = __riscv_vmflt(vx_hi, 0x1.0p1, vlen);

    if (__riscv_vcpop(x_lt_2, vlen) > 0) {
      // So x = 2^(-m) val, val is in [1, 2)
      // Create fixed point X in scale Q(61+m)
      VINT m = __riscv_vsra(F_AS_I(vx_hi), MAN_LEN, vlen);
      m = __riscv_vrsub(__riscv_vand(m, 0x7FF, vlen), EXP_BIAS, vlen);
      // at this point, m >= 0, x = 2^(-m) val, val in [1, 2)
      VINT XF_scale_1 = __riscv_vadd(m, 61, vlen);
      VINT scale_m =
          __riscv_vsll(__riscv_vadd(XF_scale_1, EXP_BIAS, vlen), MAN_LEN, vlen);
      VFLOAT x_tmp = __riscv_vfmul(vx_hi, I_AS_F(scale_m), vlen);
      VINT X = __riscv_vfcvt_x(x_tmp, vlen);
      // X is vx_hi in fixed-point, Q(61+m)
      x_tmp = __riscv_vfmul(vx_lo, I_AS_F(scale_m), vlen);
      X = __riscv_vadd(X, __riscv_vfcvt_x(x_tmp, vlen), vlen);
      // X is (vx_hi + vx_lo) in fixed-point, Q(61+m)
      VINT One_plus_X =
          __riscv_vadd(XF, __riscv_vsra(X, I_AS_U(m), vlen), vlen);
      // One_plus_X is 1+x in Q61
      VFLOAT b = VFMV_VF(fp_posZero, vlen);

      // if 1 <= x < 2, gamma(x) = (1/x) gamma(x+1)
      // if 0 < x < 1,  gamma(x) = (1/(x(x+1))) gamma(x+2)
      VBOOL x_ge_1 = __riscv_vmfge(vx_hi, fp_posOne, vlen);
      VBOOL cond = __riscv_vmand(x_lt_2, x_ge_1, vlen);
      // cond is 1 <= x < 2
      XF_scale = __riscv_vmerge(XF_scale, XF_scale_1, cond, vlen);
      XF = __riscv_vmerge(XF, X, cond, vlen);
      b = __riscv_vfmerge(b, fp_posOne, cond, vlen);
      // at this point, if input x is between [1, 2), XF is x in scale 61+m
      // which is 61 (as m is 0).

      cond = __riscv_vmandn(x_lt_2, x_ge_1, vlen);
      // cond is 0 < x < 1
      X = __riscv_vsmul(X, One_plus_X, 1, vlen);
      XF_scale_1 = __riscv_vadd(m, 59, vlen);
      XF_scale = __riscv_vmerge(XF_scale, XF_scale_1, cond, vlen);
      XF = __riscv_vmerge(XF, X, cond, vlen);
      b = __riscv_vfmerge(b, 0x1.0p1, cond, vlen);
      // at this point, XF is either 1, x, or x(x+1) in fixed point
      // scale given in XF_scale which is either 62, 61+m, or 59+m

      // now set (vx_hi, vx_lo) to x + b, b = 0, 1, or 2
      x_tmp = __riscv_vfadd(b, vx_hi, vlen);
      VFLOAT x_tmp2 = __riscv_vfsub(b, x_tmp, vlen);
      x_tmp2 = __riscv_vfadd(x_tmp2, vx_hi, vlen);
      vx_hi = x_tmp;
      vx_lo = __riscv_vfadd(vx_lo, x_tmp2, vlen);
    }

    STIRLING_POWER(vx_hi, vx_lo, n, EXP, vlen);
    /* Stirling factor is 2^n * e, EXP is e in Q62 */

    VINT P_SC, Q_SC, Numer_tail, Denom_tail;
    STIRLING_CORRECTION(vx_hi, vx_lo, P_SC, Q_SC, vlen);
    /* correction term is 2 * P_SC / Q_SC, P_SC is Q78, Q_SC is Q79 */

    /* 2^(n-61) * EXP * P_SC / Q_SC is gamma(x) for x >= 2 */
    VINT P = __riscv_vsmul(EXP, P_SC, 1, vlen);
    /* P is Q77 */

    /* now incoporate XF into Q_SC */
    VINT Q = __riscv_vsmul(XF, Q_SC, 1, vlen);
    /* scale of Q is 79 - 63 + XF_scale  = 16 + XF_scale */

    /* difference is 16 + XF_scale - 77, which is XF_scale - 61 */
    XF_scale = __riscv_vsub(XF_scale, 61, vlen);
    n = __riscv_vadd(n, XF_scale, vlen);
    /* 2^n P / Q is the answer if input is positive */
    /* For negative input, the answer is the reciprocal times pi/sin(pi x) */

    VINT Numer = P;
    VINT Denom = Q;
    VINT vy_sign;
    vy_sign = __riscv_vxor(vy_sign, vy_sign, vlen);

    if (__riscv_vcpop(x_lt_0, vlen) > 0) {
      /* we first recipricate and change n to negative n */
      Numer = __riscv_vmerge(Numer, Q, x_lt_0, vlen);
      Denom = __riscv_vmerge(Denom, P, x_lt_0, vlen);

      VINT P_SIN, SIN_scale, lsb;
      TGAMMA_SIN(vx_orig, P_SIN, SIN_scale, lsb, vy_special, special_args,
                 vlen);

      vy_sign = __riscv_vmerge(vy_sign, lsb, x_lt_0, vlen);

      P_SIN = __riscv_vsmul(P_SIN, Denom, 1, vlen);
      Denom = __riscv_vmerge(Denom, P_SIN, x_lt_0, vlen);

      SIN_scale = __riscv_vsub(SIN_scale, 63, vlen);
      VINT n_prime = __riscv_vrsub(n, 0, vlen);
      n_prime = __riscv_vadd(n_prime, SIN_scale, vlen);
      n = __riscv_vmerge(n, n_prime, x_lt_0, vlen);
    }

    VFLOAT numer_hi, numer_lo, denom_hi, denom_lo;
    numer_hi = __riscv_vfcvt_f(Numer, vlen);
    Numer_tail = __riscv_vsub(Numer, __riscv_vfcvt_x(numer_hi, vlen), vlen);
    numer_lo = __riscv_vfcvt_f(Numer_tail, vlen);

    denom_hi = __riscv_vfcvt_f(Denom, vlen);
    Denom_tail = __riscv_vsub(Denom, __riscv_vfcvt_x(denom_hi, vlen), vlen);
    denom_lo = __riscv_vfcvt_f(Denom_tail, vlen);

    DIV_N2D2(numer_hi, numer_lo, denom_hi, denom_lo, vy, vlen);
    FAST_LDEXP(vy, n, vlen);

    vy_sign = __riscv_vsll(vy_sign, 63, vlen);
    vy = __riscv_vfsgnjx(vy, I_AS_F(vy_sign), vlen);

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
