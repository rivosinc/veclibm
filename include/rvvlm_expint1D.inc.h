// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_EXPINT1D_STD
#else
#define F_VER1 RVVLM_EXPINT1DI_STD
#endif

// expint1 exceptions are:
// sNaN, -ve values: return qNaN and invalid exception signal
// +-0: return +Inf and divide-by-zero exception signal
// +Inf: return +Inf, no exception
// positive denorm: normalize for later processing
#define EXCEPTION_HANDLING(vx, special_args, vy_special, n_adjust, vlen)       \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    IDENTIFY(vclass, 0x3BF, (special_args), (vlen));                           \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    (n_adjust) = __riscv_vxor((n_adjust), (n_adjust), (vlen));                 \
    if (nb_special_args > 0) {                                                 \
      VBOOL id_mask;                                                           \
      /* substitute negative arguments with sNaN */                            \
      IDENTIFY(vclass, class_negative, id_mask, (vlen));                       \
      (vx) = __riscv_vfmerge((vx), fp_sNaN, id_mask, vlen);                    \
      /* substitute -0 argument with +0 */                                     \
      IDENTIFY(vclass, class_negZero, id_mask, vlen);                          \
      (vx) = __riscv_vfmerge((vx), fp_posZero, id_mask, vlen);                 \
      /* eliminate positive denorm from special arguments */                   \
      IDENTIFY(vclass, 0x39F, (special_args), (vlen));                         \
      /* for narrowed set of special arguments, compute vx+vfrec7(vx) */       \
      (vy_special) = __riscv_vfrec7((special_args), (vx), (vlen));             \
      (vy_special) =                                                           \
          __riscv_vfadd((special_args), (vy_special), (vx), (vlen));           \
      (vx) = __riscv_vfmerge((vx), fp_posOne, (special_args), (vlen));         \
      /* scale up input for positive denormals */                              \
      IDENTIFY(vclass, class_posDenorm, id_mask, (vlen));                      \
      (n_adjust) = __riscv_vmerge((n_adjust), 64, id_mask, (vlen));            \
      VFLOAT vx_normalized = __riscv_vfmul(id_mask, (vx), 0x1.0p64, (vlen));   \
      (vx) = __riscv_vmerge((vx), vx_normalized, id_mask, (vlen));             \
      (vx) = __riscv_vfmerge((vx), fp_posOne, (special_args), (vlen));         \
    }                                                                          \
  } while (0)

#define NEG_LOG2_HI -0x1.62e42fefa4000p-1
#define NEG_LOG2_LO 0x1.8432a1b0e2634p-43

