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

