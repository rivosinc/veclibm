
#include <stdio.h>
#include <riscv_vector.h>

#include "rvvlm.h"
#include "rvvlm_fp64m1.h"
#include "rvvlm_expD.h"

#define DEBUG 0

void RVVLM_EXPD_STD(size_t x_len, const double *x, double *y) {
    size_t vlen;
    VFLOAT vx, vy, vy_special;
    VBOOL special_args;

    // stripmining over input arguments
    for (; x_len > 0; x_len -= vlen) {
        vlen = VSET(x_len);
        vx = VFLOAD(x, vlen);

	// Set results for input of NaN and Inf; substitute them with zero
	EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen)
 
        // Clip
	vx = FCLIP(vx, X_MIN, X_MAX, vlen);

        // Argument reduction
        VFLOAT flt_n = __riscv_vfmul(vx, LOG2_INV, vlen);
        VINT n = __riscv_vfcvt_x(flt_n, vlen);
        flt_n = __riscv_vfcvt_f(n, vlen);
        VFLOAT r = __riscv_vfnmsac(vx, LOG2_HI, flt_n, vlen);
        r = __riscv_vfnmsac(r, LOG2_LO, flt_n, vlen);

        // Polynomial computation, we have a degree 11
        // We compute the part from r^3 in three segments, increasing parallelism
	// Ideally the compiler will interleave the computations of the segments
	VFLOAT poly_right = PSTEP(  0x1.71df804f1baa1p-19, r,
                            PSTEP(  0x1.28aa3ea739296p-22, 0x1.acf86201fd199p-26, r,
                            vlen), vlen);

	VFLOAT poly_mid = PSTEP( 0x1.6c16c1825c970p-10, r,
                          PSTEP( 0x1.a01a00fe6f730p-13, 0x1.a0199e1789c72p-16, r, 
                          vlen), vlen);

	VFLOAT poly_left = PSTEP( 0x1.55555555554d2p-3, r, 
                           PSTEP( 0x1.5555555551307p-5, 0x1.11111111309a4p-7, r,  
                           vlen), vlen);

        VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
        VFLOAT r_cube = __riscv_vfmul(r_sq, r, vlen);

        VFLOAT poly = __riscv_vfmadd(poly_right, r_cube, poly_mid, vlen);
        poly = __riscv_vfmadd(poly, r_cube, poly_left, vlen);

	poly = PSTEP( 0x1.0000000000007p-1, r, poly, vlen);

        r = __riscv_vfmacc(r, r_sq, poly, vlen);
        vy = __riscv_vfadd(r, 0x1.0p0, vlen);

        // at this point, vy is the entire degree-11 polynomial
        // vy ~=~ exp(r)

        // Need to compute 2^n * exp(r). 
	FAST_LDEXP(vy, n, vlen)

	// Incorporate results of exceptional inputs
	vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

        // copy vy into y and increment addr pointers
        VSE (y, vy, vlen);

        x += vlen; y += vlen;
    }
}

void RVVLM_EXPD_STD_R_EXTRA(size_t x_len, const double *x, double *y) {
    size_t vlen;
    VFLOAT vx, vy, vy_special;
    VBOOL special_args;

    // stripmining over input arguments
    for (; x_len > 0; x_len -= vlen) {
        vlen = VSET(x_len);
        vx = VFLOAD(x, vlen);

	// Set results for input of NaN and Inf; substitute them with zero
	EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen)
 
        // Clip
	vx = FCLIP(vx, X_MIN, X_MAX, vlen);

        // Argument reduction
        VFLOAT flt_n = __riscv_vfmul(vx, LOG2_INV, vlen);
        VINT n = __riscv_vfcvt_x(flt_n, vlen);
        flt_n = __riscv_vfcvt_f(n, vlen);
        VFLOAT r_hi = __riscv_vfnmsac(vx, LOG2_HI, flt_n, vlen);
        VFLOAT r_lo = __riscv_vfmul(flt_n, LOG2_LO, vlen);
        VFLOAT r = __riscv_vfsub(r_hi, r_lo, vlen);

        // Polynomial computation, we have a degree 11
        // We compute the part from r^3 in three segments, increasing parallelism
	// Ideally the compiler will interleave the computations of the segments
	VFLOAT poly_right = PSTEP(  0x1.71df804f1baa1p-19, r,
                            PSTEP(  0x1.28aa3ea739296p-22, 0x1.acf86201fd199p-26, r,
                            vlen), vlen);

	VFLOAT poly_mid = PSTEP( 0x1.6c16c1825c970p-10, r,
                          PSTEP( 0x1.a01a00fe6f730p-13, 0x1.a0199e1789c72p-16, r, 
                          vlen), vlen);

	VFLOAT poly_left = PSTEP( 0x1.55555555554d2p-3, r, 
                           PSTEP( 0x1.5555555551307p-5, 0x1.11111111309a4p-7, r,  
                           vlen), vlen);

        VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
        VFLOAT r_cube = __riscv_vfmul(r_sq, r, vlen);

        VFLOAT poly = __riscv_vfmadd(poly_right, r_cube, poly_mid, vlen);
        poly = __riscv_vfmadd(poly, r_cube, poly_left, vlen);

	poly = PSTEP( 0x1.0000000000007p-1, r, poly, vlen);

        poly = __riscv_vfmsub(poly, r_sq, r_lo, vlen);
        vy = __riscv_vfadd(poly, r_hi, vlen);
        vy = __riscv_vfadd(vy, 0x1.0p0, vlen);
        // at this point, vy is the entire degree-11 polynomial
        // vy ~=~ exp(r)

        // Need to compute 2^n * exp(r). 
	FAST_LDEXP(vy, n, vlen)

	// Incorporate results of exceptional inputs
	vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

        // copy vy into y and increment addr pointers
        VSE (y, vy, vlen);

        x += vlen; y += vlen;
    }
}

