// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ERFD_STD
#else
#define F_VER1 RVVLM_ERFDI_STD
#endif

// T is 2.0
#define T 0x1.0p+1

// For x in [0, T] odd-polynomial
// coefficients P_1 to P_17 are in fixed-point
// scaled so that they have high precision
#define P_1 0x120dd750429b6d0f  // Q60
#define P_3 -0x1812746b0379e00c // Q62
#define P_5 0x1ce2f21a04292b5f  // Q64
#define P_7 -0x1b82ce31281b38e1 // Q66
#define P_9 0x1565bcd0dd0bcd58  // Q68
#define P_11 -0xe016d9f815a019d // Q70
#define P_13 0x7e68c976c0ebcdc  // Q72
#define P_15 -0x3e9a49c76e6ee9a // Q74
#define P_17 0x1b9e64a589f8da9  // Q76
#define P_19 -0x1.5f70cd90f1878p-23
#define P_21 0x1.fca2b5f17c85ap-27
#define P_23 -0x1.514eafaeffc30p-30
#define P_25 0x1.9b3583b6b826dp-34
#define P_27 -0x1.c97ffcf4f4e22p-38
#define P_29 0x1.c2f4a46d3297dp-42
#define P_31 -0x1.6ef3c7000b58bp-46
#define P_33 0x1.ac36453182837p-51
#define P_35 -0x1.0482966738f0ep-56

// For x in (T, 6.0], general polynomial
// Coefficients Q_0 through Q_8 are in fixed points
#define Q_0 0xffff87b6641370f   // Q60
#define Q_1 -0x9062a79f9b29022  // Q62
#define Q_2 -0x11dc7e40e4efb77d // Q64
#define Q_3 -0x1dd1004e1f59ed4  // Q66
#define Q_4 0x1980c051527d41e7  // Q68
#define Q_5 0x902cddcb829790b   // Q70
#define Q_6 -0x33d6f572cdbfa228 // Q72
#define Q_7 0x425f9974bef87221  // Q74
#define Q_8 -0x5363e91dfca5d4df // Q76
#define Q_9 0x1.b5eea4ad8cdbfp-16
#define Q_10 -0x1.ded0a34468c8cp-18
#define Q_11 0x1.af4968b4d634ap-20
#define Q_12 -0x1.51de51c57f11ap-22
#define Q_13 0x1.cbbf535e64b65p-25
#define Q_14 -0x1.025a03d4fdf7bp-27
#define Q_15 0x1.c735f1e16e8cdp-31
#define Q_16 -0x1.2de00f5eeee49p-34
#define Q_17 0x1.219bdcb68d070p-38
#define Q_18 -0x1.7b5fc54357bcfp-43
#define Q_19 0x1.301ac8caec6e3p-48
#define Q_20 -0x1.c3232aa28d427p-55

