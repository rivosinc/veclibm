
#include <stdint.h>

union sui32_fp32 {int32_t si; uint32_t ui; float f;};
union sui64_fp64 {int64_t si; uint64_t ui; double f; uint32_t ui_hilo[2];};

#define ui_hilo_HI 1
#define ui_hilo_LO 0
// so that union sui64_f64 X will have X.hilo[HI] as the high bits (containing expoent)
// and X.hilo[LO] has the lower order bits (containing the lsb for example)

#define PSTEP(coeff_j, x, poly, vlen)			         		        \
  __riscv_vfmadd((poly), (x), VFMV_VF((coeff_j), (vlen)), (vlen)) 	

#define FAST2SUM(X, Y, S, s, vlen) {	                 				\
  S = __riscv_vfadd((X), (Y), (vlen));			 				\
  s = __riscv_vfsub((X), (S), (vlen)); 			 				\
  s = __riscv_vfadd((s), (Y), (vlen)); 			 				\
}

#define KNUTH2SUM(X, Y, S, s, vlen) {					        	\
  S = __riscv_vfadd((X), (Y), (vlen)); 					        	\
  VFLOAT X_hat = __riscv_vfsub((S), (Y), (vlen));                                       \
  s = __riscv_vfadd(__riscv_vfsub((X), X_hat, (vlen)),                                  \
		     __riscv_vfsub((Y), __riscv_vfsub((S), X_hat, (vlen)), (vlen)))     \
}

#define IDENTIFY2(__vclass, __stencil, __signature, __identity_mask, __vlen) \
  __signature = __riscv_vand(__vclass, __stencil, __vlen); \
  __identity_mask = __riscv_vmsgtu( __riscv_vand(__vclass, __stencil, __vlen), \
		                    0, __vlen); 

#define IDENTIFY(vclass, stencil, identity_mask, vlen)				\
  identity_mask = __riscv_vmsgtu( __riscv_vand((vclass), (stencil), (vlen)), 0, (vlen)); 

#define FCLIP(vx, x_min, x_max, vlen)					 \
  __riscv_vfmin(__riscv_vfmax((vx), X_MIN, (vlen)), X_MAX, (vlen))

#define FAST_LDEXP(num, exp, vlen) {					\
  VINT n1 =__riscv_vsra((exp), 1, (vlen));				\
  VINT n2 = __riscv_vsub((exp), n1, (vlen));				\
  n1 = __riscv_vsll(n1, MAN_LEN, (vlen));				\
  num = I_AS_F( __riscv_vadd(F_AS_I((num)), n1, (vlen)));		\
  n2 = __riscv_vadd( n2, EXP_BIAS, (vlen));				\
  n2 = __riscv_vsll(n2, MAN_LEN, (vlen));				\
  num = __riscv_vfmul((num), I_AS_F(n2), (vlen));			\
}

// Some of the functions have multiple implementations using different algorithms or styles.
// The following configure the name of each of these variations, thus allowing one to be
// set to the standard libm name.

#define RVVLM_EXPD_STD rvvlm_expD_std
#define RVVLM_EXPD_STD_R_EXTRA rvvlm_expD_r_extra
#define RVVLM_EXPD_STD_EPSIM rvvlm_expD_epsim
#define RVVLM_EXPD_TBL64  rvvlm_exp


// Define the various tables for table-driven implementations
extern int64_t expD_tbl64_fixedpt[128];


// Define the functions in the vector math library

void RVVLM_EXPD_STD(size_t x_len, const double *x, double *y);
void RVVLM_EXPD_STD_R_EXTRA(size_t x_len, const double *x, double *y);
void RVVLM_EXPD_STD_EPSIM(size_t x_len, const double *x, double *y);
void RVVLM_EXPD_TBL64(size_t x_len, const double *x, double *y);

