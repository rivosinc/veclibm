// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if defined(COMPILE_FOR_ERFC)
#define Y_AT_posINF fp_posZero
#define Y_AT_negINF 0x1.0p1
#elif defined(COMPILE_FOR_ERF)
#define Y_AT_posINF 0x1.0p0
#define Y_AT_negINF -0x1.0p0
#elif defined(COMPILE_FOR_CDFNORM)
#define Y_AT_posINF 0x1.0p0
#define Y_AT_negINF fp_posZero
#else
static_assert(false, "Must define COMPILE_FOR_{ERFC,ERF,CDFNORM}" __FILE__);
#endif

#define EXCEPTION_HANDLING(vx, special_args, vy_special, vlen)                 \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    IDENTIFY(vclass, class_NaN | class_Inf, (special_args), (vlen));           \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      VBOOL id_mask;                                                           \
      IDENTIFY(vclass, class_NaN, id_mask, (vlen));                            \
      (vy_special) = __riscv_vfadd(id_mask, (vx), (vx), (vlen));               \
      IDENTIFY(vclass, class_posInf, id_mask, (vlen));                         \
      (vy_special) =                                                           \
          __riscv_vfmerge((vy_special), Y_AT_posINF, id_mask, (vlen));         \
      IDENTIFY(vclass, class_negInf, id_mask, (vlen));                         \
      (vy_special) =                                                           \
          __riscv_vfmerge((vy_special), Y_AT_negINF, id_mask, (vlen));         \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)

#define LOG2_HI 0x1.62e42fefa39efp-1
#define LOG2_LO 0x1.abc9e3b39803fp-56
#define NEG_LOG2_INV -0x1.71547652b82fep+0

// compute exp(-A*B) as 2^n * Z, Z is a Q62 fixed-point value
// A, B are non-negative, and |A*B| <= 1200 log(2)
#define EXP_negAB(va, vb, n, Z, vlen)                                          \
  do {                                                                         \
    VFLOAT r = __riscv_vfmul((va), (vb), (vlen));                              \
    VFLOAT delta_r = __riscv_vfmsub((va), (vb), r, (vlen));                    \
    VFLOAT n_flt = __riscv_vfmul(r, NEG_LOG2_INV, (vlen));                     \
    (n) = __riscv_vfcvt_x(n_flt, (vlen));                                      \
    n_flt = __riscv_vfcvt_f((n), (vlen));                                      \
    r = __riscv_vfnmacc(r, LOG2_HI, n_flt, (vlen));                            \
    delta_r = __riscv_vfnmacc(delta_r, LOG2_LO, n_flt, (vlen));                \
    VINT R = __riscv_vfcvt_x(__riscv_vfmul(r, 0x1.0p63, (vlen)), (vlen));      \
    VINT DELTA_R =                                                             \
        __riscv_vfcvt_x(__riscv_vfmul(delta_r, 0x1.0p63, (vlen)), (vlen));     \
    R = __riscv_vadd(R, DELTA_R, (vlen));                                      \
    VINT P_RIGHT = PSTEP_I(                                                    \
        0x16c16c185646e2, R,                                                   \
        PSTEP_I(0x3403401a3f740, R,                                            \
                PSTEP_I(0x680665cc2958, R,                                     \
                        PSTEP_I(0xb8efdcde680, R,                              \
                                PSTEP_I(0x128afc94c08, 0x1acc4c50c4, R, vlen), \
                                vlen),                                         \
                        vlen),                                                 \
                vlen),                                                         \
        vlen);                                                                 \
                                                                               \
    VINT RSQ = __riscv_vsmul(R, R, 1, (vlen));                                 \
    VINT R6 = __riscv_vsmul(RSQ, RSQ, 1, (vlen));                              \
    R6 = __riscv_vsmul(R6, RSQ, 1, (vlen));                                    \
                                                                               \
    VINT P_LEFT =                                                              \
        PSTEP_I(0x4000000000000000, R,                                         \
                PSTEP_I(0x40000000000000ed, R,                                 \
                        PSTEP_I(0x2000000000001659, R,                         \
                                PSTEP_I(0xaaaaaaaaaaa201b, R,                  \
                                        PSTEP_I(0x2aaaaaaaaa03367,             \
                                                0x888888889fe9c4, R, vlen),    \
                                        vlen),                                 \
                                vlen),                                         \
                        vlen),                                                 \
                vlen);                                                         \
    P_RIGHT = __riscv_vsmul(P_RIGHT, R6, 1, (vlen));                           \
    Z = __riscv_vadd(P_LEFT, P_RIGHT, (vlen));                                 \
  } while (0)

