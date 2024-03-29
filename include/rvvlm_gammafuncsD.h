// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

// gamma(+inf) = +inf; gamma(-inf/sNaN) is qNaN with invalid
// gamma(qNaN) is qNaN
// gamma(+-0) is +-inf and divide by 0
// gamma(tiny) is 1/tiny
#define EXCEPTION_HANDLING_TGAMMA(vx, special_args, vy_special, vlen)          \
  do {                                                                         \
    VUINT expo_x = __riscv_vand(__riscv_vsrl(F_AS_U((vx)), MAN_LEN, (vlen)),   \
                                0x7FF, (vlen));                                \
    VBOOL x_small = __riscv_vmsltu(expo_x, EXP_BIAS - 60, (vlen));             \
    VBOOL x_InfNaN = __riscv_vmseq(expo_x, 0x7FF, (vlen));                     \
    (special_args) = __riscv_vmor(x_small, x_InfNaN, (vlen));                  \
    if (__riscv_vcpop((special_args), (vlen)) > 0) {                           \
      VUINT vclass = __riscv_vfclass((vx), (vlen));                            \
      VBOOL x_negInf;                                                          \
      IDENTIFY(vclass, class_negInf, x_negInf, (vlen));                        \
      (vx) = __riscv_vfmerge((vx), fp_sNaN, x_negInf, (vlen));                 \
      VFLOAT y_tmp = __riscv_vfadd(x_InfNaN, (vx), (vx), (vlen));              \
      (vy_special) = __riscv_vmerge((vy_special), y_tmp, x_InfNaN, (vlen));    \
      y_tmp = __riscv_vfrdiv(x_small, (vx), fp_posOne, (vlen));                \
      (vy_special) = __riscv_vmerge((vy_special), y_tmp, x_small, (vlen));     \
      (vx) = __riscv_vfmerge((vx), fp_posOne, (special_args), (vlen));         \
    }                                                                          \
  } while (0)
