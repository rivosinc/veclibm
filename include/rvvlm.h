//
//   Copyright 2023  Rivos Inc.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

union sui32_fp32 {int32_t si; uint32_t ui; float f;};
union sui64_fp64 {int64_t si; uint64_t ui; double f; uint32_t ui_hilo[2];};

#define ui_hilo_HI 1
#define ui_hilo_LO 0
// so that union sui64_f64 X will have X.hilo[HI] as the high bits (containing expoent)
// and X.hilo[LO] has the lower order bits (containing the lsb for example)

#define API_SIGNAUTRE_11 1
#define API_SIGNATURE_21 2
#define API_SIGNATURE_12 3
#define API_SIGNATURE_22 4

#define UNIT_STRIDE 1
#define GENERAL_STRIDE 2

#define SET_ROUNDTONEAREST \
int original_frm; bool need_to_restore; \
do { \
  (original_frm) = fegetround(); \
  need_to_restore = (original_frm != FE_TONEAREST); \
} while(0)

#define RESTORE_FRM \
do { \
  if (need_to_restore) {fesetround((original_frm));} \
} while(0)


#define PSTEP(coeff_j, x, poly, vlen) \
  __riscv_vfmadd((poly), (x), VFMV_VF((coeff_j), (vlen)), (vlen))   

#define FAST2SUM(X, Y, S, s, vlen)                              \
do {                                                            \
  S = __riscv_vfadd((X), (Y), (vlen));                          \
  s = __riscv_vfsub((X), (S), (vlen));                          \
  s = __riscv_vfadd((s), (Y), (vlen));                          \
} while(0)

#define POS2SUM(X, Y, S, s, vlen) \
do { \
  VFLOAT _first = __riscv_vfmax((X), (Y), (vlen)); \
  VFLOAT _second = __riscv_vfmin((X), (Y), (vlen)); \
  S = __riscv_vfadd((X), (Y), (vlen)); \
  s = __riscv_vfadd(__riscv_vfsub(_first, (S), (vlen)), _second, (vlen)); \
} while(0)

#define KNUTH2SUM(X, Y, S, s, vlen)                                                 \
do {                                                                                \
  S = __riscv_vfadd((X), (Y), (vlen));                                              \
  VFLOAT X_hat = __riscv_vfsub((S), (Y), (vlen));                                   \
  s = __riscv_vfadd(__riscv_vfsub((X), X_hat, (vlen)),                              \
             __riscv_vfsub((Y), __riscv_vfsub((S), X_hat, (vlen)), (vlen)), (vlen));\
} while(0)

#define IDENTIFY2(__vclass, __stencil, __signature, __identity_mask, __vlen)    \
do {                                                                            \
  __signature = __riscv_vand(__vclass, __stencil, __vlen);                      \
  __identity_mask = __riscv_vmsgtu( __riscv_vand(__vclass, __stencil, __vlen),  \
                            0, __vlen)                                          \
} while(0)

#define IDENTIFY(vclass, stencil, identity_mask, vlen) \
  identity_mask = __riscv_vmsgtu( __riscv_vand((vclass), (stencil), (vlen)), 0, (vlen)); 

#define FCLIP(vx, x_min, x_max, vlen) \
  __riscv_vfmin(__riscv_vfmax((vx), X_MIN, (vlen)), X_MAX, (vlen))

#define FAST_LDEXP(num, exp, vlen)                          \
do {                                                        \
  VINT n1 =__riscv_vsra((exp), 1, (vlen));                  \
  VINT n2 = __riscv_vsub((exp), n1, (vlen));                \
  n1 = __riscv_vsll(n1, MAN_LEN, (vlen));                   \
  num = I_AS_F( __riscv_vadd(F_AS_I((num)), n1, (vlen)));   \
  n2 = __riscv_vadd( n2, EXP_BIAS, (vlen));                 \
  n2 = __riscv_vsll(n2, MAN_LEN, (vlen));                   \
  num = __riscv_vfmul((num), I_AS_F(n2), (vlen));           \
} while(0)

// Some of the functions have multiple implementations using different algorithms or styles.
// The following configure the name of each of these variations, thus allowing one to be
// set to the standard libm name.

// FP64 exp function configuration
#define RVVLM_EXPD_VSET_CONFIG "rvvlm_fp64m4.h"
#define RVVLM_EXPD_STD rvvlm_expD_std
#define RVVLM_EXPD_STD_R_EXTRA rvvlm_exp
#define RVVLM_EXPD_STD_EPSIM rvvlm_expD_std_epsim
#define RVVLM_EXPD_TBL64  rvvlm_expD_tbl64

#define RVVLM_EXPDI_VSET_CONFIG "rvvlm_fp64m4.h"
#define RVVLM_EXPDI_STD rvvlm_expID_std
#define RVVLM_EXPDI_STD_R_EXTRA rvvlm_expI
#define RVVLM_EXPDI_STD_EPSIM rvvlm_expDI_std_epsim
#define RVVLM_EXPDI_TBL64  rvvlm_expDI_tbl64