// Transform x into (x-a)/(x+b) return as Q63 fixed point
// x is non-negative and x < 32; the result is strictly below 1
// in magnitude and thus we can use Q63 fixed point
// On input, we have x, -2^63 a, and b in floating point
// Both a and b are scalar between 3 and 5 and just a few bits
// thus we can use fast sum with a and b as the dominant term
// to get 2^63 x + neg_a_scaled, and x + b to extra precision
#define X_TRANSFORM(vx, neg_a_scaled, b, R, vlen)                              \
  do {                                                                         \
    VFLOAT numer, d_numer, denom, d_denom;                                     \
    denom = __riscv_vfadd((vx), (b), (vlen));                                  \
    d_denom = __riscv_vfrsub(denom, (b), (vlen));                              \
    d_denom = __riscv_vfadd(d_denom, (vx), (vlen));                            \
    VFLOAT one = VFMV_VF(fp_posOne, (vlen));                                   \
    VFLOAT recip, d_recip;                                                     \
    DIV_N1D2(one, denom, d_denom, recip, d_recip, (vlen));                     \
    numer = __riscv_vfmul((vx), 0x1.0p63, (vlen));                             \
    numer = __riscv_vfadd(numer, (neg_a_scaled), (vlen));                      \
    d_numer = __riscv_vfrsub(numer, (neg_a_scaled), (vlen));                   \
    d_numer = __riscv_vfmacc(d_numer, 0x1.0p63, (vx), (vlen));                 \
    /* (numer + d_numer) * (recip + d_recip) */                                \
    VFLOAT r, d_r;                                                             \
    r = __riscv_vfmul(numer, recip, (vlen));                                   \
    d_r = __riscv_vfmsub(numer, recip, r, (vlen));                             \
    d_r = __riscv_vfmacc(d_r, numer, d_recip, (vlen));                         \
    d_r = __riscv_vfmacc(d_r, d_numer, recip, (vlen));                         \
    (R) = __riscv_vfcvt_x(r, (vlen));                                          \
    (R) = __riscv_vadd((R), __riscv_vfcvt_x(d_r, (vlen)), (vlen));             \
  } while (0)

// Compute 1/(1+2x) as Q_m, m >= 62 fixed point. x >= 0
// If x < 1, m is 62, otherwise, m is 62+k+1, 2^k <= x < 2^(k+1)
#define RECIP_SCALE(vx, B, m, vlen)                                            \
  do {                                                                         \
    VFLOAT one = VFMV_VF(fp_posOne, (vlen));                                   \
    VFLOAT denom = __riscv_vfmadd((vx), 0x1.0p1, one, (vlen));                 \
    VFLOAT d_denom = __riscv_vfsub(one, denom, (vlen));                        \
    d_denom = __riscv_vfmacc(d_denom, 0x1.0p1, (vx), (vlen));                  \
    VFLOAT recip, d_recip;                                                     \
    DIV_N1D2(one, denom, d_denom, recip, d_recip, (vlen));                     \
    (m) = __riscv_vsra(F_AS_I((vx)), MAN_LEN, (vlen));                         \
    (m) = __riscv_vmax((m), EXP_BIAS - 1, (vlen));                             \
    (m) = __riscv_vadd((m), 63, (vlen));                                       \
    VFLOAT scale = I_AS_F(__riscv_vsll((m), MAN_LEN, (vlen)));                 \
    (m) = __riscv_vsub((m), EXP_BIAS, (vlen));                                 \
    (B) = __riscv_vfcvt_x(__riscv_vfmul(recip, scale, (vlen)), (vlen));        \
    d_recip = __riscv_vfmul(d_recip, scale, (vlen));                           \
    (B) = __riscv_vadd((B), __riscv_vfcvt_x(d_recip, (vlen)), (vlen));         \
  } while (0)
