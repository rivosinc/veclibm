// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#define PIBY2_INV 0x1.45f306dc9c883p-1
#define PIBY2_HI 0x1.921fb54442d18p+0
#define PIBY2_MID 0x1.1a62633145c07p-54
#define PIBY2_LO -0x1.f1976b7ed8fbcp-110
#define PI_HI 0x1.921fb54442d18p+1
#define PI_MID 0x1.1a62633145c07p-53

#if defined(COMPILE_FOR_SIN) || defined(COMPILE_FOR_TAN)
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfmadd((small_x), (vx), 0x1.0p-60, (vx), (vlen))
#elif defined(COMPILE_FOR_SINPI) || defined(COMPILE_FOR_TANPI)
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfmadd((small_x), (vx), PI_HI,                                       \
                 __riscv_vfmul((small_x), (vx), PI_MID, (vlen)), (vlen))
#elif defined(COMPILE_FOR_SINCOS)
#define SIN_NEAR_ZERO(small_x, vx, vlen)                                       \
  __riscv_vfmadd((small_x), (vx), 0x1.0p-60, (vx), (vlen))
#define COS_NEAR_ZERO(small_x, vx, vlen)                                       \
  __riscv_vfadd((small_x), (vx), 0x1.0p0, (vlen))
#elif defined(COMPILE_FOR_SINCOSPI)
#define SIN_NEAR_ZERO(small_x, vx, vlen)                                       \
  __riscv_vfmadd((small_x), (vx), PI_HI,                                       \
                 __riscv_vfmul((small_x), (vx), PI_MID, (vlen)), (vlen))
#define COS_NEAR_ZERO(small_x, vx, vlen)                                       \
  __riscv_vfadd((small_x), (vx), 0x1.0p0, (vlen))
#else
#define FUNC_NEAR_ZERO(small_x, vx, vlen)                                      \
  __riscv_vfadd((small_x), (vx), 0x1.0p0, (vlen))
#endif

#define EXCEPTION_HANDLING_TRIG(vx, expo_x, special_args, vy_special, vlen)    \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    VBOOL NaN_Inf;                                                             \
    IDENTIFY(vclass, class_NaN | class_Inf, NaN_Inf, (vlen));                  \
    VBOOL small_x = __riscv_vmsleu((expo_x), EXP_BIAS - MAN_LEN - 5, vlen);    \
    (special_args) = __riscv_vmor(NaN_Inf, small_x, vlen);                     \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      /* Substitute Inf with sNaN */                                           \
      VBOOL id_mask;                                                           \
      IDENTIFY(vclass, class_Inf, id_mask, (vlen));                            \
      (vy_special) = FUNC_NEAR_ZERO(small_x, vx, vlen);                        \
      VFLOAT vy_NaN_Inf = __riscv_vfmerge(vx, fp_sNaN, id_mask, (vlen));       \
      vy_NaN_Inf = __riscv_vfadd(NaN_Inf, vy_NaN_Inf, vy_NaN_Inf, (vlen));     \
      (vy_special) =                                                           \
          __riscv_vmerge((vy_special), vy_NaN_Inf, NaN_Inf, (vlen));           \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)

#define EXCEPTION_HANDLING_SINCOS(vx, expo_x, special_args, vy_special,        \
                                  vz_special, vlen)                            \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    VBOOL NaN_Inf;                                                             \
    IDENTIFY(vclass, class_NaN | class_Inf, NaN_Inf, (vlen));                  \
    VBOOL small_x = __riscv_vmsleu((expo_x), EXP_BIAS - MAN_LEN - 5, vlen);    \
    (special_args) = __riscv_vmor(NaN_Inf, small_x, vlen);                     \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      /* Substitute Inf with sNaN */                                           \
      VBOOL id_mask;                                                           \
      IDENTIFY(vclass, class_Inf, id_mask, (vlen));                            \
      (vy_special) = SIN_NEAR_ZERO(small_x, vx, vlen);                         \
      (vz_special) = COS_NEAR_ZERO(small_x, vx, vlen);                         \
      VFLOAT vy_NaN_Inf = __riscv_vfmerge(vx, fp_sNaN, id_mask, (vlen));       \
      vy_NaN_Inf = __riscv_vfadd(NaN_Inf, vy_NaN_Inf, vy_NaN_Inf, (vlen));     \
      (vy_special) =                                                           \
          __riscv_vmerge((vy_special), vy_NaN_Inf, NaN_Inf, (vlen));           \
      (vz_special) =                                                           \
          __riscv_vmerge((vz_special), vy_NaN_Inf, NaN_Inf, (vlen));           \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)

