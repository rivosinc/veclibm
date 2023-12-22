// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if defined(COMPILE_FOR_SINH) || defined(COMPILE_FOR_TANH)
#define GEN_EXCEPTIONS(special_args, vx, vlen)                                 \
  __riscv_vfmadd((special_args), (vx), 0x1.0p-60, (vx), (vlen))
#else
#define GEN_EXCEPTIONS(special_args, vx, vlen)                                 \
  __riscv_vfmadd((special_args), (vx), (vx), VFMV_VF(0x1.0p0, (vlen)), (vlen))
#endif

#if defined(COMPILE_FOR_SINH) || defined(COMPILE_FOR_COSH)
#define EXCEPTION_HANDLING_HYPER(vx, expo_x, special_args, vy_special, vlen)   \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    VBOOL NaN_Inf;                                                             \
    IDENTIFY(vclass, class_NaN | class_Inf, NaN_Inf, (vlen));                  \
    VBOOL small_x = __riscv_vmsleu((expo_x), EXP_BIAS - MAN_LEN - 5, (vlen));  \
    (special_args) = __riscv_vmor(NaN_Inf, small_x, (vlen));                   \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      (vy_special) = GEN_EXCEPTIONS((special_args), (vx), (vlen));             \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)
#else
#define EXCEPTION_HANDLING_HYPER(vx, expo_x, special_args, vy_special, vlen)   \
  do {                                                                         \
    VUINT vclass = __riscv_vfclass((vx), (vlen));                              \
    VBOOL NaN_Inf;                                                             \
    IDENTIFY(vclass, class_NaN | class_Inf, NaN_Inf, (vlen));                  \
    VBOOL small_x = __riscv_vmsleu((expo_x), EXP_BIAS - MAN_LEN - 5, (vlen));  \
    (special_args) = __riscv_vmor(NaN_Inf, small_x, (vlen));                   \
    UINT nb_special_args = __riscv_vcpop((special_args), (vlen));              \
    if (nb_special_args > 0) {                                                 \
      (vy_special) = GEN_EXCEPTIONS((special_args), (vx), (vlen));             \
      VBOOL Inf;                                                               \
      IDENTIFY(vclass, class_Inf, Inf, (vlen));                                \
      VFLOAT one = VFMV_VF(fp_posOne, (vlen));                                 \
      one = __riscv_vfsgnj(one, (vx), (vlen));                                 \
      (vy_special) = __riscv_vmerge((vy_special), one, Inf, (vlen));           \
      (vx) = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
    }                                                                          \
  } while (0)
#endif

#define LOG2_INV 0x1.71547652b82fep+0
#define LOG2_HI 0x1.62e42fefa39efp-1
#define LOG2_LO 0x1.abc9e3b39803fp-56

#define ARGUMENT_REDUCTION(vx, n, r, r_delta, vlen)                            \
  do {                                                                         \
    VFLOAT n_flt = __riscv_vfmul((vx), LOG2_INV, (vlen));                      \
    (n) = __riscv_vfcvt_x(n_flt, (vlen));                                      \
    n_flt = __riscv_vfcvt_f((n), (vlen));                                      \
    (r_delta) = __riscv_vfnmsac((vx), LOG2_HI, n_flt, (vlen));                 \
    (r) = __riscv_vfnmsac((r_delta), LOG2_LO, n_flt, (vlen));                  \
    (r_delta) = __riscv_vfsub((r_delta), (r), (vlen));                         \
    (r_delta) = __riscv_vfnmsac((r_delta), LOG2_LO, n_flt, (vlen));            \
  } while (0)