// The error function erf is an odd function: erf(-x) = -erf(x)
// and thus we compute erf(|x|) and restore the sign at the end.
// For x >= 6, erf(x) rounds to 1.0
// The algorithm uses two approximation methods on [0, T], and
// (T, 6.]. For the first region, we approximate with an odd
// polynomial. For the second region, the polynomial used actually
// approximates (erfc(x))^(1/8). The desired result is 1 - (poly(x))^8
// Some algorithm for erf approximates log(erfc(x)) for x large. But
// this requires an evaluation of expm1(y) after the polynomial approximation.
// We essentially replaced the the cost of expm1 with 3 multiplications.
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
    EXCEPTION_HANDLING_ERF(vx, special_args, vy_special, vlen);

    // At this point, vx is 0 or >= 2^(-30). Can saturate vx at 6.0
    vx = __riscv_vfsgnj(vx, fp_posOne, vlen);
    vx = __riscv_vfmin(vx, 0x1.8p+2, vlen);

    VBOOL x_gt_T = __riscv_vmfgt(vx, T, vlen);
    VFLOAT r, delta_r, xsq;
    xsq = __riscv_vfmul(vx, vx, vlen);
    r = __riscv_vmerge(xsq, vx, x_gt_T, vlen);
    delta_r = I_AS_F(__riscv_vxor(F_AS_I(delta_r), F_AS_I(delta_r), vlen));
    delta_r = __riscv_vmerge(__riscv_vfmsub(vx, vx, xsq, vlen), delta_r, x_gt_T,
                             vlen);

    // Compute polynomial in r.
    // For x in [0, T], r = x*x
    // the polynomial in r is x*(p_1 + p_3 r + p_5 r^2 ... + p_35 r^22)
    // For x in (T, 6], r = x
    // the polynomial in r is q_0 + q_1 r + q_2 r^2 + ... + q_20 r^20
    // The higher order of the polynomial is computed in floating point;
    // the lower order part (more significant) are then done in fixed point
    // Both lower parts have 17 coefficients and so can be done with the
    // exact instruction sequence using the corresponding coefficients

    VFLOAT poly = PSTEP(Q_18, r, PSTEP(Q_19, Q_20, r, vlen), vlen);

    VFLOAT poly_right;
    poly_right =
        I_AS_F(__riscv_vxor(F_AS_I(poly_right), F_AS_I(poly_right), vlen));
    poly_right = __riscv_vmerge(poly_right, poly, x_gt_T, vlen);

    poly_right = PSTEP_ab(x_gt_T, Q_17, P_35, r, poly_right, vlen);
    poly_right = PSTEP_ab(x_gt_T, Q_16, P_33, r, poly_right, vlen);
    poly_right = PSTEP_ab(x_gt_T, Q_15, P_31, r, poly_right, vlen);
    poly_right = PSTEP_ab(x_gt_T, Q_14, P_29, r, poly_right, vlen);
    poly_right = PSTEP_ab(x_gt_T, Q_13, P_27, r, poly_right, vlen);

    VFLOAT r4 = __riscv_vfmul(r, r, vlen);
    r4 = __riscv_vfmul(r4, r4, vlen);

    VFLOAT poly_left = VFMV_VF(P_25, vlen);
    poly_left = __riscv_vfmerge(poly_left, Q_12, x_gt_T, vlen);
    poly_left = PSTEP_ab(x_gt_T, Q_11, P_23, r, poly_left, vlen);
    poly_left = PSTEP_ab(x_gt_T, Q_10, P_21, r, poly_left, vlen);
    poly_left = PSTEP_ab(x_gt_T, Q_9, P_19, r, poly_left, vlen);

    poly = __riscv_vfmadd(poly_right, r4, poly_left, vlen);
    VINT POLY = __riscv_vfcvt_x(__riscv_vfmul(poly, 0x1.0p78, vlen), vlen);

    VINT R = __riscv_vfcvt_x(__riscv_vfmul(r, 0x1.0p60, vlen), vlen);
    VINT D_R = __riscv_vfcvt_x(__riscv_vfmul(delta_r, 0x1.0p60, vlen), vlen);
    R = __riscv_vadd(R, D_R, vlen);
    // POLY is in Q78, R is in Q60

    VINT COEFF = __riscv_vmerge(VMVI_VX(P_17, vlen), Q_8, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q76

    COEFF = __riscv_vmerge(VMVI_VX(P_15, vlen), Q_7, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q74

    COEFF = __riscv_vmerge(VMVI_VX(P_13, vlen), Q_6, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q72

    COEFF = __riscv_vmerge(VMVI_VX(P_11, vlen), Q_5, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q70

    COEFF = __riscv_vmerge(VMVI_VX(P_9, vlen), Q_4, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q68

    COEFF = __riscv_vmerge(VMVI_VX(P_7, vlen), Q_3, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q66

    COEFF = __riscv_vmerge(VMVI_VX(P_5, vlen), Q_2, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q64

    COEFF = __riscv_vmerge(VMVI_VX(P_3, vlen), Q_1, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q62

    COEFF = __riscv_vmerge(VMVI_VX(P_1, vlen), Q_0, x_gt_T, vlen);
    POLY = __riscv_vsll(__riscv_vsmul(R, POLY, 1, vlen), 1, vlen);
    POLY = __riscv_vadd(POLY, COEFF, vlen); // Q60

    VINT POLY_RIGHT = __riscv_vsll(POLY, 3, vlen); // Q63
    POLY_RIGHT = __riscv_vsmul(POLY_RIGHT, POLY_RIGHT, 1, vlen);
    POLY_RIGHT = __riscv_vsmul(POLY_RIGHT, POLY_RIGHT, 1, vlen);
    POLY_RIGHT = __riscv_vsmul(POLY_RIGHT, POLY_RIGHT, 1, vlen);
    // POLY_RIGHT is POLY^8

    // convert x to fixed-point Q62+m, 2^m <= x < 2^(m+1)
    VINT e = __riscv_vsra(F_AS_I(vx), MAN_LEN, vlen);
    e = __riscv_vmax(e, EXP_BIAS - 40, vlen);
    e = __riscv_vrsub(e, 2 * EXP_BIAS + 62, vlen);
    VFLOAT scale = I_AS_F(__riscv_vsll(e, MAN_LEN, vlen));
    // scale is 2^(62-m), X is x in Q_(62-m)
    VINT X = __riscv_vfcvt_x(__riscv_vfmul(vx, scale, vlen), vlen);
    POLY = __riscv_vsmul(X, POLY, 1, vlen);
    // X is Q_(62-m) POLY is now Q_(59-m)
    // x in [0, T], POLY is result in Q 59-m

    // x in (T, 6], result is 1 - 2^(-63) POLY_RIGHT
    // that is,  2^(-62)(2^62 - (POLY_RIGHT>>1))
    INT one = (1LL << 62);
    POLY_RIGHT = __riscv_vsra(POLY_RIGHT, 1, vlen);
    POLY_RIGHT = __riscv_vrsub(POLY_RIGHT, one, vlen);

    POLY = __riscv_vmerge(POLY, POLY_RIGHT, x_gt_T, vlen);
    // POLY contains the result in fixed point
    // scale is 59-m for x in [0, T] and 62 for x > T

    e = __riscv_vrsub(e, 2 * EXP_BIAS + 3, vlen);
    // exponent field of 2^(-59+m)
    e = __riscv_vmerge(e, EXP_BIAS - 62, x_gt_T, vlen);
    scale = I_AS_F(__riscv_vsll(e, MAN_LEN, vlen));

    vy = __riscv_vfcvt_f(POLY, vlen);
    vy = __riscv_vfmul(vy, scale, vlen);
    vy = __riscv_vfsgnj(vy, vx_orig, vlen);

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);
    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
