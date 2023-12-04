// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

// Macros for common small-block codes
#define EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen)             \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    IDENTIFY(vclass, class_NaN | class_Inf, (special_args), (vlen))            \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      /* Substitute -Inf with +0 */                                            \
      VBOOL id_mask;                                                           \
      IDENTIFY(vclass, class_negInf, id_mask, (vlen))                          \
      vx = __riscv_vfmerge(vx, fp_posZero, id_mask, (vlen));                   \
      vy_special = __riscv_vfadd((special_args), (vx), (vx), (vlen));          \
      vx = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));          \
    }                                                                          \
  } while (0)

#if defined(COMPILE_FOR_EXP)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_EXPD_STD
#define F_VER2 RVVLM_EXPD_STD_R_EXTRA
#define F_VER3 RVVLM_EXPD_STD_EPSIM
#define F_VER4 RVVLM_EXPD_TBL64
#else
#define F_VER1 RVVLM_EXPDI_STD
#define F_VER2 RVVLM_EXPDI_STD_R_EXTRA
#define F_VER3 RVVLM_EXPDI_STD_EPSIM
#define F_VER4 RVVLM_EXPDI_TBL64
#endif
#define P_INV_STD 0x1.71547652b82fep+0
#define P_HI_STD 0x1.62e42fefa39efp-1
#define P_LO_STD 0x1.abc9e3b39803fp-56
#define P_INV_TBL 0x1.71547652b82fep+6
#define P_HI_TBL 0x1.62e42fefa39efp-7
#define P_LO_TBL 0x1.abc9e3b39803fp-62
#define X_MAX 0x1.65p+9
#define X_MIN -0x1.77p+9
#elif defined(COMPILE_FOR_EXP2)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_EXP2D_STD
#define F_VER2 RVVLM_EXP2D_STD_R_EXTRA
#define F_VER3 RVVLM_EXP2D_STD_EPSIM
#define F_VER4 RVVLM_EXP2D_TBL64
#else
#define F_VER1 RVVLM_EXP2DI_STD
#define F_VER2 RVVLM_EXP2DI_STD_R_EXTRA
#define F_VER3 RVVLM_EXP2DI_STD_EPSIM
#define F_VER4 RVVLM_EXP2DI_TBL64
#endif
#define P_INV_STD 0x1.71547652b82fep+0
#define P_HI_STD 0x1.62e42fefa39efp-1
#define P_LO_STD 0x1.abc9e3b39803fp-56
#define P_INV_TBL 0x1.0p6
#define P_HI_TBL 0x1.0p-6
#define P_LO_TBL 0x1.abc9e3b39803fp-56
#define LOGB_HI 0x1.62e42fefa39efp-1
#define LOGB_LO 0x1.abc9e3b39803fp-56
#define X_MAX 0x1.018p10
#define X_MIN -0x1.0ep10
#elif defined(COMPILE_FOR_EXP10)
#if (STRIDE == 1)
#define F_VER1 RVVLM_EXP10D_STD
#define F_VER2 RVVLM_EXP10D_STD_R_EXTRA
#define F_VER3 RVVLM_EXP10D_STD_EPSIM
#define F_VER4 RVVLM_EXP10D_TBL64
#else
#define F_VER1 RVVLM_EXP10DI_STD
#define F_VER2 RVVLM_EXP10DI_STD_R_EXTRA
#define F_VER3 RVVLM_EXP10DI_STD_EPSIM
#define F_VER4 RVVLM_EXP10DI_TBL64
#endif
#define P_INV_STD 0x1.a934f0979a371p+1
#define P_HI_STD 0x1.34413509f79ffp-2
#define P_LO_STD -0x1.9dc1da994fd21p-59
#define P_INV_TBL 0x1.a934f0979a371p+7
#define P_HI_TBL 0x1.34413509f79ffp-8
#define P_LO_TBL -0x1.9dc1da994fd21p-65
#define LOGB_HI 0x1.26bb1bbb55516p+1
#define LOGB_LO -0x1.f48ad494ea3e9p-53
#define X_MAX 0x1.36p8
#define X_MIN -0x1.46p8
#else
static_assert(false, "Must specify base of exponential" __FILE__);
#endif

