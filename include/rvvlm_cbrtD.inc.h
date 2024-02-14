// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_CBRTD_ITER
#else
#define F_VER1 RVVLM_CBRTDI_ITER
#endif

#define EXCEPTION_HANDLING_CBRT(vx, special_args, vy_special, n_adjust, vlen)  \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    /* special handling NaN, +-Inf, +-0 */                                     \
    IDENTIFY(vclass, 0x399, (special_args), (vlen));                           \
    VBOOL denorm;                                                              \
    IDENTIFY(vclass, 0x24, denorm, (vlen));                                    \
    VBOOL special_n_denorm = __riscv_vmor((special_args), denorm, (vlen));     \
    (n_adjust) = __riscv_vxor((n_adjust), (n_adjust), (vlen));                 \
    if (__riscv_vcpop(special_n_denorm, (vlen)) > 0) {                         \
      /* normalize denormal numbers */                                         \
      VFLOAT vx_normal = __riscv_vfmul(denorm, vx, 0x1.0p60, (vlen));          \
      (vx) = __riscv_vmerge((vx), vx_normal, denorm, (vlen));                  \
      (n_adjust) = __riscv_vmerge((n_adjust), -20, denorm, (vlen));            \
      (vy_special) = __riscv_vfadd((special_args), (vx), (vx), (vlen));        \
      (vx) = __riscv_vfmerge((vx), fp_posOne, (special_args), (vlen));         \
    }                                                                          \
  } while (0)

#define ONE_OV_3 0x1.5555555555555p-2
#define THIRD_Q62 0x1555555555555555
#define ONE_Q62 0x4000000000000000
#define CBRT_2_Q62 0x50a28be635ca2b89
#define CBRT_4_Q62 0x6597fa94f5b8f20b

// This version uses a short polynomial to approximate x^(-1/3) to 14+ bits
// It then iterates to improve the accuracy. Finally x * (x^(-1/3))^2 gives
// x^(1/3)
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx_orig, vx, vy, vy_special;
  VBOOL special_args;
  VINT n_adjust;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx_orig = VFLOAD_INARG1(vlen);

    // Set results for input of NaN, +-Inf, +-0, and normalize denormals
    EXCEPTION_HANDLING_CBRT(vx_orig, special_args, vy_special, n_adjust, vlen);
    vx = __riscv_vfsgnj(vx_orig, fp_posOne, vlen);

    VINT N = __riscv_vsra(F_AS_I(vx), MAN_LEN, vlen);
    N = __riscv_vsub(N, EXP_BIAS, vlen);
    vx = I_AS_F(__riscv_vsub(F_AS_I(vx), __riscv_vsll(N, MAN_LEN, vlen), vlen));
    // vx are now in [1, 2), the original argument is 2^N * vx
    // cube root is 2^M * 2^(J/3) * vx^(1/3) where N = 3 * M + J, 0 <= J <= 2

    VFLOAT poly_left = PSTEP(
        0x1.c7feaf5d6cc3bp+0, vx,
        PSTEP(-0x1.910e22c54a1eap+0, 0x1.3e9d3512b6a5ap+0, vx, vlen), vlen);

    VFLOAT xcube = __riscv_vfmul(vx, vx, vlen);
    xcube = __riscv_vfmul(xcube, vx, vlen);

    VFLOAT poly_right = PSTEP(
        -0x1.3261c716ecf2dp-1, vx,
        PSTEP(0x1.3ffc61ff0985dp-3, -0x1.173278cb4b00fp-6, vx, vlen), vlen);

    VFLOAT z = __riscv_vfmadd(poly_right, xcube, poly_left, vlen);
    // z ~=~ x^(-1/3) to relatitve error 2^(-17.3)
    // iteration is z <-- z + delta * z where delta = 1/3 - 1/3 * x * z^3

    // work on decomposing N = 3 * M + J
    // M = N // 3; it is a well known trick that one can get
    // integer quotient by multiplication of an "inverse"
    // but this works only for non-negative (or non-positive) dividends
    // So we add 1023 to N to make it non-negative
    VINT L = __riscv_vadd(N, 1023, vlen);
    VINT M = __riscv_vsra(__riscv_vmul(L, 1366, vlen), 12, vlen);
    ;
    // 0 <= L <= 2046; 1366 is ceil(2^12/3); M = L // 3
    VINT J = __riscv_vadd(__riscv_vsll(M, 1, vlen), M, vlen);
    J = __riscv_vsub(L, J, vlen);   // J is N mod 3
    M = __riscv_vsub(M, 341, vlen); // 341 is 1023/3
    // At this point, N = 3 * M + J

    VINT R = VMVI_VX(ONE_Q62, vlen);
    VBOOL J_is_1 = __riscv_vmseq(J, 1, vlen);
    VBOOL J_is_2 = __riscv_vmseq(J, 2, vlen);
    R = __riscv_vmerge(R, CBRT_2_Q62, J_is_1, vlen);
    R = __riscv_vmerge(R, CBRT_4_Q62, J_is_2, vlen);

    // two iterations of z <-- z + delta * z
    // rounding error in the first iteration is immaterial
    VFLOAT a = __riscv_vfmul(z, z, vlen);
    VFLOAT b = __riscv_vfmul(vx, z, vlen);
    b = __riscv_vfmul(b, a, vlen);
    VFLOAT c = VFMV_VF(ONE_OV_3, vlen);
    VFLOAT delta = __riscv_vfnmsub(b, ONE_OV_3, c, vlen);
    z = __riscv_vfmacc(z, delta, z, vlen);

    // the second iteration we perform in fixed point
    // as the rounding errors need to be controlled
    double two_to_62 = 0x1.0p62;
    VINT Z_Q62 = __riscv_vfcvt_x(__riscv_vfmul(z, two_to_62, vlen), vlen);
    VINT X_Q62 = __riscv_vfcvt_x(__riscv_vfmul(vx, two_to_62, vlen), vlen);
    VINT A = __riscv_vsll(__riscv_vsmul(Z_Q62, Z_Q62, 1, vlen), 1, vlen);
    VINT B = __riscv_vsll(__riscv_vsmul(X_Q62, Z_Q62, 1, vlen), 1, vlen);
    B = __riscv_vsll(__riscv_vsmul(A, B, 1, vlen), 1, vlen);
    B = __riscv_vsll(__riscv_vsmul(B, THIRD_Q62, 1, vlen), 1, vlen);
    VINT DELTA = __riscv_vrsub(B, THIRD_Q62, vlen);
    A = __riscv_vsll(__riscv_vsmul(DELTA, Z_Q62, 1, vlen), 1, vlen);
    Z_Q62 = __riscv_vadd(Z_Q62, A, vlen);

    // X * Z * Z is cube root of x in [1, 2)
    // then we need to multiply with 2^(1/3) or 4^(1/3) as needed
    // together with multiplication with 2^m
    Z_Q62 = __riscv_vsll(__riscv_vsmul(Z_Q62, Z_Q62, 1, vlen), 1, vlen);
    Z_Q62 = __riscv_vsll(__riscv_vsmul(Z_Q62, X_Q62, 1, vlen), 1, vlen);
    R = __riscv_vsmul(R, Z_Q62, 1, vlen); // scale is 61 now

    M = __riscv_vadd(M, EXP_BIAS - 61, vlen);
    M = __riscv_vadd(M, n_adjust, vlen);
    VFLOAT scale = I_AS_F(__riscv_vsll(M, MAN_LEN, vlen));
    vy = __riscv_vfcvt_f(R, vlen);
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
