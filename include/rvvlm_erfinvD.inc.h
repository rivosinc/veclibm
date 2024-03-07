// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ERFINVD_STD
#else
#define F_VER1 RVVLM_ERFINVDI_STD
#endif

// Two regions of approximation: left is [0, 0x1.7p-1) and right is [0x1.7p-1,
// 1) Both are done with rational functions. For left, x*P(x)/Q(x) x in [0,
// 0x1.7p-1) For right, y*P(t)/Q(t), y = sqrt(-log(1-x)); and t = 1/y

// P_coefficients in asending order, all in Q79. p0_delta is in floating point
#define P_left_0 -0x48dbe9f5b3eabaa
#define P_left_1 -0xb35279f1a626ae5
#define P_left_2 0x33789911873d184a
#define P_left_3 -0x1bf9138fc77c0fbf
#define P_left_4 -0x2d4ec43bc48403d4
#define P_left_5 0x2d61deb53842cca1
#define P_left_6 0x23324eca6b3ff02
#define P_left_7 -0xd4ec1d31542c4fc
#define P_left_8 0x2ecf3c60308b0f2
#define P_left_9 0x7c917b3378071e
#define P_left_10 -0x1e09b597f226ca
#define DELTA_P0_left -0x1.ec7dc41c17860p-2

// Q_coefficients in asending order, all in Q79. q0_delta is in floating point
#define Q_left_0 -0x52366e5b14c0970
#define Q_left_1 -0xca57e95abcc599b
#define Q_left_2 0x3b6c91ec67f5759c
#define Q_left_3 -0x1c40d5daa3be22bc
#define Q_left_4 -0x41f11eb5d837386c
#define Q_left_5 0x3c6ce478fcd75c9a
#define Q_left_6 0xbb1cd7270cfba1d
#define Q_left_7 -0x1988a4116498f1af
#define Q_left_8 0x44dc3042f103d20
#define Q_left_9 0x2390e683d02edf3
#define Q_left_10 -0x8ec66f2a7e410c
#define DELTA_Q0_left -0x1.29a0161e99446p-3

// P_coefficients in asending order, all in Q67. p0_delta is in floating point
#define P_right_0 0x17a0bb69321df
#define P_right_1 0x402eb416ae6015
#define P_right_2 0x2973eb18028ce34
#define P_right_3 0x8034a7ece1d5370
#define P_right_4 0xa76c08a74dae273
#define P_right_5 0x11dd3876b83dd078
#define P_right_6 0xfdd7693c3b77653
#define P_right_7 0xb33d66152b3c223
#define P_right_8 0x5a564c28c6a41a9
#define P_right_9 0x1190449fe630213
#define P_right_10 -0x659c784274e1
#define DELTA_P0_right -0x1.d622f4cbe0eeep-2

// Q_coefficients in asending order, all in Q67. q0_delta is in floating point
#define Q_right_0 0x17a09aabf9cee
#define Q_right_1 0x4030b9059ffcad
#define Q_right_2 0x29b26b0d87f7855
#define Q_right_3 0x87572a13d3fa2dd
#define Q_right_4 0xd7a728b5620ac3c
#define Q_right_5 0x1754392b473fd439
#define Q_right_6 0x1791b9a091a816c2
#define Q_right_7 0x167f71db9e13b075
#define Q_right_8 0xcb9f5f3e5e618a4
#define Q_right_9 0x68271fae767c68e
#define Q_right_10 0x13745c4fa224b25
#define DELTA_Q0_right 0x1.f7e7557a34ae6p-2