// This is a macro for trigometric argument reduction for |x| >= 2^24.
#define LARGE_ARGUMENT_REDUCTION_Piby2(vx, vlen, x_large, n_xlarge, r_xlarge,  \
                                       r_delta_xlarge)                         \
  do {                                                                         \
    /* All variable are local except for those given as arguments above. */    \
    /* First, set the non-large argument to 2**30 so that we can go through    \
       the same code without worrying about unexpected behavior. */            \
    VBOOL x_not_large = __riscv_vmnot((x_large), vlen);                        \
    VFLOAT VX = __riscv_vfmerge(vx, 0x1.0p30, x_not_large, vlen);              \
                                                                               \
    /* Get exponent of VX, normalize it to 1 <= |VX| < 2 */                    \
    VINT lsb_x = U_AS_I(__riscv_vsrl(F_AS_U(VX), 52, vlen));                   \
    lsb_x =                                                                    \
        __riscv_vand(lsb_x, 0x7ff, vlen); /* this is the biased exponent */    \
    /* lsb of X is the unbiased exponent - 52, = biased_exponent - (1023+52)   \
     */                                                                        \
    lsb_x = __riscv_vsub(lsb_x, 1075, vlen);                                   \
                                                                               \
    VUINT expo_mask = VMVU_VX(0x7ff, vlen);                                    \
    expo_mask = __riscv_vsll(expo_mask, 52, vlen); /* 0x7FF0000000000000 */    \
    VFLOAT scale = U_AS_F(__riscv_vand(F_AS_U(VX), expo_mask, vlen));          \
    scale = __riscv_vfmul(scale, 0x1.0p-500, vlen);                            \
                                                                               \
    expo_mask = __riscv_vnot(expo_mask, vlen); /* 0x800FFFFFFFFFFFFF */        \
    VUINT expo_1 = VMVU_VX(0x3ff, vlen);                                       \
    expo_1 = __riscv_vsll(expo_1, 52, vlen);                                   \
                                                                               \
    VX = U_AS_F(__riscv_vand(F_AS_U(VX), expo_mask, vlen));                    \
    VX = U_AS_F(__riscv_vor(F_AS_U(VX), expo_1, vlen));                        \
    /* At this point, |VX| in in [1, 2), but lsb of the original x is recorded \
   \                                                                           \
       We figure out which portions of 2/pi is needed. Recall                  \
       that the need is to get N mod 4 and R, where  x * (2/pi) = N + R, |R|   \
   <= 1/2. So we do not need the portions of 2/pi  whose product with x is an  \
   integer >= 4 Also, from the first relevant portion of 2/pi, we only needed  \
   5 portions of 2/pi                                                          \
   \                                                                           \
       We figure out the first index of 2/pi that is needed using lsb_x        \
       This first index is FLOOR( (max(lsb_x,2) - 2) / 52 ), which can be      \
   computed as  FLOOR( (20165 * (max(lsb_x,2) - 2)) / 2^20 )                   \
    */                                                                         \
    VUINT j_start = I_AS_U(__riscv_vmax(lsb_x, 2, vlen));                      \
    j_start = __riscv_vsub(j_start, 2, vlen);                                  \
    j_start = __riscv_vmul(j_start, 20165, vlen);                              \
    j_start = __riscv_vsrl(j_start, 20, vlen);                                 \
    VUINT ind =                                                                \
        __riscv_vsll(j_start, 3, vlen); /* 8 bytes for indexing into table */  \
                                                                               \
    /*                                                                         \
       Need to compute y * 2ovpi_tbl[j] in 2 pieces, lsb(y*2ovpi_tbl[j]) is    \
       -52 + 500 - (52 (j+1)); we chose Peg = sign(2ovpi_tbl[j]) x 2^(52+53) * \
       lsb that is, sgn * 2^(501 - 52*j)                                       \
    */                                                                         \
    VFLOAT two_by_pi;                                                          \
    two_by_pi = __riscv_vluxei64(dbl_2ovpi_tbl, ind, vlen);                    \
    VUINT peg_expo = VMVU_VX(1524, vlen); /* bias + 501 */                     \
                                                                               \
    peg_expo =                                                                 \
        __riscv_vnmsac(peg_expo, 52, j_start, vlen); /* biased expo of peg */  \
    VFLOAT peg = U_AS_F(__riscv_vsll(peg_expo, 52, vlen));                     \
    peg = __riscv_vfsgnj(peg, two_by_pi, vlen);                                \
    peg = __riscv_vfsgnjx(peg, VX, vlen);                                      \
    VFLOAT S = __riscv_vfmadd(VX, two_by_pi, peg, vlen);                       \
    S = __riscv_vfsub(S, peg, vlen);                                           \
    VFLOAT s = __riscv_vfmsub(VX, two_by_pi, S, vlen);                         \
                                                                               \
    VFLOAT prod_0 = S;                                                         \
    VFLOAT prod_1 = s;                                                         \
    prod_0 = __riscv_vfmul(prod_0, scale, vlen);                               \
                                                                               \
    ind = __riscv_vadd(ind, 8, vlen);                                          \
    two_by_pi = __riscv_vluxei64(dbl_2ovpi_tbl, ind, vlen);                    \
    peg_expo = __riscv_vsub(peg_expo, 52, vlen);                               \
    peg = U_AS_F(__riscv_vsll(peg_expo, 52, vlen));                            \
    peg = __riscv_vfsgnj(peg, two_by_pi, vlen);                                \
    peg = __riscv_vfsgnjx(peg, VX, vlen);                                      \
    S = __riscv_vfmadd(VX, two_by_pi, peg, vlen);                              \
    S = __riscv_vfsub(S, peg, vlen);                                           \
    s = __riscv_vfmsub(VX, two_by_pi, S, vlen);                                \
    prod_1 = __riscv_vfadd(prod_1, S, vlen);                                   \
    VFLOAT prod_2 = I_AS_F(__riscv_vor(F_AS_I(s), F_AS_I(s), vlen));           \
    prod_1 = __riscv_vfmul(prod_1, scale, vlen);                               \
                                                                               \
    ind = __riscv_vadd(ind, 8, vlen);                                          \
    two_by_pi = __riscv_vluxei64(dbl_2ovpi_tbl, ind, vlen);                    \
    peg_expo = __riscv_vsub(peg_expo, 52, vlen);                               \
    peg = U_AS_F(__riscv_vsll(peg_expo, 52, vlen));                            \
    peg = __riscv_vfsgnj(peg, two_by_pi, vlen);                                \
    peg = __riscv_vfsgnjx(peg, VX, vlen);                                      \
    S = __riscv_vfmadd(VX, two_by_pi, peg, vlen);                              \
    S = __riscv_vfsub(S, peg, vlen);                                           \
    s = __riscv_vfmsub(VX, two_by_pi, S, vlen);                                \
    prod_2 = __riscv_vfadd(prod_2, S, vlen);                                   \
    VFLOAT prod_3 = I_AS_F(__riscv_vor(F_AS_I(s), F_AS_I(s), vlen));           \
    prod_2 = __riscv_vfmul(prod_2, scale, vlen);                               \
                                                                               \
    /*                                                                         \
       At this point, we can get N from prod_0, prod_1, prod_2                 \
       and start the summation for the reduced fraction                        \
       In case of |VX| >= 2^54, prod_0 can be set to 0                         \
       That is scale >= 2^(54-500)                                             \
    */                                                                         \
    VBOOL ignore_prod_0 = __riscv_vmfge(scale, 0x1.0p-446, vlen);              \
    prod_0 = __riscv_vfmerge(prod_0, 0.0, ignore_prod_0, vlen);                \
                                                                               \
    /*                                                                         \
       extracting the integer part of SUM prod_j;                              \
       put in precaution that the value may be too big so that                 \
       rounded integer value is not exact in FP format                         \
    */                                                                         \
    VFLOAT flt_n = __riscv_vfmul(prod_0, 0x1.0p-12, vlen);                     \
    (n_xlarge) = __riscv_vfcvt_x(flt_n, vlen);                                 \
    flt_n = __riscv_vfcvt_f((n_xlarge), vlen);                                 \
    prod_0 = __riscv_vfnmsac(prod_0, 0x1.0p12, flt_n, vlen);                   \
                                                                               \
    flt_n = __riscv_vfmul(prod_1, 0x1.0p-12, vlen);                            \
    (n_xlarge) = __riscv_vfcvt_x(flt_n, vlen);                                 \
    flt_n = __riscv_vfcvt_f((n_xlarge), vlen);                                 \
    prod_1 = __riscv_vfnmsac(prod_1, 0x1.0p12, flt_n, vlen);                   \
                                                                               \
    /* we are now safe to get N from prod_0 + prod_1 + prod_2 */               \
    flt_n = __riscv_vfadd(prod_1, prod_2, vlen);                               \
    flt_n = __riscv_vfadd(flt_n, prod_0, vlen);                                \
    (n_xlarge) = __riscv_vfcvt_x(flt_n, vlen);                                 \
    flt_n = __riscv_vfcvt_f((n_xlarge), vlen);                                 \
    prod_0 = __riscv_vfsub(prod_0, flt_n, vlen);                               \
                                                                               \
    VFLOAT r_hi = __riscv_vfadd(prod_0, prod_1, vlen);                         \
    VFLOAT r_lo = __riscv_vfsub(prod_0, r_hi, vlen);                           \
    r_lo = __riscv_vfadd(r_lo, prod_1, vlen);                                  \
                                                                               \
    VFLOAT tmp_1, tmp_2;                                                       \
    tmp_1 = __riscv_vfadd(r_hi, prod_2, vlen);                                 \
    tmp_2 = __riscv_vfsub(r_hi, tmp_1, vlen);                                  \
    tmp_2 = __riscv_vfadd(tmp_2, prod_2, vlen);                                \
    r_hi = tmp_1;                                                              \
    r_lo = __riscv_vfadd(r_lo, tmp_2, vlen);                                   \
                                                                               \
    ind = __riscv_vadd(ind, 8, vlen);                                          \
    two_by_pi = __riscv_vluxei64(dbl_2ovpi_tbl, ind, vlen);                    \
    peg_expo = __riscv_vsub(peg_expo, 52, vlen);                               \
    peg = U_AS_F(__riscv_vsll(peg_expo, 52, vlen));                            \
    peg = __riscv_vfsgnj(peg, two_by_pi, vlen);                                \
    peg = __riscv_vfsgnjx(peg, VX, vlen);                                      \
    S = __riscv_vfmadd(VX, two_by_pi, peg, vlen);                              \
    S = __riscv_vfsub(S, peg, vlen);                                           \
    s = __riscv_vfmsub(VX, two_by_pi, S, vlen);                                \
    prod_3 = __riscv_vfadd(prod_3, S, vlen);                                   \
    VFLOAT prod_4 = I_AS_F(__riscv_vor(F_AS_I(s), F_AS_I(s), vlen));           \
    prod_3 = __riscv_vfmul(prod_3, scale, vlen);                               \
                                                                               \
    tmp_1 = __riscv_vfadd(r_hi, prod_3, vlen);                                 \
    tmp_2 = __riscv_vfsub(r_hi, tmp_1, vlen);                                  \
    tmp_2 = __riscv_vfadd(tmp_2, prod_3, vlen);                                \
    r_hi = tmp_1;                                                              \
    r_lo = __riscv_vfadd(r_lo, tmp_2, vlen);                                   \
                                                                               \
    ind = __riscv_vadd(ind, 8, vlen);                                          \
    two_by_pi = __riscv_vluxei64(dbl_2ovpi_tbl, ind, vlen);                    \
    peg_expo = __riscv_vsub(peg_expo, 52, vlen);                               \
    peg = U_AS_F(__riscv_vsll(peg_expo, 52, vlen));                            \
    peg = __riscv_vfsgnj(peg, two_by_pi, vlen);                                \
    peg = __riscv_vfsgnjx(peg, VX, vlen);                                      \
    S = __riscv_vfmadd(VX, two_by_pi, peg, vlen);                              \
    S = __riscv_vfsub(S, peg, vlen);                                           \
    prod_4 = __riscv_vfadd(prod_4, S, vlen);                                   \
    prod_4 = __riscv_vfmul(prod_4, scale, vlen);                               \
                                                                               \
    tmp_1 = __riscv_vfadd(r_hi, prod_4, vlen);                                 \
    tmp_2 = __riscv_vfsub(r_hi, tmp_1, vlen);                                  \
    tmp_2 = __riscv_vfadd(tmp_2, prod_4, vlen);                                \
    r_hi = tmp_1;                                                              \
    r_lo = __riscv_vfadd(r_lo, tmp_2, vlen);                                   \
                                                                               \
    /*                                                                         \
       Finally, (r_hi + r_lo) * pi/2 is the reduced argument                   \
       we want: that is x - N * pi/2                                           \
    */                                                                         \
    (r_xlarge) = __riscv_vfmul(r_hi, PIBY2_HI, vlen);                          \
    (r_delta_xlarge) = __riscv_vfmsub(r_hi, PIBY2_HI, (r_xlarge), vlen);       \
    (r_delta_xlarge) =                                                         \
        __riscv_vfmacc((r_delta_xlarge), PIBY2_MID, r_hi, vlen);               \
    (r_delta_xlarge) = __riscv_vfmacc((r_delta_xlarge), PIBY2_HI, r_lo, vlen); \
  } while (0)