// This one computes 1 + r + r^2/2 to extra precision using FP techniques
// The r in this expression is the extra-precise reduced argument
void RVVLM_EXPD_STD_EPSIM(size_t x_len, const double *x, double *y) {
    size_t vlen;
    VFLOAT vx, vy, vy_special;
    VBOOL special_args;
    double Peg = 0x1.8p+27;
    
    // stripmining over input arguments
    for (; x_len > 0; x_len -= vlen) {
        vlen = VSET(x_len);
        vx = VFLOAD(x, vlen);

	// Set results for input of NaN and Inf; substitute them with zero
	EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen)
 
        // Clip
	vx = FCLIP(vx, X_MIN, X_MAX, vlen);

        // Argument reduction
        VFLOAT flt_n = __riscv_vfmul(vx, LOG2_INV, vlen);
        VINT n = __riscv_vfcvt_x(flt_n, vlen);
        flt_n = __riscv_vfcvt_f(n, vlen);

        VFLOAT r = __riscv_vfnmsac(vx, LOG2_HI, flt_n, vlen);
        VFLOAT r_hi = __riscv_vfadd(r, Peg , vlen);
        r_hi = __riscv_vfsub(r_hi, Peg, vlen);
        // r_hi has lsb at 2^(-25)
        VFLOAT r_lo = __riscv_vfsub(r, r_hi, vlen);
        r_lo = __riscv_vfnmsac(r_lo, LOG2_LO, flt_n, vlen);
	r = __riscv_vfadd(r_hi, r_lo, vlen);	

        // r_hi + r_lo is extra-precise reduced argument
        // and also 1 + r_hi + r_hi^2/2 is computable error-free
	// 1 + (r_hi + r_lo) + (r_hi + r_lo)^2/2  is
        // P_head + P_tail where
        // P_head = 1 + r_hi + r_hi^2/2 = 1 + r_hi*(1 + r_hi/2) 
        // P_tail = r_lo + r_lo(r_hi + r_lo/2)

	VFLOAT P_head = PSTEP( 0x1.0p0, r_hi, 
			PSTEP( 0x1.0p0, 0x1.0p-1, r_hi,
			vlen), vlen);	

        VFLOAT P_tail = __riscv_vfmacc(r_hi, 0x1.0p-1, r_lo, vlen);
        P_tail = __riscv_vfmadd(P_tail, r_lo, r_lo, vlen);

        // Polynomial computation, we have a degree 11
        // We compute the part from r^3 in three segments, increasing parallelism
	// Ideally the compiler will interleave the computations of the segments
	VFLOAT poly_right = PSTEP( 0x1.71dfc3d471117p-19, r,
                            PSTEP( 0x1.289feff6e458ap-22, 0x1.ac82ecdeaa346p-26, r,  
                            vlen), vlen);

	VFLOAT poly_mid = PSTEP( 0x1.6c16c17ce42b2p-10, r,
                          PSTEP( 0x1.a01a00e44cd99p-13, 0x1.a019aac62dedbp-16, r, 
                          vlen), vlen);

	VFLOAT poly_left = PSTEP( 0x1.55555555554cap-3, r,
                           PSTEP( 0x1.555555555319fp-5,  0x1.1111111134581p-7, r,
                           vlen), vlen);

        VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
        VFLOAT r_cube = __riscv_vfmul(r_sq, r, vlen);

        VFLOAT poly = __riscv_vfmadd(poly_right, r_cube, poly_mid, vlen);
        poly = __riscv_vfmadd(poly, r_cube, poly_left, vlen);

        // poly_lo = __riscv_vfmadd(poly_lo, r_cube, poly_mid, vlen);
        // poly_lo = __riscv_vfmadd(poly_lo, r_cube, poly_hi, vlen);

        poly = __riscv_vfmadd(poly, r_cube, P_tail, vlen);
        vy = __riscv_vfadd(poly, P_head, vlen);
        // at this point, vy is the entire degree-11 polynomial
        // vy ~=~ exp(r)
	
        // Need to compute 2^n * exp(r). 
	FAST_LDEXP(vy, n, vlen)

	// Incorporate results of exceptional inputs
	vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

        // copy vy into y and increment addr pointers
        VSE (y, vy, vlen);

        x += vlen; y += vlen;
    }
}

