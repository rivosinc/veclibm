// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

// Macros for common small-block codes
#define EXCEPTION_HANDLING_LOG1P(vx, vxp1, special_args, vy_special, vlen)     \
  do {                                                                         \
    vxp1 = __riscv_vfadd(vx, fp_posOne, vlen);                                 \
    VUINT vclass = __riscv_vfclass((vxp1), (vlen));                            \
    /* special handling except positive normal number */                       \
    IDENTIFY(vclass, 0x3BF, (special_args), (vlen));                           \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      VBOOL id_mask;                                                           \
      /* substitute negative arguments with sNaN */                            \
      IDENTIFY(vclass, class_negative, id_mask, (vlen));                       \
      vxp1 = __riscv_vfmerge(vxp1, fp_sNaN, id_mask, vlen);                    \
      /* substitute 0 with -0 */                                               \
      IDENTIFY(vclass, class_Zero, id_mask, vlen);                             \
      vxp1 = __riscv_vfmerge(vxp1, fp_negZero, id_mask, vlen);                 \
      /* eliminate positive denorm input from special_args */                  \
      IDENTIFY(vclass, 0x39F, (special_args), (vlen));                         \
      /* for narrowed set of special arguments, compute vx+vfrec7(vx) */       \
      vy_special = __riscv_vfrec7((special_args), (vxp1), (vlen));             \
      vy_special = __riscv_vfadd((special_args), vy_special, (vxp1), (vlen));  \
      vx = __riscv_vfmerge((vx), fp_posOne, (special_args), (vlen));           \
      vxp1 = __riscv_vfmerge((vxp1), 0x1.0p1, (special_args), (vlen));         \
    }                                                                          \
  } while (0)

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_LOG1PD_TBL128
#else
#define F_VER1 RVVLM_LOG1PDI_TBL128
#endif
#define LOG2_HI 0x1.62e42fefa39efp-1
#define LOG2_LO 0x1.abc9e3b39803fp-56

#include <fenv.h>