#define EXPINT1_NEG_LOG(vx_in, n_adjust, y_hi, y_lo, vlen)                     \
  do {                                                                         \
    /* in_arg at this point are positive, finite and not subnormal */          \
    /* Decompose in_arg into 2^n * X, where 0.75 <= X < 1.5 */                 \
    /* log(2^n X) = n * log(2)  +  log(X) */                                   \
    /* log(X) = 2 atanh((X-1)/(X+1)) */                                        \
                                                                               \
    /* Argument reduction: represent in_arg as 2^n X */                        \
    /* where 1/rt(2) <= X < rt(2) approximately */                             \
    /* Then compute 2(X-1)/(X+1) as r + delta_r. */                            \
    /* natural log, log(X) = 2 atanh(w/2)  = w + p1 w^3 + p2 w5 ...; */        \
    /* w = r+delta_r */                                                        \
    VFLOAT vx = (vx_in);                                                       \
    VINT n = U_AS_I(__riscv_vadd(                                              \
        __riscv_vsrl(F_AS_U((vx)), MAN_LEN - 8, (vlen)), 0x96, (vlen)));       \
    n = __riscv_vsra(n, 8, (vlen));                                            \
    n = __riscv_vsub(n, EXP_BIAS, (vlen));                                     \
    (vx) = I_AS_F(                                                             \
        __riscv_vsub(F_AS_I((vx)), __riscv_vsll(n, MAN_LEN, (vlen)), (vlen))); \
    n = __riscv_vsub(n, (n_adjust), (vlen));                                   \
    VFLOAT n_flt = __riscv_vfcvt_f(n, (vlen));                                 \
                                                                               \
    VFLOAT numer = __riscv_vfsub((vx), fp_posOne, (vlen));                     \
    numer = __riscv_vfadd(numer, numer, (vlen));                               \
    VFLOAT denom, delta_d;                                                     \
    denom = __riscv_vfadd((vx), fp_posOne, (vlen));                            \
    delta_d = __riscv_vfrsub(denom, fp_posOne, (vlen));                        \
    delta_d = __riscv_vfadd(delta_d, (vx), (vlen));                            \
    VFLOAT r, delta_r;                                                         \
    DIV_N1D2(numer, denom, delta_d, r, delta_r, (vlen));                       \
                                                                               \
    VFLOAT rsq = __riscv_vfmul(r, r, (vlen));                                  \
    VFLOAT rcube = __riscv_vfmul(rsq, r, (vlen));                              \
    VFLOAT r6 = __riscv_vfmul(rcube, rcube, (vlen));                           \
                                                                               \
    VFLOAT poly_right = PSTEP(0x1.c71c51c73bb7fp-12, rsq,                      \
                              PSTEP(0x1.74664bed42062p-14, rsq,                \
                                    PSTEP(0x1.39a071f83b771p-16,               \
                                          0x1.2f123764244dfp-18, rsq, (vlen)), \
                                    (vlen)),                                   \
                              (vlen));                                         \
                                                                               \
    VFLOAT poly_left =                                                         \
        PSTEP(0x1.5555555555594p-4, rsq,                                       \
              PSTEP(0x1.999999997f6b6p-7, 0x1.2492494248a48p-9, rsq, (vlen)),  \
              (vlen));                                                         \
                                                                               \
    VFLOAT poly = __riscv_vfmadd(poly_right, r6, poly_left, (vlen));           \
    delta_r = __riscv_vfmsac(delta_r, NEG_LOG2_LO, n_flt, (vlen));             \
    poly = __riscv_vfnmsub(poly, rcube, delta_r, (vlen));                      \
    /* At this point -r + poly approximates -log(X) */                         \
                                                                               \
    /* Reconstruction: -log(in_arg) is -n log(2) - log(X) computed as */       \
    /* n*(-log_2_hi - log_2_lo) - r - poly */                                  \
    /* n*log_2_hi is exact as log_2_hi has enough trailing zeros */            \
    VFLOAT A = __riscv_vfmul(n_flt, NEG_LOG2_HI, (vlen));                      \
    /* A either donimates r, or A is exactly 0 */                              \
    r = __riscv_vfsgnjx(r, fp_negOne, (vlen));                                 \
    FAST2SUM(A, r, (y_hi), (y_lo), (vlen));                                    \
    (y_lo) = __riscv_vfadd((y_lo), poly, (vlen));                              \
  } while (0)

// EXPINT1 on [0,1] is approximated by -log(x) + poly(x)
//    poly(x) = p0 + p1 x + ... + p12 x^12
//    p0, ..., p4 fixed point in Q62
#define P_near0_0 -0x24f119f8df6c31e9
#define P_near0_1 0x3fffffffffffffdc
#define P_near0_2 -0x0ffffffffffff993
#define P_near0_3 0x038e38e38e3862ef
#define P_near0_4 -0x00aaaaaaaaa516c7
//   p5, ..., p13 as floating point
#define P_near0_5 0x1.b4e81b4c194fap-10
#define P_near0_6 -0x1.e573ac379c696p-13
#define P_near0_7 0x1.db8b66c555673p-16
#define P_near0_8 -0x1.a01962ee439b6p-19
#define P_near0_9 0x1.48bd8c51ff717p-22
#define P_near0_10 -0x1.d8dbdf85f5051p-26
#define P_near0_11 0x1.355966c463c2ap-29
#define P_near0_12 -0x1.5f2978a4477ccp-33
#define P_near0_13 0x1.0c38e425fee47p-37

