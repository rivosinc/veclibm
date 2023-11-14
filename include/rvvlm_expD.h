
// Macros for common small-block codes
#define EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen) {     \
  VUINT vclass = __riscv_vfclass((vx), (vlen));  			 \
  IDENTIFY(vclass, class_NaN|class_Inf, (special_args), (vlen)) 	 \
  UINT nb_special_args = __riscv_vcpop((special_args), (vlen)); 	 \
  if (nb_special_args > 0) { 						 \
  /* Substitute -Inf with +0 */ 					 \
  VBOOL id_mask; 							 \
  IDENTIFY(vclass, class_negInf, id_mask, (vlen)) 			 \
  vx = __riscv_vfmerge(vx, fp_posZero, id_mask, (vlen)); 		 \
  vy_special = __riscv_vfadd((special_args), (vx), (vx), (vlen));        \
  vx = __riscv_vfmerge((vx), fp_posZero, (special_args), (vlen));        \
  } 									 \
}	

#define LOG2_INV 0x1.71547652b82fep+0
#define LOG2_HI  0x1.62e42fefa39efp-1
#define LOG2_LO  0x1.abc9e3b39803fp-56

#define LOG2_INV_64   0x1.71547652b82fep+6
#define LOG2_BY64_HI  0x1.62e42fefa39efp-7
#define LOG2_BY64_LO  0x1.abc9e3b39803fp-62

#define X_MAX  0x1.65p+9
#define X_MIN -0x1.77p+9

