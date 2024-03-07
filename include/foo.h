
#define PSTEP_I_SLL(COEFF_j, X, K, POLY, vlen)                                 \
  __riscv_vsadd(__riscv_vsll(                                                  \
    __riscv_vsmul((POLY), (X), 1, (vlen)), (K), (vlen)),                       \
        (COEFF_j), (vlen))

#define PSTEP_I_SRA(COEFF_j, X, K, POLY, vlen)                                 \
  __riscv_vsadd(__riscv_vsra(                                                  \
    __riscv_vsmul((POLY), (X), 1, (vlen)), (K), (vlen)),                       \
        (COEFF_j), (vlen))