#define EXPINT_POLY_01(vx, y_hi, y_lo, vlen)                                   \
  do {                                                                         \
    /* Compute leading poly in fixed point */                                  \
    VINT P_FIX, X;                                                             \
    X = __riscv_vfcvt_x(__riscv_vfmul((vx), 0x1.0p63, (vlen)), (vlen));        \
    P_FIX = PSTEP_I(                                                           \
        P_near0_0, X,                                                          \
        PSTEP_I(P_near0_1, X,                                                  \
                PSTEP_I(P_near0_2, X,                                          \
                        PSTEP_I(P_near0_3, P_near0_4, X, (vlen)), (vlen)),     \
                (vlen)),                                                       \
        (vlen));                                                               \
    VFLOAT p_left_hi, p_left_lo;                                               \
    p_left_hi = __riscv_vfcvt_f(P_FIX, (vlen));                                \
    P_FIX = __riscv_vsub(P_FIX, __riscv_vfcvt_x(p_left_hi, (vlen)), (vlen));   \
    p_left_lo = __riscv_vfcvt_f(P_FIX, (vlen));                                \
    p_left_lo = __riscv_vfmul(p_left_lo, 0x1.0p-62, (vlen));                   \
    p_left_hi = __riscv_vfmul(p_left_hi, 0x1.0p-62, (vlen));                   \
                                                                               \
    VFLOAT poly_mid, poly_right;                                               \
    poly_right = PSTEP(                                                        \
        P_near0_9, (vx),                                                       \
        PSTEP(P_near0_10, (vx),                                                \
              PSTEP(P_near0_11, (vx),                                          \
                    PSTEP(P_near0_12, P_near0_13, (vx), (vlen)), (vlen)),      \
              (vlen)),                                                         \
        (vlen));                                                               \
                                                                               \
    poly_mid = PSTEP(P_near0_5, (vx),                                          \
                     PSTEP(P_near0_6, (vx),                                    \
                           PSTEP(P_near0_7, P_near0_8, (vx), (vlen)), (vlen)), \
                     ((vlen)));                                                \
    VFLOAT x4, x5;                                                             \
    x4 = __riscv_vfmul((vx), (vx), (vlen));                                    \
    x4 = __riscv_vfmul(x4, x4, (vlen));                                        \
    x5 = __riscv_vfmul((vx), x4, (vlen));                                      \
    poly_mid = __riscv_vfmacc(poly_mid, x4, poly_right, (vlen));               \
    p_left_lo = __riscv_vfmacc(p_left_lo, x5, poly_mid, (vlen));               \
    KNUTH2SUM(p_left_hi, p_left_lo, (y_hi), (y_lo), (vlen));                   \
  } while (0)

// Rational function deg-11 for x >= 1
// expint1(x) ~= exp(-x) * y * P(y)/Q(y), y=1/x
#define P_0 0x01cd7ed8aff2c99a  // Q fmt 89
#define P_1 0x569fe822aee57cb5  // Q fmt 89
#define P_2 0x066ef748e71155e7  // Q fmt 81
#define P_3 0x3eef1f3e5518e60c  // Q fmt 81
#define P_4 0x0565f4de088a3f6f  // Q fmt 75
#define P_5 0x110eb5a49f6eb3f7  // Q fmt 75
#define P_6 0x1eb3d58b6063612a  // Q fmt 75
#define P_7 0x1e310833e20e2c95  // Q fmt 75
#define P_8 0x0ef22d4e45dc890f  // Q fmt 75
#define P_9 0x0332908f3ee32e50  // Q fmt 75
#define P_10 0x00363991b63aebbe // Q fmt 75
#define P_11 0x00003fcc5b1fe05f // Q fmt 75
#define delta_P_0 -0x1.991be2150638dp-91