#include <fenv.h>

// Version 1 is reduction to standard primary interval.
// Reduced argument is represented as one FP64 variable.
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    // Set results for input of NaN and Inf; substitute them with zero
    EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen);

    // Clip
    vx = FCLIP(vx, X_MIN, X_MAX, vlen);

    // Argument reduction
#if !defined(COMPILE_FOR_EXP2)
    VFLOAT flt_n = __riscv_vfmul(vx, P_INV_STD, vlen);
    VINT n = __riscv_vfcvt_x(flt_n, vlen);
    flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT r = __riscv_vfnmsac(vx, P_HI_STD, flt_n, vlen);
#else
    VINT n = __riscv_vfcvt_x(vx, vlen);
    VFLOAT flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT r = __riscv_vfsub(vx, flt_n, vlen);
    r = __riscv_vfmul(r, LOGB_HI, vlen);
#endif

#if defined(COMPILE_FOR_EXP)
    r = __riscv_vfnmsac(r, P_LO_STD, flt_n, vlen);
#elif defined(COMPILE_FOR_EXP10)
    VFLOAT r_lo = __riscv_vfmul(flt_n, P_LO_STD, vlen);
    r_lo =
        __riscv_vfnmsac(__riscv_vfmul(r, LOGB_LO, vlen), LOGB_HI, r_lo, vlen);
    r = __riscv_vfmadd(r, LOGB_HI, r_lo, vlen);
#endif

    // Polynomial computation, we have a degree 11
    // We compute the part from r^3 in three segments, increasing parallelism
    // Ideally the compiler will interleave the computations of the segments
    VFLOAT poly_right = PSTEP(
        0x1.71df804f1baa1p-19, r,
        PSTEP(0x1.28aa3ea739296p-22, 0x1.acf86201fd199p-26, r, vlen), vlen);

    VFLOAT poly_mid = PSTEP(
        0x1.6c16c1825c970p-10, r,
        PSTEP(0x1.a01a00fe6f730p-13, 0x1.a0199e1789c72p-16, r, vlen), vlen);

    VFLOAT poly_left =
        PSTEP(0x1.55555555554d2p-3, r,
              PSTEP(0x1.5555555551307p-5, 0x1.11111111309a4p-7, r, vlen), vlen);

    VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
    VFLOAT r_cube = __riscv_vfmul(r_sq, r, vlen);

    VFLOAT poly = __riscv_vfmadd(poly_right, r_cube, poly_mid, vlen);
    poly = __riscv_vfmadd(poly, r_cube, poly_left, vlen);

    poly = PSTEP(0x1.0000000000007p-1, r, poly, vlen);

    r = __riscv_vfmacc(r, r_sq, poly, vlen);
    vy = __riscv_vfadd(r, 0x1.0p0, vlen);

    // at this point, vy is the entire degree-11 polynomial
    // vy ~=~ exp(r)

    // Need to compute 2^n * exp(r).
    FAST_LDEXP(vy, n, vlen);

    // Incorporate results of exceptional inputs
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}

// Version 2 is reduction to standard primary interval.
// Reduced argument is represented as two FP64 variables r, delta_r.
void F_VER2(API) {
  size_t vlen;
  VFLOAT vx, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    // Set results for input of NaN and Inf; substitute them with zero
    EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen);

    // Clip
    vx = FCLIP(vx, X_MIN, X_MAX, vlen);

    // Argument reduction
#if defined(COMPILE_FOR_EXP)
    VFLOAT flt_n = __riscv_vfmul(vx, P_INV_STD, vlen);
    VINT n = __riscv_vfcvt_x(flt_n, vlen);
    flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT r_tmp = __riscv_vfnmsac(vx, P_HI_STD, flt_n, vlen);
    VFLOAT r = __riscv_vfnmsac(r_tmp, P_LO_STD, flt_n, vlen);
    VFLOAT delta_r = __riscv_vfsub(r_tmp, r, vlen);
    delta_r = __riscv_vfnmsac(delta_r, P_LO_STD, flt_n, vlen);
