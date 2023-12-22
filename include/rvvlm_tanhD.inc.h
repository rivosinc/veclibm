// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "rvvlm_hyperbolicsD.h"

#include <fenv.h>

#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_TANHD_STD
#else
#define F_VER1 RVVLM_TANHDI_STD
#endif

#include <fenv.h>

// This versions reduces argument to [-log2/2, log2/2]
// Exploit common expressions exp(R) and exp(-R),
// and uses purely floating-point computation
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx_orig, vx, vy, vy_special;
  VBOOL special_args;
  VUINT expo_x;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx_orig = VFLOAD_INARG1(vlen);
    expo_x =
        __riscv_vand(__riscv_vsrl(F_AS_U(vx_orig), MAN_LEN, vlen), 0x7FF, vlen);

    // Set results for input of NaN and Inf and also for |x| very small
    EXCEPTION_HANDLING_HYPER(vx_orig, expo_x, special_args, vy_special, vlen);

    // tanh(x) = sign(x) * tanh(|x|); suffices to work on |x| for the main part
    vx = __riscv_vfsgnj(vx_orig, fp_posOne, vlen);

    // Suffices to clip |x| to 20, which is bigger than 28 log(2)
    vx = __riscv_vfmin(vx, 0x1.4p4, vlen);
    VINT n;
    VFLOAT r, r_delta;
    // tanh(x) = (1 - exp(-2x)) / (1 + exp(-2x)); so we compute exp(-2x)
    // by replacing x by -2x
    vx = __riscv_vfmul(vx, -0x1.0p1, vlen);
    ARGUMENT_REDUCTION(vx, n, r, r_delta, vlen);
    ////////////////////

    // exp(x) = 2^n exp(r'), r' = r + r_delta and thus we compute 1 +/- exp(x)
    // as 1 +/- 2^(n)(1 + r' + (r')^2/2 + r^3 p(r)) (1 +/- s) +/- s(r' +
    // (r')^2/2) +/- s r^3 p(r) To maintain good precision, 1 +/- s and r' +
    // (r')^2/2 are computed to extra precision in a leading term and a
    // correctional term. This leads to representing 1 +/- exp(x) in a leading
    // and correctional term.

    VFLOAT s =
        I_AS_F(__riscv_vsll(__riscv_vadd(n, EXP_BIAS, vlen), MAN_LEN, vlen));
    VBOOL s_is_small = __riscv_vmsle(n, -(MAN_LEN + 1), vlen);
    VBOOL s_not_small = __riscv_vmnot(s_is_small, vlen);
    // 1 +/- s is exact when s is not small
    VFLOAT s_head = __riscv_vfmerge(s, fp_posZero, s_is_small, vlen);
    VFLOAT s_tail = __riscv_vfmerge(s, fp_posZero, s_not_small, vlen);
    // s_head + s_tail = s; and 1 +/- s is (1 +/- s_head) +/- s_tail

    // exp(r') is approximated by 1 + r' + (r')^2/2 + r^3(p_even(r^2) +
    // r*p_odd(r^2)) using r without delta_r sufficies from the third order
    // onwards
    VFLOAT rsq = __riscv_vfmul(r, r, vlen);
    VFLOAT rcube = __riscv_vfmul(rsq, r, vlen);

    VFLOAT p_even = PSTEP(0x1.555555555555ap-3, rsq,
                          PSTEP(0x1.111111110ef6ap-7, rsq,
                                PSTEP(0x1.a01a01b32b633p-13, rsq,
                                      PSTEP(0x1.71ddef82f4beep-19,
                                            0x1.af6eacd796f0bp-26, rsq, vlen),
                                      vlen),
                                vlen),
                          vlen);

    VFLOAT p_odd = PSTEP(
        0x1.5555555553aefp-5, rsq,
        PSTEP(0x1.6c16c17a09506p-10, rsq,
              PSTEP(0x1.a019b37a2b3dfp-16, 0x1.289788d8bdadfp-22, rsq, vlen),
              vlen),
        vlen);

    VFLOAT poly = __riscv_vfmadd(p_odd, r, p_even, vlen);
    // r^3 * poly will be r^3(...)
    // we delay this multiplication with r^3 for now

    // Compute r' + (r')^2/2 extra precisely
    VFLOAT r_prime = __riscv_vfmul(r, 0x1.0p-1, vlen);
    VFLOAT B = __riscv_vfmadd(r, r_prime, r, vlen);
    VFLOAT b = __riscv_vfsub(r, B, vlen);
    b = __riscv_vfmacc(b, r, r_prime, vlen);
    // B + b is r' + (r')^2/2 extra precisely
    // incoporate r_delta in R + R^2/2
    VFLOAT c = __riscv_vfmadd(r, r_delta, r_delta, vlen);
    b = __riscv_vfadd(b, c, vlen);
    poly = __riscv_vfmadd(poly, rcube, b, vlen);
    // B + poly is r' + (r')^2/2 + r^3(.....)
    // and exp(r') is well approximated by s*(1 + B + poly)

    // We compute the denominator 1 + exp(R) first as
    // we will need to recipricate afterwards, the latency of which
    // can be hidden somewhat by proceeding with the numerator
    // at that time
    VFLOAT Z = __riscv_vfadd(s_head, fp_posOne, vlen);
    VFLOAT D_tmp = __riscv_vfmadd(B, s, Z, vlen);
    VFLOAT d_tmp = __riscv_vfsub(Z, D_tmp, vlen);
    d_tmp = __riscv_vfmacc(d_tmp, s, B, vlen);
    d_tmp = __riscv_vfadd(d_tmp, s_tail, vlen);
    d_tmp = __riscv_vfmacc(d_tmp, s, poly, vlen);
    // D_tmp + d_tmp is 1 + exp(R) to high precision, but we have to
    // normalize this representation so that the leading term
    // has full FP64 precision of this sum
    VFLOAT D, d;
    FAST2SUM(D_tmp, d_tmp, D, d, vlen);
    // VFLOAT D = __riscv_vfadd(D_tmp, d, vlen);
    // Z = __riscv_vfsub(D_tmp, D, vlen);
    // d = __riscv_vfadd(Z, d, vlen);

    // Now start to compute 1/(D+d) as E + e
    VFLOAT One = VFMV_VF(fp_posOne, vlen);
    VFLOAT E, e;
    DIV_N1D2(One, D, d, E, e, vlen);
    // E + e is 1/(D+d) to extra precision

    // Overlap much of the 1/(D+d) computation with
    // computing 1 - s(1 + B + poly)
    Z = __riscv_vfrsub(s_head, fp_posOne, vlen);

    VFLOAT Numer = __riscv_vfnmsub(B, s, Z, vlen);
    VFLOAT numer = __riscv_vfsub(Z, Numer, vlen);
    numer = __riscv_vfnmsac(numer, s, B, vlen);

    // Numer + numer = Z - s * B accurately
    numer = __riscv_vfsub(numer, s_tail, vlen);
    numer = __riscv_vfnmsac(numer, s, poly, vlen);

    // (Numer + numer) * (E + e)
    // Numer * E + ( numer * E + (Numer * e + (e*numer)) )
    vy = __riscv_vfmul(e, numer, vlen);
    vy = __riscv_vfmacc(vy, Numer, e, vlen);
    vy = __riscv_vfmacc(vy, numer, E, vlen);
    vy = __riscv_vfmacc(vy, Numer, E, vlen);

    vy = __riscv_vfsgnj(vy, vx_orig, vlen);
    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