#define Q_0 0x01cd7ed8aff2c99a  // Q fmt 89
#define Q_1 0x586d66fb5ed84240  // Q fmt 89
#define Q_2 0x06c3c9b231105002  // Q fmt 81
#define Q_3 0x450cdf1ba384745a  // Q fmt 81
#define Q_4 0x0325d39f7df69cef  // Q fmt 74
#define Q_5 0x0adb448d52de9c87  // Q fmt 74
#define Q_6 0x162a2f5f98fba589  // Q fmt 74
#define Q_7 0x1a29d73cbd365659  // Q fmt 74
#define Q_8 0x10f713cf2428b2ff  // Q fmt 74
#define Q_9 0x0582b960921c6dee  // Q fmt 74
#define Q_10 0x00c13a848700a0a5 // Q fmt 74
#define Q_11 0x0007802b6e574e3e // Q fmt 74
#define delta_Q_0 0x1.91d57a67cdde2p-91

// Compute p(y)/q(y), y = 1/x
// Computation done in fixed point. Y62 is 1/x in Q62 format
// On return rat_hi and rat_lo are floating-point values
#define EXPINT1_RAT_GE1(x_hi, x_lo, scale, rat_hi, rat_lo, vlen)               \
  do {                                                                         \
    VINT P75, P81, P89, Q74, Q81, Q89, Ysq, Y4;                                \
    VINT _X;                                                                   \
    FLT2FIX((x_hi), (x_lo), (scale), _X, (vlen));                              \
    P75 = PSTEP_I_SLL(P_10, P_11, 1, _X, (vlen));                              \
    P75 = PSTEP_I_SLL(P_9, _X, 1, P75, (vlen));                                \
    P75 = PSTEP_I_SLL(P_8, _X, 1, P75, (vlen));                                \
    P75 = PSTEP_I_SLL(P_7, _X, 1, P75, (vlen));                                \
    P75 = PSTEP_I_SLL(P_6, _X, 1, P75, (vlen));                                \
    P75 = PSTEP_I_SLL(P_5, _X, 1, P75, (vlen));                                \
    P75 = PSTEP_I_SLL(P_4, _X, 1, P75, (vlen));                                \
    VFLOAT _xsq_hi, _xsq_lo;                                                   \
    SQR_X2((x_hi), (x_lo), _xsq_hi, _xsq_lo, (vlen));                          \
    VFLOAT _p_right_hi, _p_right_lo;                                           \
    FIX2FLT(P75, 0x1.0p-75, _p_right_hi, _p_right_lo, (vlen));                 \
    VFLOAT _p_tmp1_hi, _p_tmp1_lo;                                             \
    PROD_X2Y2(_xsq_hi, _xsq_lo, _p_right_hi, _p_right_lo, _p_tmp1_hi,          \
              _p_tmp1_lo, (vlen));                                             \
                                                                               \
    P81 = PSTEP_I_SLL(P_2, P_3, 1, _X, (vlen));                                \
    VFLOAT _p_mid_hi, _p_mid_lo;                                               \
    FIX2FLT(P81, 0x1.0p-81, _p_mid_hi, _p_mid_lo, (vlen));                     \
    VFLOAT _p_tmp2_hi, _p_tmp2_lo;                                             \
    POS2SUM(_p_tmp1_hi, _p_mid_hi, _p_tmp2_hi, _p_tmp2_lo, (vlen));            \
    _p_tmp2_lo = __riscv_vfadd(_p_tmp2_lo, _p_tmp1_lo, (vlen));                \
    _p_tmp2_lo = __riscv_vfadd(_p_tmp2_lo, _p_mid_lo, (vlen));                 \
                                                                               \
    PROD_X2Y2(_xsq_hi, _xsq_lo, _p_tmp2_hi, _p_tmp2_lo, _p_tmp1_hi,            \
              _p_tmp1_lo, (vlen));                                             \
    VFLOAT _p_left_hi, _p_left_lo;                                             \
    P89 = PSTEP_I_SLL(P_0, P_1, 1, _X, (vlen));                                \
    FIX2FLT(P89, 0x1.0p-89, _p_left_hi, _p_left_lo, (vlen));                   \
    POS2SUM(_p_left_hi, _p_tmp1_hi, _p_tmp2_hi, _p_tmp2_lo, (vlen));           \
    _p_tmp2_lo = __riscv_vfadd(_p_tmp2_lo, _p_left_lo, (vlen));                \
    _p_tmp2_lo = __riscv_vfadd(_p_tmp2_lo, _p_tmp1_lo, (vlen));                \
    _p_tmp2_lo = __riscv_vfadd(_p_tmp2_lo, delta_P_0, (vlen));                 \
    VFLOAT _p_hi, _p_lo;                                                       \
    FAST2SUM(_p_tmp2_hi, _p_tmp2_lo, _p_hi, _p_lo, (vlen));                    \
    /* (_p_hi, _p_lo) is an accurate version of p(x) */                        \
                                                                               \
    VFLOAT AA, aa;                                                             \
    Q74 = PSTEP_I_SLL(Q_10, Q_11, 1, _X, (vlen));                              \
    Q74 = PSTEP_I_SLL(Q_9, _X, 1, Q74, (vlen));                                \
    Q74 = PSTEP_I_SLL(Q_8, _X, 1, Q74, (vlen));                                \
    Q74 = PSTEP_I_SLL(Q_7, _X, 1, Q74, (vlen));                                \
    Q74 = PSTEP_I_SLL(Q_6, _X, 1, Q74, (vlen));                                \
    Q74 = PSTEP_I_SLL(Q_5, _X, 1, Q74, (vlen));                                \
    Q74 = PSTEP_I_SLL(Q_4, _X, 1, Q74, (vlen));                                \
                                                                               \
    VFLOAT _q_right_hi, _q_right_lo;                                           \
    FIX2FLT(Q74, 0x1.0p-74, _q_right_hi, _q_right_lo, (vlen));                 \
    VFLOAT _q_tmp1_hi, _q_tmp1_lo;                                             \
    PROD_X2Y2(_xsq_hi, _xsq_lo, _q_right_hi, _q_right_lo, _q_tmp1_hi,          \
              _q_tmp1_lo, (vlen));                                             \
                                                                               \
    Q81 = PSTEP_I_SLL(Q_2, Q_3, 1, _X, (vlen));                                \
    VFLOAT _q_mid_hi, _q_mid_lo;                                               \
    FIX2FLT(Q81, 0x1.0p-81, _q_mid_hi, _q_mid_lo, (vlen));                     \
    VFLOAT _q_tmp2_hi, _q_tmp2_lo;                                             \
    POS2SUM(_q_tmp1_hi, _q_mid_hi, _q_tmp2_hi, _q_tmp2_lo, (vlen));            \
    _q_tmp2_lo = __riscv_vfadd(_q_tmp2_lo, _q_tmp1_lo, (vlen));                \
    _q_tmp2_lo = __riscv_vfadd(_q_tmp2_lo, _q_mid_lo, (vlen));                 \
                                                                               \
    PROD_X2Y2(_xsq_hi, _xsq_lo, _q_tmp2_hi, _q_tmp2_lo, _q_tmp1_hi,            \
              _q_tmp1_lo, (vlen));                                             \
    VFLOAT _q_left_hi, _q_left_lo;                                             \
    Q89 = PSTEP_I_SLL(Q_0, Q_1, 1, _X, (vlen));                                \
    FIX2FLT(Q89, 0x1.0p-89, _q_left_hi, _q_left_lo, (vlen));                   \
    POS2SUM(_q_left_hi, _q_tmp1_hi, _q_tmp2_hi, _q_tmp2_lo, (vlen));           \
    _q_tmp2_lo = __riscv_vfadd(_q_tmp2_lo, _q_left_lo, (vlen));                \
    _q_tmp2_lo = __riscv_vfadd(_q_tmp2_lo, _q_tmp1_lo, (vlen));                \
    _q_tmp2_lo = __riscv_vfadd(_q_tmp2_lo, delta_Q_0, (vlen));                 \
    VFLOAT _q_hi, _q_lo;                                                       \
    FAST2SUM(_q_tmp2_hi, _q_tmp2_lo, _q_hi, _q_lo, (vlen));                    \
    /* deliver the final rat_hi, rat_lo */                                     \
    DIV2_N2D2(_p_hi, _p_lo, _q_hi, _q_lo, (rat_hi), (rat_lo), (vlen));         \
  } while (0)