void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vx_orig, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);
    vx_orig = vx;

    // Handle Inf and NaN
    EXCEPTION_HANDLING_ERFINV(vx, special_args, vy_special, vlen);

    vx = __riscv_vfsgnj(vx, fp_posOne, vlen);
    VBOOL x_in_right = __riscv_vmfge(vx, 0x1.7p-1, vlen);

    VFLOAT w_hi, w_lo, w_hi_right, w_lo_right, y_hi, y_lo;
    VINT T, T_right;

    if (__riscv_vcpop(x_in_right, vlen) > 0) {
      VFLOAT one_minus_x;
      one_minus_x = __riscv_vfrsub(vx, fp_posOne, vlen);

      VINT n_adjust;
      n_adjust = __riscv_vxor(n_adjust, n_adjust, vlen);

      NEG_LOGX_4_TRANSFORM(one_minus_x, n_adjust, y_hi, y_lo, vlen);

      SQRTX_4_TRANSFORM(y_hi, y_lo, w_hi_right, w_lo_right, T_right, 0x1.0p63,
                        0x1.0p-63, vlen);
    }
    T = __riscv_vfcvt_x(__riscv_vfmul(vx, 0x1.0p63, vlen), vlen);
    T = __riscv_vmerge(T, T_right, x_in_right, vlen);

    w_hi = vx;
    w_lo = I_AS_F(__riscv_vxor(F_AS_I(w_lo), F_AS_I(w_lo), vlen));
    w_hi = __riscv_vmerge(w_hi, w_hi_right, x_in_right, vlen);
    w_lo = __riscv_vmerge(w_lo, w_lo_right, x_in_right, vlen);

    // For transformed branch, compute (w_hi + w_lo) * P(T)/Q(T)
    VINT P, Q;

    P = __riscv_vmerge(VMVI_VX(P_left_10, vlen), P_right_10, x_in_right, vlen);
    P = PSTEP_I_ab(x_in_right, P_right_6, P_left_6, T,
                   PSTEP_I_ab(x_in_right, P_right_7, P_left_7, T,
                              PSTEP_I_ab(x_in_right, P_right_8, P_left_8, T,
                                         PSTEP_I_ab(x_in_right, P_right_9,
                                                    P_left_9, T, P, vlen),
                                         vlen),
                              vlen),
                   vlen);

    Q = __riscv_vmerge(VMVI_VX(Q_left_10, vlen), Q_right_10, x_in_right, vlen);
    Q = PSTEP_I_ab(x_in_right, Q_right_6, Q_left_6, T,
                   PSTEP_I_ab(x_in_right, Q_right_7, Q_left_7, T,
                              PSTEP_I_ab(x_in_right, Q_right_8, Q_left_8, T,
                                         PSTEP_I_ab(x_in_right, Q_right_9,
                                                    Q_left_9, T, Q, vlen),
                                         vlen),
                              vlen),
                   vlen);

    P = PSTEP_I_ab(
        x_in_right, P_right_0, P_left_0, T,
        PSTEP_I_ab(
            x_in_right, P_right_1, P_left_1, T,
            PSTEP_I_ab(x_in_right, P_right_2, P_left_2, T,
                       PSTEP_I_ab(x_in_right, P_right_3, P_left_3, T,
                                  PSTEP_I_ab(x_in_right, P_right_4, P_left_4, T,
                                             PSTEP_I_ab(x_in_right, P_right_5,
                                                        P_left_5, T, P, vlen),
                                             vlen),
                                  vlen),
                       vlen),
            vlen),
        vlen);

    Q = PSTEP_I_ab(
        x_in_right, Q_right_0, Q_left_0, T,
        PSTEP_I_ab(
            x_in_right, Q_right_1, Q_left_1, T,
            PSTEP_I_ab(x_in_right, Q_right_2, Q_left_2, T,
                       PSTEP_I_ab(x_in_right, Q_right_3, Q_left_3, T,
                                  PSTEP_I_ab(x_in_right, Q_right_4, Q_left_4, T,
                                             PSTEP_I_ab(x_in_right, Q_right_5,
                                                        Q_left_5, T, Q, vlen),
                                             vlen),
                                  vlen),
                       vlen),
            vlen),
        vlen);

    VFLOAT p_hi, p_lo;
    p_hi = __riscv_vfcvt_f(P, vlen);

    p_lo = __riscv_vfcvt_f(__riscv_vsub(P, __riscv_vfcvt_x(p_hi, vlen), vlen),
                           vlen);
    VFLOAT delta_p0 = VFMV_VF(DELTA_P0_left, vlen);
    delta_p0 = __riscv_vfmerge(delta_p0, DELTA_P0_right, x_in_right, vlen);
    p_lo = __riscv_vfadd(p_lo, delta_p0, vlen);

    VFLOAT q_hi, q_lo;
    q_hi = __riscv_vfcvt_f(Q, vlen);
    q_lo = __riscv_vfcvt_f(__riscv_vsub(Q, __riscv_vfcvt_x(q_hi, vlen), vlen),
                           vlen);
    VFLOAT delta_q0 = VFMV_VF(DELTA_Q0_left, vlen);
    delta_q0 = __riscv_vfmerge(delta_q0, DELTA_Q0_right, x_in_right, vlen);
    q_lo = __riscv_vfadd(q_lo, delta_q0, vlen);

    // (y_hi, y_lo) <-- (w_hi + w_lo) * (p_hi + p_lo)
    y_hi = __riscv_vfmul(w_hi, p_hi, vlen);
    y_lo = __riscv_vfmsub(w_hi, p_hi, y_hi, vlen);
    y_lo = __riscv_vfmacc(y_lo, w_hi, p_lo, vlen);
    y_lo = __riscv_vfmacc(y_lo, w_lo, p_hi, vlen);

    DIV_N2D2(y_hi, y_lo, q_hi, q_lo, w_hi, vlen);

    vy = w_hi;

    vy = __riscv_vfsgnj(vy, vx_orig, vlen);
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);
    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