// prototype of simple table-driven exp(x), no exceptional arguments
// The reduced argument is in FP64 only, without extra-precise compensation
// Basic steps
// 1. n = round-int( X * 64/log2 ); r := X - N * log(2)/64 (but log(2)/64 need extra precision)
// 2. Approximate exp(r) by 1 + poly(r)  poly(r) = r*(p4 + r(p3 + r(p2 + r(p1 + r*p0))))
void RVVLM_EXPD_TBL64(size_t x_len, const double *x, double *y) {
    size_t vlen;
    VFLOAT vx, vy, vy_special;
    VBOOL special_args;
    
    // stripmining over input arguments
    for (; x_len > 0; x_len -= vlen) {
        vlen = VSET(x_len);
        vx = VFLOAD(x, vlen);

	// Set results for input of NaN and Inf; substitute them with zero
	EXCEPTION_HANDLING_EXP(vx, special_args, vy_special, vlen)
 
        // Clip
	vx = FCLIP(vx, X_MIN, X_MAX, vlen);

        // Argument reduction
        VFLOAT flt_n = __riscv_vfmul(vx, LOG2_INV_64, vlen);
        VINT n = __riscv_vfcvt_x(flt_n, vlen);
        flt_n = __riscv_vfcvt_f(n, vlen);
        VFLOAT r = __riscv_vfnmsac(vx, LOG2_BY64_HI, flt_n, vlen);
        r = __riscv_vfnmsac(r, LOG2_BY64_LO, flt_n, vlen);

        // Polynomial computation, we have a degree 5
        // We break this up into 2 pieces
	// Ideally the compiler will interleave the computations of the segments
	VFLOAT poly_right = PSTEP(0x1.5555722e87735p-5, 0x1.1107f5fc29bb7p-7, r, vlen);
	VFLOAT poly_left = PSTEP(0x1.fffffffffe1f5p-2, 0x1.55555556582a8p-3, r, vlen);
        VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
        VFLOAT poly = __riscv_vfmadd(poly_right, r_sq, poly_left, vlen);
        poly = __riscv_vfmadd(poly, r_sq, r, vlen);
        poly = __riscv_vfmul(poly, 0x1.0p63, vlen);

        VINT P = __riscv_vfcvt_x(poly, vlen);
        VINT j = __riscv_vand(n, 0x3f, vlen);
        j = __riscv_vsll(j, 3, vlen);
        VINT T = __riscv_vloxei64(expD_tbl64_fixedpt, I_AS_U(j), vlen);

        P = __riscv_vsmul(P, T, 1, vlen);
        P = __riscv_vsadd(P, T, vlen);
        vy = __riscv_vfcvt_f(P, vlen);
        // at this point, vy ~=~ 2^62 * exp(r)

        // Need to compute 2^(n-62) * exp(r). 
        n = __riscv_vsra(n, 6, vlen);
        
        n = __riscv_vsub(n, 62, vlen);
        FAST_LDEXP(vy, n, vlen)

	vy = __riscv_vmerge(vy, vy_special, special_args, vlen);

        // copy vy into y and increment addr pointers
        // VSE (y, vy, vlen);
	VSE(y, vy, vlen);

        x += vlen; y += vlen;
    }
}