// exp(x) for x in [-log/2, log2/2], deg {deg}
// the coefficients are scaled up by 2^62
#define P_one 0x4000000000000000
#define P_half 0x2000000000000000
#define P_exp_0 0x1.0000000000000p+62
#define P_exp_1 0x1.0000000000000p+62
#define P_exp_2 0x1.0000000000000p+61

#define P_exp_3 0x1.555555555555ap+59
#define P_exp_4 0x1.5555555555533p+57

#define P_exp_5 0x1.111111110ef1dp+55
#define P_exp_6 0x1.6c16c16c23cabp+52
#define P_exp_7 0x1.a01a01b2eeafdp+49
#define P_exp_8 0x1.a01a016c97838p+46

#define P_exp_9 0x1.71ddf0af3f3a4p+43
#define P_exp_10 0x1.27e542d471a01p+40
#define P_exp_11 0x1.af6bfc694314ap+36
#define P_exp_12 0x1.1ef1a5cf633bap+33

#define LOG2_HI 0x1.62e42fefa39efp-1
#define LOG2_LO 0x1.abc9e3b39803fp-56
#define NEG_LOG2_INV -0x1.71547652b82fep+0

// compute exp(-x) as 2^n(y_hi + y_lo)
#define EXP_NEGX(vx, n, y_hi, y_lo, vlen)                                      \
  do {                                                                         \
    VFLOAT _n_flt = __riscv_vfmul((vx), NEG_LOG2_INV, (vlen));                 \
    (n) = __riscv_vfcvt_x(_n_flt, (vlen));                                     \
    _n_flt = __riscv_vfcvt_f((n), (vlen));                                     \
    VFLOAT _r_hi = __riscv_vfnmadd(_n_flt, LOG2_HI, (vx), (vlen));             \
    VFLOAT _r_lo = __riscv_vfmul(_n_flt, LOG2_LO, (vlen));                     \
    /* _r_hi - _r_lo is _r */                                                  \
    VFLOAT _r = __riscv_vfsub(_r_hi, _r_lo, (vlen));                           \
    _r_lo = __riscv_vfsgnjx(_r_lo, fp_negOne, (vlen));                         \
    VFLOAT _p_right, _p_mid;                                                   \
    _p_right = PSTEP(                                                          \
        P_exp_9, _r,                                                           \
        PSTEP(P_exp_10, _r, PSTEP(P_exp_11, P_exp_12, _r, (vlen)), (vlen)),    \
        (vlen));                                                               \
    _p_mid =                                                                   \
        PSTEP(P_exp_5, _r,                                                     \
              PSTEP(P_exp_6, _r, PSTEP(P_exp_7, P_exp_8, _r, (vlen)), (vlen)), \
              (vlen));                                                         \
    VFLOAT _rsq, _r4;                                                          \
    _rsq = __riscv_vfmul(_r, _r, (vlen));                                      \
    _r4 = __riscv_vfmul(_rsq, _rsq, (vlen));                                   \
    _p_mid = __riscv_vfmacc(_p_mid, _r4, _p_right, (vlen));                    \
    VFLOAT _p_left = PSTEP(P_exp_3, P_exp_4, _r, (vlen));                      \
    _p_left = __riscv_vfmacc(_p_left, _rsq, _p_mid, (vlen));                   \
    VINT _P = __riscv_vfcvt_x(_p_left, (vlen));                                \
    VINT _R;                                                                   \
    FLT2FIX(_r_hi, _r_lo, 0x1.0p63, _R, (vlen));                               \
    _P = PSTEP_I(P_one, _R,                                                    \
                 PSTEP_I(P_one, _R, PSTEP_I(P_half, _R, _P, (vlen)), (vlen)),  \
                 (vlen));                                                      \
    FIX2FLT(_P, 0x1.0p-62, (y_hi), (y_lo), (vlen));                            \
  } while (0)