#elif defined(COMPILE_FOR_EXP2)
    VINT n = __riscv_vfcvt_x(vx, vlen);
    VFLOAT flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT r_tmp = __riscv_vfsub(vx, flt_n, vlen);
    VFLOAT r = __riscv_vfmul(r_tmp, LOGB_HI, vlen);
    VFLOAT delta_r = __riscv_vfmsub(r_tmp, LOGB_HI, r, vlen);
    delta_r = __riscv_vfmacc(delta_r, LOGB_LO, r_tmp, vlen);
#else
    VFLOAT flt_n = __riscv_vfmul(vx, P_INV_STD, vlen);
    VINT n = __riscv_vfcvt_x(flt_n, vlen);
    flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT s_tmp = __riscv_vfnmsac(vx, P_HI_STD, flt_n, vlen);
    VFLOAT s = __riscv_vfnmsac(s_tmp, P_LO_STD, flt_n, vlen);
    VFLOAT delta_s = __riscv_vfsub(s_tmp, s, vlen);
    delta_s = __riscv_vfnmsac(delta_s, P_LO_STD, flt_n, vlen);

    VFLOAT r = __riscv_vfmul(s, LOGB_HI, vlen);
    s_tmp = __riscv_vfmsub(s, LOGB_HI, r, vlen);
    VFLOAT delta_r = __riscv_vfmul(s, LOGB_LO, vlen);
    delta_r = __riscv_vfmacc(delta_r, LOGB_HI, delta_s, vlen);
    delta_r = __riscv_vfadd(delta_r, s_tmp, vlen);
#endif

    // Polynomial computation, we have a degree 11
    // We compute the part from r^3 in three segments, increasing parallelism
    // Ideally the compiler will interleave the computations of the segments
    VFLOAT poly_right = PSTEP(
        0x1.71df804f1baa1p-19, r,
        PSTEP(0x1.28aa3ea739296p-22, 0x1.acf86201fd199p-26, r, vlen), vlen);

    VFLOAT poly_mid = PSTEP(
        0x1.6c16c1825c970p-10, r,
        PSTEP(0x1.a01a00fe6f730p-13, 0x1.a0199e1789c72p-16, r, vlen), vlen);

    VFLOAT poly_left =
        PSTEP(0x1.55555555554d2p-3, r,
              PSTEP(0x1.5555555551307p-5, 0x1.11111111309a4p-7, r, vlen), vlen);

    VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
    VFLOAT r_cube = __riscv_vfmul(r_sq, r, vlen);

    VFLOAT poly = __riscv_vfmadd(poly_right, r_cube, poly_mid, vlen);
    poly = __riscv_vfmadd(poly, r_cube, poly_left, vlen);

    poly = PSTEP(0x1.0000000000007p-1, r, poly, vlen);

    poly = __riscv_vfmadd(poly, r_sq, delta_r, vlen);
    vy = __riscv_vfadd(poly, r, vlen);
    vy = __riscv_vfadd(vy, 0x1.0p0, vlen);
    // at this point, vy is the entire degree-11 polynomial
    // vy ~=~ exp(r)

    // Need to compute 2^n * exp(r).
    FAST_LDEXP(vy, n, vlen);

    // Incorporate results of exceptional inputs
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}

// Version 3 computes 1 + r + r^2/2 to extra precision using FP techniques
// The r in this expression is the extra-precise reduced argument
void F_VER3(API) {
  size_t vlen;
  VFLOAT vx, vy, vy_special;
  VBOOL special_args;
  double Peg = 0x1.8p+27;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    // Set results for input of NaN and Inf; substitute them with zero
    EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen);

    // Clip
    vx = FCLIP(vx, X_MIN, X_MAX, vlen);

    // Argument reduction