// Version 1 uses a 128-entry LUT
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vxp1, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    // Handle cases when 1+x is NaN, Inf, 0 and -ve
    EXCEPTION_HANDLING_LOG1P(vx, vxp1, special_args, vy_special, vlen);

    // 1+in_arg at this point are positive, finite and not subnormal
    // Decompose in_arg into n, B, r: 1+in_arg = 2^n (1/B) (1 + r)
    // B is equivalently defined by ind, 0 <= ind < 128

    // First compute 1+in_arg as vxp1 + u
    VFLOAT first = __riscv_vfmax(vx, fp_posOne, vlen);
    VFLOAT second = __riscv_vfmin(vx, fp_posOne, vlen);
    VFLOAT u = __riscv_vfsub(first, vxp1, vlen);
    u = __riscv_vfadd(u, second, vlen);

    // Decompose 1+in_arg as 2^n (1/B) (1 + r). Unlike log, r has more precision
    // then FP64 and needs to be preserved to some extent To prevent loss of
    // precision in frec7 when 1+in_arg is too large, we modify vxp1 However,
    // unlike in the case of log where we simply change the exponent to 0, this
    // technique does not work too well here as we would than need to adjust the
    // exponent of u accordingly as well. Here we modify vxp1 only when it is
    // large, which in this case u will so small in comparison that missing this
    // theoretically needed adjustment does not matter.
    VINT n_adjust = VMVI_VX(0, vlen);
    VFLOAT scale = VFMV_VF(fp_posOne, vlen);
    VBOOL adjust = __riscv_vmfge(vxp1, 0x1.0p1020, vlen);
    n_adjust = __riscv_vmerge(n_adjust, 4, adjust, vlen);
    scale = __riscv_vfmerge(scale, 0x1.0p-4, adjust, vlen);
    vxp1 = __riscv_vfmul(vxp1, scale, vlen);

    VINT n = U_AS_I(
        __riscv_vadd(__riscv_vsrl(F_AS_U(vxp1), MAN_LEN - 1, vlen), 1, vlen));
    n = __riscv_vsra(n, 1, vlen);
    n = __riscv_vsub(n, EXP_BIAS, vlen);
    n = __riscv_vadd(n, n_adjust, vlen);
    VFLOAT n_flt = __riscv_vfcvt_f(n, vlen);

    VUINT ind = __riscv_vand(__riscv_vsrl(F_AS_U(vxp1), MAN_LEN - 10, vlen),
                             0x3F8, vlen);

    // normalize exponent of vx
    VFLOAT B = __riscv_vfrec7(vxp1, vlen);
    // adjust B if ind == 0. We cannot simply set B to be 1.0, but round up at
    // the 7-th bit
    VBOOL adjust_B = __riscv_vmseq(ind, 0, vlen);
    B = I_AS_F(__riscv_vadd_mu(adjust_B, F_AS_I(B), F_AS_I(B),
                               0x0000200000000000, vlen));

    VFLOAT r_hi = VFMV_VF(fp_posOne, vlen);
    r_hi = __riscv_vfmsac(r_hi, vxp1, B, vlen);
    VFLOAT r = __riscv_vfmacc(r_hi, u, B, vlen);
    VFLOAT delta_r = __riscv_vfsub(r_hi, r, vlen);
    delta_r = __riscv_vfmacc(delta_r, u, B, vlen);

    // 1+in_arg = 2^n (1/B) (1 + r + delta_r)
    // Thus log(1+in_arg) = (n + T) * log(2) + log(1 + r + delta_r)
    // log is natural log (aka ln)
    // log(1 + r + delta_r) is approximated by r + delta_r + r^2(p0 + p1 r + ...
    // )
    VFLOAT rsq = __riscv_vfmul(r, r, vlen);
    VFLOAT rcube = __riscv_vfmul(rsq, r, vlen);

    VFLOAT poly_right = PSTEP(
        0x1.9999998877038p-3, r,
        PSTEP(-0x1.555c54f8b7c6cp-3, 0x1.2499765b3c27ap-3, r, vlen), vlen);

    VFLOAT poly_left = PSTEP(
        -0x1.000000000001cp-1, r,
        PSTEP(0x1.55555555555a9p-2, -0x1.fffffff2018cfp-3, r, vlen), vlen);

    VFLOAT poly = __riscv_vfmadd(poly_right, rcube, poly_left, vlen);
    poly = __riscv_vfmadd(rsq, poly, delta_r, vlen);
    // At this point, log(1+r+delta_r) is approximated by r + poly

    // Load table values and get n_flt + T to be A + a
    VINT T = __riscv_vluxei64(logD_tbl128_fixedpt, ind, vlen);
    VINT T_hi = __riscv_vsll(__riscv_vsra(T, 24, vlen), 24, vlen);
    VINT T_lo = __riscv_vsub(T, T_hi, vlen);
    VFLOAT T_hi_flt = __riscv_vfcvt_f(T_hi, vlen);
    VFLOAT A = __riscv_vfmadd(T_hi_flt, 0x1.0p-63, n_flt, vlen);
    VFLOAT a = __riscv_vfcvt_f(T_lo, vlen);
    a = __riscv_vfmul(a, 0x1.0p-63, vlen);

    // Compute (A + a) * (log2_hi + log2_lo) + (r + poly)
    // where B can be e, 2, or 10
    VFLOAT delta_1 = __riscv_vfmul(A, LOG2_LO, vlen);
    delta_1 = __riscv_vfmadd(a, LOG2_HI, delta_1, vlen);
    poly = __riscv_vfadd(poly, delta_1, vlen);
    poly = __riscv_vfadd(poly, r, vlen);
    vy = __riscv_vfmadd(A, LOG2_HI, poly, vlen);

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