void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vx_sign, vy, vy_special;
  VBOOL special_args;
  VINT n_adjust;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    // Handle Inf, NaN, +-0, -ve, and positive denormals
    EXCEPTION_HANDLING(vx, special_args, vy_special, n_adjust, vlen);

    // Compute for 0 < x < 1 if such arguments exist
    VBOOL args_lt_1 = __riscv_vmflt(vx, fp_posOne, vlen);
    VBOOL args_ge_1 = __riscv_vmnot(args_lt_1, vlen);
    VFLOAT vy_xlt1;
    if (__riscv_vcpop(args_lt_1, vlen) > 0) {
      VFLOAT vx_lt_1 = __riscv_vfmerge(vx, 0x1.0p-1, args_ge_1, vlen);
      VFLOAT neg_logx_hi, neg_logx_lo;
      EXPINT1_NEG_LOG(vx_lt_1, n_adjust, neg_logx_hi, neg_logx_lo, vlen);
      VFLOAT poly_hi, poly_lo;
      EXPINT_POLY_01(vx_lt_1, poly_hi, poly_lo, vlen);

      VFLOAT AA, aa;
      KNUTH2SUM(neg_logx_hi, poly_hi, AA, aa, vlen);
      aa = __riscv_vfadd(aa, neg_logx_lo, vlen);
      aa = __riscv_vfadd(aa, poly_lo, vlen);
      vy_xlt1 = __riscv_vfadd(AA, aa, vlen);
    }
    VFLOAT vy_xge1;
    if (__riscv_vcpop(args_ge_1, vlen) > 0) {
      VFLOAT vx_ge_1 = __riscv_vfmerge(vx, 0x1.0p1, args_lt_1, vlen);
      // suffices to clip at 750.0
      vx_ge_1 = __riscv_vfmin(vx_ge_1, 0x1.77p+9, vlen);
      VFLOAT recip_x_hi, recip_x_lo;
      recip_x_hi = __riscv_vfrdiv(vx_ge_1, fp_posOne, vlen);
      recip_x_lo = VFMV_VF(fp_posOne, vlen);
      recip_x_lo = __riscv_vfnmsac(recip_x_lo, vx_ge_1, recip_x_hi, vlen);
      recip_x_lo = __riscv_vfmul(recip_x_hi, recip_x_lo, vlen);
      VFLOAT rat_hi, rat_lo;
      EXPINT1_RAT_GE1(recip_x_hi, recip_x_lo, 0x1.0p62, rat_hi, rat_lo, vlen);
      // (rat_hi, rat_lo) approximates expint1(x)*exp(x)*x
      // so we need to multiply (rat_hi, rat_lo) by (recip_x_hi, recip_x_lo)
      // and exp(-x)
      VFLOAT rat_by_x_hi, rat_by_x_lo;
      PROD_X2Y2(recip_x_hi, recip_x_lo, rat_hi, rat_lo, rat_by_x_hi,
                rat_by_x_lo, vlen);
      VFLOAT exp_negx_hi, exp_negx_lo;
      VINT n;
      EXP_NEGX(vx_ge_1, n, exp_negx_hi, exp_negx_lo, vlen);
      VFLOAT result_hi, result_lo;
      PROD_X2Y2(rat_by_x_hi, rat_by_x_lo, exp_negx_hi, exp_negx_lo, result_hi,
                result_lo, vlen);
      vy_xge1 = __riscv_vfadd(result_hi, result_lo, vlen);
      FAST_LDEXP(vy_xge1, n, vlen);
    }
    vy = __riscv_vmerge(vy_xlt1, vy_xge1, args_ge_1, vlen);
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