#if defined(COMPILE_FOR_EXP)
    VFLOAT flt_n = __riscv_vfmul(vx, P_INV_STD, vlen);
    VINT n = __riscv_vfcvt_x(flt_n, vlen);
    flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT r = __riscv_vfnmsac(vx, P_HI_STD, flt_n, vlen);
    VFLOAT r_hi = __riscv_vfadd(r, Peg, vlen);
    r_hi = __riscv_vfsub(r_hi, Peg, vlen);
    VFLOAT r_lo = __riscv_vfsub(r, r_hi, vlen);
    r_lo = __riscv_vfnmsac(r_lo, P_LO_STD, flt_n, vlen);
    r = __riscv_vfadd(r_hi, r_lo, vlen);
#elif defined(COMPILE_FOR_EXP2)
    VINT n = __riscv_vfcvt_x(vx, vlen);
    VFLOAT flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT r_tmp = __riscv_vfsub(vx, flt_n, vlen);
    VFLOAT r_hi = VFMV_VF(Peg, vlen);
    r_hi = __riscv_vfmacc(r_hi, LOGB_HI, r_tmp, vlen);
    r_hi = __riscv_vfsub(r_hi, Peg, vlen);
    VFLOAT r_lo = __riscv_vfmsub(r_tmp, LOGB_HI, r_hi, vlen);
    r_lo = __riscv_vfmacc(r_lo, LOGB_LO, r_tmp, vlen);
    VFLOAT r = __riscv_vfadd(r_hi, r_lo, vlen);
#else
    VFLOAT flt_n = __riscv_vfmul(vx, P_INV_STD, vlen);
    VINT n = __riscv_vfcvt_x(flt_n, vlen);
    flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT s_tmp = __riscv_vfnmsac(vx, P_HI_STD, flt_n, vlen);
    VFLOAT s = __riscv_vfnmsac(s_tmp, P_LO_STD, flt_n, vlen);
    VFLOAT delta_s = __riscv_vfsub(s_tmp, s, vlen);
    delta_s = __riscv_vfnmsac(delta_s, P_LO_STD, flt_n, vlen);

    VFLOAT r_hi = VFMV_VF(Peg, vlen);
    r_hi = __riscv_vfmacc(r_hi, LOGB_HI, s, vlen);
    r_hi = __riscv_vfsub(r_hi, Peg, vlen);
    VFLOAT r_lo = __riscv_vfmsub(s, LOGB_HI, r_hi, vlen);
    r_lo = __riscv_vfmacc(r_lo, LOGB_LO, s, vlen);
    VFLOAT r = __riscv_vfadd(r_hi, r_lo, vlen);
#endif

    // r_hi + r_lo is extra-precise reduced argument
    // and also 1 + r_hi + r_hi^2/2 is computable error-free
    // 1 + (r_hi + r_lo) + (r_hi + r_lo)^2/2  is
    // P_head + P_tail where
    // P_head = 1 + r_hi + r_hi^2/2 = 1 + r_hi*(1 + r_hi/2)
    // P_tail = r_lo + r_lo(r_hi + r_lo/2)
    VFLOAT P_head =
        PSTEP(0x1.0p0, r_hi, PSTEP(0x1.0p0, 0x1.0p-1, r_hi, vlen), vlen);

    VFLOAT P_tail = __riscv_vfmacc(r_hi, 0x1.0p-1, r_lo, vlen);
    P_tail = __riscv_vfmadd(P_tail, r_lo, r_lo, vlen);

    // Polynomial computation, we have a degree 11
    // We compute the part from r^3 in three segments, increasing parallelism
    // Ideally the compiler will interleave the computations of the segments
    VFLOAT poly_right = PSTEP(
        0x1.71dfc3d471117p-19, r,
        PSTEP(0x1.289feff6e458ap-22, 0x1.ac82ecdeaa346p-26, r, vlen), vlen);

    VFLOAT poly_mid = PSTEP(
        0x1.6c16c17ce42b2p-10, r,
        PSTEP(0x1.a01a00e44cd99p-13, 0x1.a019aac62dedbp-16, r, vlen), vlen);

    VFLOAT poly_left =
        PSTEP(0x1.55555555554cap-3, r,
              PSTEP(0x1.555555555319fp-5, 0x1.1111111134581p-7, r, vlen), vlen);

    VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
    VFLOAT r_cube = __riscv_vfmul(r_sq, r, vlen);

    VFLOAT poly = __riscv_vfmadd(poly_right, r_cube, poly_mid, vlen);
    poly = __riscv_vfmadd(poly, r_cube, poly_left, vlen);
    poly = __riscv_vfmadd(poly, r_cube, P_tail, vlen);
    vy = __riscv_vfadd(poly, P_head, vlen);
    // at this point, vy is the entire degree-11 polynomial
    // vy ~=~ exp(r)

    // Need to compute 2^n * exp(r).
    FAST_LDEXP(vy, n, vlen);

    // Incorporate results of exceptional inputs
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}