// FP64 exp2 function configuration
#define RVVLM_EXP2D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_EXP2D_STD rvvlm_exp2D_std
#define RVVLM_EXP2D_STD_R_EXTRA rvvlm_exp2
#define RVVLM_EXP2D_STD_EPSIM rvvlm_exp2D_std_epsim
#define RVVLM_EXP2D_TBL64  rvvlm_exp2D_tbl64

#define RVVLM_EXP2DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_EXP2DI_STD rvvlm_exp2ID_std
#define RVVLM_EXP2DI_STD_R_EXTRA rvvlm_exp2I
#define RVVLM_EXP2DI_STD_EPSIM rvvlm_exp2DI_std_epsim
#define RVVLM_EXP2DI_TBL64  rvvlm_exp2DI_tbl64

// FP64 exp10 function configuration
#define RVVLM_EXP10D_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_EXP10D_STD rvvlm_exp10D_std
#define RVVLM_EXP10D_STD_R_EXTRA rvvlm_exp10
#define RVVLM_EXP10D_STD_EPSIM rvvlm_exp10D_std_epsim
#define RVVLM_EXP10D_TBL64  rvvlm_exp10D_tbl64

#define RVVLM_EXP10DI_VSET_CONFIG "rvvlm_fp64m1.h"
#define RVVLM_EXP10DI_STD rvvlm_exp10ID_std
#define RVVLM_EXP10DI_STD_R_EXTRA rvvlm_exp10I
#define RVVLM_EXP10DI_STD_EPSIM rvvlm_exp10DI_std_epsim
#define RVVLM_EXP10DI_TBL64  rvvlm_exp10DI_tbl64

// FP64 expm1 function configuration
#define RVVLM_EXPM1D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_EXPM1D_STD_EPSIM rvvlm_expm1

#define RVVLM_EXPM1DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_EXPM1DI_STD_EPSIM rvvlm_expm1I

// FP64 log function configuration
#define RVVLM_LOGD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOGD_TBL128 rvvlm_log

#define RVVLM_LOGDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOGDI_TBL128 rvvlm_logI

#define RVVLM_LOG2D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG2D_TBL128 rvvlm_log2

#define RVVLM_LOG2DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG2DI_TBL128 rvvlm_log2I

#define RVVLM_LOG10D_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG10D_TBL128 rvvlm_log10

#define RVVLM_LOG10DI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG10DI_TBL128 rvvlm_log10I

// FP64 log1p function configuration
#define RVVLM_LOG1PD_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG1PD_TBL128 rvvlm_log1p

#define RVVLM_LOG1PDI_VSET_CONFIG "rvvlm_fp64m2.h"
#define RVVLM_LOG1PDI_TBL128 rvvlm_log1pI

// Define the various tables for table-driven implementations
extern int64_t expD_tbl64_fixedpt[64];
extern int64_t logD_tbl128_fixedpt[128];

// Define the functions in the vector math library
void RVVLM_EXPD_STD(size_t x_len, const double *x, double *y);
void RVVLM_EXPDI_STD(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXPD_STD_R_EXTRA(size_t x_len, const double *x, double *y);
void RVVLM_EXPDI_STD_R_EXTRA(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXPD_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXPDI_STD_EPSIM(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXPD_TBL64(size_t x_len, const double *x, double *y);
void RVVLM_EXPDI_TBL64(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);

void RVVLM_EXP2D_STD(size_t x_len, const double *x, double *y);
void RVVLM_EXP2DI_STD(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXP2D_STD_R_EXTRA(size_t x_len, const double *x, double *y);
void RVVLM_EXP2DI_STD_R_EXTRA(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXP2D_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXP2DI_STD_EPSIM(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXP2D_TBL64(size_t x_len, const double *x, double *y);
void RVVLM_EXP2DI_TBL64(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);

void RVVLM_EXP10D_STD(size_t x_len, const double *x, double *y);
void RVVLM_EXP10DI_STD(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXP10D_STD_R_EXTRA(size_t x_len, const double *x, double *y);
void RVVLM_EXP10DI_STD_R_EXTRA(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXP10D_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXP10DI_STD_EPSIM(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);
void RVVLM_EXP10D_TBL64(size_t x_len, const double *x, double *y);
void RVVLM_EXP10DI_TBL64(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);

void RVVLM_EXPM1D_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXPM1DI_STD_EPSIM(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);

void RVVLM_LOGD_TBL128(size_t x_len, const double *x, double *y);
void RVVLM_LOGDI_TBL128(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);

void RVVLM_LOG10D_TBL128(size_t x_len, const double *x, double *y);
void RVVLM_LOG10DI_TBL128(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);

void RVVLM_LOG2D_TBL128(size_t x_len, const double *x, double *y);
void RVVLM_LOG2DI_TBL128(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);

void RVVLM_LOG1PD_TBL128(size_t x_len, const double *x, double *y);
void RVVLM_LOG1PDI_TBL128(size_t x_len, const double *x, size_t stride_x, double *y, size_t stride_y);


#ifdef __cplusplus
}
#endif
