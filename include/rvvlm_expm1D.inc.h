// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

// Macros for common small-block codes
#define EXCEPTION_HANDLING_EXPM1(vx, special_args, vy_special, vlen)           \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    IDENTIFY(vclass, class_NaN | class_Inf, (special_args), (vlen));           \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      /* Substitute -Inf with -1 */                                            \
      VBOOL id_mask;                                                           \
      IDENTIFY(vclass, class_negInf, id_mask, (vlen));                         \
      vx = __riscv_vfmerge((vx), fp_negOne, id_mask, (vlen));                  \
      vy_special = __riscv_vfmul((special_args), (vx), fp_posOne, (vlen));     \
      vx = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));          \
    }                                                                          \
  } while (0)

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_EXPM1D_STD_EPSIM
#else
#define F_VER1 RVVLM_EXPM1DI_STD_EPSIM
#endif
#define P_INV_STD 0x1.71547652b82fep+0
#define P_HI_STD 0x1.62e42fefa39efp-1
#define P_LO_STD 0x1.abc9e3b39803fp-56
#define P_INV_TBL 0x1.71547652b82fep+6
#define P_HI_TBL 0x1.62e42fefa39efp-7
#define P_LO_TBL 0x1.abc9e3b39803fp-62
#define X_MAX 0x1.65p+9
#define X_MIN -0x1.5p+5

// We use the EPsim version of expD to compute expm1
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    EXCEPTION_HANDLING_EXPM1(vx, special_args, vy_special, vlen);

    // Clip
    vx = FCLIP(vx, X_MIN, X_MAX, vlen);

    // Argument reduction
    VFLOAT flt_n = __riscv_vfmul(vx, P_INV_STD, vlen);
    VINT n = __riscv_vfcvt_x(flt_n, vlen);
    flt_n = __riscv_vfcvt_f(n, vlen);

    VFLOAT r_tmp = __riscv_vfnmsac(vx, P_HI_STD, flt_n, vlen);
    VFLOAT r = __riscv_vfnmsub(flt_n, P_LO_STD, r_tmp, vlen);
    VFLOAT r_lo = __riscv_vfsub(r_tmp, r, vlen);
    r_lo = __riscv_vfnmsac(r_lo, P_LO_STD, flt_n, vlen);
    // r is the reduced argument in working precision; but r + r_lo is extra
    // precise exp(r+r_lo) is 1 + (r+r_lo) + (r+r_lo)^2/2 + r^3 * polynomial(r)
    // 1 + r + r^2/2 + (r_lo + r * r_lo) + r^3 * polynomial(r)
    r_lo = __riscv_vfmacc(r_lo, r, r_lo, vlen);
    // 1 + r + r^2/2 + r_lo + r^3 * polynomial(r)

    // Compute P_head + P_tail as r + r^2/2 accurately
    VFLOAT r_prime = __riscv_vfmul(r, 0x1.0p-1, vlen); // this coeff is 1/2
    VFLOAT P_head = __riscv_vfmadd(r, r_prime, r, vlen);
    VFLOAT P_tail = __riscv_vfsub(r, P_head, vlen);
    P_tail = __riscv_vfmacc(P_tail, r, r_prime, vlen);

    // Polynomial computation, we have a degree 11 polynomial
    VFLOAT poly_right = PSTEP(
        0x1.71ddf7aef0679p-19, r,
        PSTEP(0x1.27e4e210af311p-22, r,
              PSTEP(0x1.af5ff637cd647p-26, 0x1.1f6562eae5ba9p-29, r, vlen),
              vlen),
        vlen);

    VFLOAT poly_mid = PSTEP(
        0x1.6c16c16c166f3p-10, r,
        PSTEP(0x1.a01a01b0207e3p-13, 0x1.a01a01a4af90ap-16, r, vlen), vlen);

    VFLOAT poly_left =
        PSTEP(0x1.5555555555559p-3, r,
              PSTEP(0x1.5555555555556p-5, 0x1.111111110f62ap-7, r, vlen), vlen);

    VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
    VFLOAT r_cube = __riscv_vfmul(r_sq, r, vlen);

    VFLOAT poly = __riscv_vfmadd(poly_right, r_cube, poly_mid, vlen);
    poly = __riscv_vfmadd(poly, r_cube, poly_left, vlen);
    // At this point, exp(r) is 1 + P_head + P_tail + r_lo + r^3 * poly
    // expm1(x) = 2^n ( 1 - 2^(-n) + P_head + P_tail + r_lo + r^3 * poly )

    // Compute 1 - 2^(-n) accurately
    // Note that n >= -61 because the input argument was clipped because
    // expm1(x) = 1 as long as x <= -54 log(2).
    // For the purpose of 1 - 2^(-n), n can be clipped to n <= 64 as well
    // Then 1 - 2^(-n) = A + a, where A := 1 - 2^(-n), and
    // a = 1.0 for n <= -54; a = -2^(-n) if n >= 54; and a = 0 otherwise
    // While it is true we can use a KNUTH2SUM to compute A and a using 6
    // floating-point instructions, we can obtain A and a with just one
    // floating-point instructions and other simple integer instructions.
    // This should be more performant on most hardware implementations as
    // integer instructions have lower latency in general and possibly using
    // hardware resources different from that for floating point.
    VFLOAT One = VFMV_VF(fp_posOne, vlen);
    VINT n_clip = __riscv_vmin(n, 64, vlen);
    //  n_clip <= 64; note that n_clip >= -61
    VBOOL n_le53 = __riscv_vmsle(n_clip, 53, vlen);
    VBOOL n_le_neg54 = __riscv_vmsle(n_clip, -54, vlen);
    VINT I_tail = __riscv_vrsub(n_clip, -(EXP_BIAS + 2), vlen);
    // The 12 lsb of I_tail is (sign,expo) of -2^(-n_clip)
    VFLOAT u = I_AS_F(__riscv_vsll(I_tail, MAN_LEN, vlen)); // u = -2^(-n_clip)
    I_tail = __riscv_vmerge(I_tail, 0, n_le53, vlen);
    I_tail = __riscv_vmerge(I_tail, EXP_BIAS, n_le_neg54, vlen);
    VFLOAT a = I_AS_F(__riscv_vsll(I_tail, MAN_LEN, vlen));
    // a is 1.0, 0, -2^(-n_clip)
    // for n <= -54, -53 <= n <= 53, 54 <= n, respectively
    VFLOAT A = __riscv_vfadd(One, u, vlen);

    // Compute A + a + P_head + P_tail + r_lo + r^3 * poly
    P_tail = __riscv_vfadd(P_tail, a, vlen);
    P_tail = __riscv_vfadd(P_tail, r_lo, vlen);
    poly = __riscv_vfmadd(poly, r_cube, P_tail, vlen);
    P_head = __riscv_vfadd(P_head, poly, vlen);

    vy = __riscv_vfadd(P_head, A, vlen);
    // vy is now exp(r) - 1 accurately.

    // Need to compute 2^n * exp(r).
    FAST_LDEXP(vy, n, vlen);

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