// Version 4 is a table driven algorithm. The table contains exp(j/64)
// for j = 0, 1, ..., 63. One key feature is that this table of values
// are stored as fixed point numbers in "Q62" format, i.e. they correspond
// to 2^62 * exp(j/64).
void F_VER4(API) {
  size_t vlen;
  VFLOAT vx, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);

    // Set results for input of NaN and Inf; substitute them with zero
    EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen);

    // Clip
    vx = FCLIP(vx, X_MIN, X_MAX, vlen);

    // Argument reduction
#if !defined(COMPILE_FOR_EXP2)
    VFLOAT flt_n = __riscv_vfmul(vx, P_INV_TBL, vlen);
    VINT n = __riscv_vfcvt_x(flt_n, vlen);
    flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT r = __riscv_vfnmsac(vx, P_HI_TBL, flt_n, vlen);
#else
    VINT n = __riscv_vfcvt_x(__riscv_vfmul(vx, 0x1.0p6, vlen), vlen);
    VFLOAT flt_n = __riscv_vfcvt_f(n, vlen);
    VFLOAT r = __riscv_vfnmsub(flt_n, 0x1.0p-6, vx, vlen);
    r = __riscv_vfmul(r, LOGB_HI, vlen);
#endif

#if defined(COMPILE_FOR_EXP)
    r = __riscv_vfnmsac(r, P_LO_TBL, flt_n, vlen);
#elif defined(COMPILE_FOR_EXP10)
    VFLOAT r_lo = __riscv_vfmul(flt_n, P_LO_TBL, vlen);
    r_lo =
        __riscv_vfnmsac(__riscv_vfmul(r, LOGB_LO, vlen), LOGB_HI, r_lo, vlen);
    r = __riscv_vfmadd(r, LOGB_HI, r_lo, vlen);
#endif

    // Polynomial computation, we have a degree 5
    // We break this up into 2 pieces
    // Ideally the compiler will interleave the computations of the segments
    VFLOAT poly_right =
        PSTEP(0x1.5555722e87735p-5, 0x1.1107f5fc29bb7p-7, r, vlen);
    VFLOAT poly_left =
        PSTEP(0x1.fffffffffe1f5p-2, 0x1.55555556582a8p-3, r, vlen);
    VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
    VFLOAT poly = __riscv_vfmadd(poly_right, r_sq, poly_left, vlen);
    poly = __riscv_vfmadd(poly, r_sq, r, vlen);
    poly = __riscv_vfmul(poly, 0x1.0p63, vlen);

    VINT P = __riscv_vfcvt_x(poly, vlen);
    VINT j = __riscv_vand(n, 0x3f, vlen);
    j = __riscv_vsll(j, 3, vlen);
    VINT T = __riscv_vluxei64(expD_tbl64_fixedpt, I_AS_U(j), vlen);

    P = __riscv_vsmul(P, T, 1, vlen);
    P = __riscv_vsadd(P, T, vlen);
    vy = __riscv_vfcvt_f(P, vlen);
    // at this point, vy ~=~ 2^62 * exp(r)

    // Need to compute 2^(n-62) * exp(r).
    n = __riscv_vsra(n, 6, vlen);

    n = __riscv_vsub(n, 62, vlen);
    FAST_LDEXP(vy, n, vlen);

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
