// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

// Macros for common small-block codes
#define EXCEPTION_HANDLING_POW(vx, vy, special_args, vz_special, vlen) \
VUINT vclass_x = __riscv_vfclass(vx, vlen); \
VUINT vclass_y = __riscv_vfclass(vy, vlen); \
do { \
/* Exception handling: handle x or y being NaN, Inf, and Zero  \
 * and replace them with 2.0 so that normal computations with them \
 * do not raise problems. \
 * Note that we do not call out negative x for special handling. The \
 * normal computation essentially computes |x|^y, but identify x < 0 \
 * later on; replacing the answer appropriately depending on whether \
 * y is an integer (resulting in +-(|x|^y)) or not (resulting in NaN). \
 * \
 * In side the special argument handling, we handle 3 cases separately \
 * x AND y both special, only x is special, and only y is special. \
 */ \
 \
    VBOOL y_special, x_special; \
    /* 0x399 is NaN/Inf/Zero */ \
    IDENTIFY(vclass_y, 0x399, y_special, vlen); \
    IDENTIFY(vclass_x, 0x399, x_special, vlen); \
 \
    special_args = __riscv_vmor(x_special, y_special, vlen); \
    UINT nb_special_args = __riscv_vcpop(special_args, vlen); \
 \
    if (nb_special_args > 0) { \
        /* Expect this to be taken rarely. We handle separately the three  \
         * mutually exclusive cases of both x and y are special, and only one is special \
         */ \
        VUINT vclass_z; \
        VBOOL id_mask; \
        vz_special = VFMV_VF(fp_posOne, vlen); \
        VBOOL current_cases = __riscv_vmand(x_special, y_special, vlen); \
        if (__riscv_vcpop(current_cases, vlen) > 0) { \
            /* x AND y are special */ \
 \
            /* pow(any, 0) is 1.0 */ \
            IDENTIFY(vclass_y, class_Zero, id_mask, vlen); \
            id_mask = __riscv_vmand(id_mask, current_cases, vlen); \
            vy = __riscv_vfmerge(vy, fp_posOne, id_mask, vlen); \
            vx = __riscv_vfmerge(vx, fp_posOne, id_mask, vlen); \
            VBOOL restricted_cases = __riscv_vmandn(current_cases, id_mask, vlen); \
 \
            /* pow(+-Inf,+-Inf) = pow(+Inf,+-Inf), so substitue -Inf by +Inf for x */ \
            IDENTIFY(vclass_x, class_negInf, id_mask, vlen); \
            id_mask = __riscv_vmand(id_mask, restricted_cases, vlen); \
            vx = __riscv_vfmerge(vx, fp_posInf, id_mask, vlen); \
 \
            /* pow(0, +-Inf) = +Inf or 0. Substitute x by -Inf to mimic log(x) */ \
            IDENTIFY(vclass_x, class_Zero, id_mask, vlen); \
            id_mask = __riscv_vmand(id_mask, restricted_cases, vlen); \
            vx = __riscv_vfmerge(vx, fp_negInf, id_mask, vlen); \
 \
            /* multiply the substituted vx * vy that mimics y*log(x) to some extent.  \
             * This product will also generate the necessary NaN and invalid operation signal \
             */ \
            vz_special = __riscv_vfmul_mu(current_cases, vz_special, vx, vy, vlen); \
            vclass_z = __riscv_vfclass(vz_special, vlen); \
            IDENTIFY(vclass_z, class_negInf, id_mask, vlen); \
            id_mask = __riscv_vmand(id_mask, current_cases, vlen); \
            vz_special = __riscv_vfmerge(vz_special, fp_posZero, id_mask, vlen); \
            /* end of handling for BOTH x and y are special */ \
        } \
 \
 \
        current_cases = __riscv_vmandn(x_special, y_special, vlen); \
        if (__riscv_vcpop(current_cases, vlen) > 0) { \
            /* x only is special */ \
 \
            VINT sign_x = __riscv_vand(F_AS_I(vx), F_AS_I(vx), vlen); \
            /* Here we change x that is +-Inf into +Inf, and x that is +-0 to -Inf */ \
            IDENTIFY(vclass_x, class_Zero, id_mask, vlen); \
            id_mask = __riscv_vmand(id_mask, current_cases, vlen); \
            vx = __riscv_vfmerge(vx, fp_negInf, id_mask, vlen); \
 \
            IDENTIFY(vclass_x, class_Inf, id_mask, vlen); \
            id_mask = __riscv_vmand(id_mask, current_cases, vlen); \
            vx = __riscv_vfmerge(vx, fp_posInf, id_mask, vlen); \
 \
            /* We need to identify whether y is of integer value and if so its parity. \
             * We first clip y values to +-2^53, because FP value of this magnitude and beyond  \
             * are always even integers \
             */ \
            vy = __riscv_vfmin_mu(current_cases, vy, vy, 0x1.0p53, vlen); \
            vy = __riscv_vfmax_mu(current_cases, vy, vy, -0x1.0p53, vlen); \
            VINT y_to_int = __riscv_vfcvt_x(current_cases, vy, vlen); \
            VFLOAT y_to_int_fp = __riscv_vfcvt_f(current_cases, y_to_int, vlen); \
            VBOOL y_is_int = __riscv_vmfeq(current_cases, vy, y_to_int_fp, vlen); \
            VINT sign_z = __riscv_vsll(y_to_int, 63, vlen);  \
            /* the parity is used later on to manipulate sign, hence sll 63 bits */ \
 \
            /* we have set vx to mimic log(|x|), so we now compute y * log(|x|) */ \
            vz_special = __riscv_vfmul_mu(current_cases, vz_special, vy, vx, vlen); \
            /* map -Inf to +0 */ \
            vclass_z = __riscv_vfclass(vz_special, vlen); \
            IDENTIFY(vclass_z, class_negInf, id_mask, vlen); \
            id_mask = __riscv_vmand(id_mask, current_cases, vlen); \
            vz_special = __riscv_vfmerge(vz_special, fp_posZero, id_mask, vlen); \
            /* now must set the sign of vz_special for x in {Zero, Inf} and y of integer value */ \
 \
            IDENTIFY(vclass_x, class_Inf|class_Zero, id_mask, vlen); \
            id_mask = __riscv_vmand(current_cases, id_mask, vlen); \
            VFLOAT vz_tmp = I_AS_F(__riscv_vand(id_mask, sign_x, sign_z, vlen)); \
            vz_tmp = __riscv_vfsgnj(id_mask, vz_special, vz_tmp, vlen); \
            vz_special = __riscv_vmerge(vz_special, vz_tmp, id_mask, vlen); \
        } \
 \
        current_cases = __riscv_vmandn(y_special, x_special, vlen); \
        if (__riscv_vcpop(current_cases, vlen) > 0) { \
            /* y only is special */ \
 \
            /* Here x is finite and non-zero. But x == 1.0 is special \
             * in that 1.0^anything is 1.0, including when y is a NaN. \
             * Aside from this case, we need to differentiate |x| <, ==, > 1 \
             * so as to handle y == +-Inf appropriately. \
             */ \
 \
            /* If |x| == 1.0, replace y with 0.0 */ \
            VFLOAT vz_tmp = __riscv_vfsgnj(current_cases, vx, fp_posOne, vlen); \
            vz_tmp = __riscv_vfsub(current_cases, vz_tmp, fp_posOne, vlen); \
            id_mask = __riscv_vmfeq(vz_tmp, fp_posZero, vlen); \
            id_mask = __riscv_vmand(current_cases, id_mask, vlen); \
            VBOOL id_mask2; \
            IDENTIFY(vclass_y, class_Inf|class_Zero, id_mask2, vlen); \
            id_mask2 = __riscv_vmand(id_mask, id_mask2, vlen); \
            vy = __riscv_vfmerge(vy, fp_posZero, id_mask2, vlen); \
 \
            /* compute (|x|-1) * y yeilding the correct signed infinities */ \
            vz_tmp = __riscv_vfmul(current_cases, vz_tmp, vy, vlen); \
            /* except we need to set this to +0 if x == 1 (even if y is NaN) */ \
            id_mask = __riscv_vmfeq(vx, fp_posOne, vlen); \
            id_mask = __riscv_vmand(current_cases, id_mask, vlen); \
            vz_tmp = __riscv_vfmerge(vz_tmp, fp_posZero, id_mask, vlen); \
            vz_special = __riscv_vmerge(vz_special, vz_tmp, current_cases, vlen); \
 \
            /* map vz_special values of -Inf to 0 and 0 to 1.0 */ \
            vclass_z = __riscv_vfclass(vz_special, vlen); \
            IDENTIFY(vclass_z, class_negInf, id_mask, vlen); \
            id_mask = __riscv_vmand(current_cases, id_mask, vlen); \
            vz_special = __riscv_vfmerge(vz_special, fp_posZero, id_mask, vlen); \
            IDENTIFY(vclass_z, class_Zero, id_mask, vlen); \
            id_mask = __riscv_vmand(current_cases, id_mask, vlen); \
            vz_special = __riscv_vfmerge(vz_special, fp_posOne, id_mask, vlen); \
        } \
 \
        /* finally, substitue 1.0 for x and y when either x or y is special */ \
        vx = __riscv_vfmerge(vx, fp_posOne, special_args, vlen); \
        vy = __riscv_vfmerge(vy, fp_posOne, special_args, vlen); \
    } \
} while(0)

static const double two_to_neg63 = 0x1.0p-63;
static const uint64_t bias = 0x3ff0000000000000;
static const int64_t round_up = 0x0008000000000000;
static const uint64_t zero_mask_expo = 0x000fffffffffffff;
static const int64_t mask_T_hi = 0xffffffffff000000;
static const int64_t mask_T_lo = 0x0000000000ffffff;
static const double two_to_63 = 0x1.0p63;
static const double log2_inv = 0x1.71547652b82fep+0;
static const double log2_hi = 0x1.62e42fefa39efp-1;
static const double log2_lo = 0x1.abc9e3b39803fp-56;
static const double log2_inv_hi = 0x1.71547652b82fep+0;
static const double log2_inv_lo = 0x1.777d0ffda0d24p-56;
static const double two_to_65 = 0x1.0p65;
static const double negtwo_to_65 = -0x1.0p65;

#if (STRIDE == UNIT_STRIDE)
#define F_VER1    RVVLM_POWD_TBL
#else
#define F_VER1    RVVLM_POWDI_TBL
#endif

#include <fenv.h>

// Version 1 is reduction to standard primary interval.
// Reduced argument is represented as one FP64 variable.
void F_VER1(API) {
    size_t vlen;
    VFLOAT vx, vy, vz, vz_special;
    VBOOL special_args;

    SET_ROUNDTONEAREST;
    // stripmining over input arguments
    for (; _inarg_n > 0; _inarg_n -= vlen) {
        vlen = VSET(_inarg_n);
        vx = VFLOAD_INARG1(vlen);
        vy = VFLOAD_INARG2(vlen);

        // Set results when one of the inputs is NaN/Inf/Zero
        EXCEPTION_HANDLING_POW(vx, vy, special_args, vz_special, vlen);

        // Normal computations. Here, both x and y are finite and non-zero.
        // We compute 2^( y log_2(x) ) on the high level. But when x < 0,
        // we must handle the cases when y is of integer value, making x^y well defined.
        // So in essence, we try to compute 2^(y log_2(|x|)) and then figure out if 
        // one should replace this with NaN, or accept this numerical result with the 
        // possible flipping of its sign (x is negative and y is an odd integer).

        // Decompose in_arg into n, B, r
        VINT n_adjust, sign_x;
        VBOOL id_mask;
        n_adjust = __riscv_vxor(n_adjust, n_adjust, vlen);
        sign_x = __riscv_vxor(sign_x, sign_x, vlen);
        sign_x = F_AS_I(__riscv_vfsgnj(I_AS_F(sign_x), vx, vlen));
        vx = __riscv_vfsgnjx(vx, vx, vlen);
        IDENTIFY(vclass_x, class_Denorm, id_mask, vlen);
        vx = __riscv_vfmul_mu(id_mask, vx, vx, 0x1.0p65, vlen);
        n_adjust = __riscv_vmerge(n_adjust, 65, id_mask, vlen);

        VINT n = __riscv_vadd(F_AS_I(vx), round_up, vlen);
        n = __riscv_vsub(n, bias, vlen);
        n = __riscv_vsra(n, 52, vlen);
        n = __riscv_vsub(n, n_adjust, vlen);

        VFLOAT A = __riscv_vfcvt_f(n, vlen);

        // To get frec7(X) suffices to get frecp7 of X with its exponent field set to bias
        // The main reason for this step is that should the exponent of X be the largest
        // finite exponent, frec7(X) will be subnormal and carry less precision.
        // Moreover, we need to get the 7 mantissa bits of X for table lookup later on
        VUINT ind = __riscv_vand(F_AS_U(vx), zero_mask_expo, vlen);

        // normalize exponent of vx
        vx = U_AS_F(__riscv_vor(ind, bias, vlen));
        VFLOAT B = __riscv_vfrec7(vx, vlen);
        ind = __riscv_vsrl(ind, 45, vlen); // 7 leading mantissa bit 
        ind = __riscv_vsll(ind, 4, vlen);  // left shifted 4 (16-byte table)

        // adjust B to be 1.0 if ind == 0
        VBOOL adjust_B = __riscv_vmseq(ind, 0, vlen);
        B = __riscv_vfmerge(B, fp_posOne, adjust_B, vlen);
        VFLOAT r = VFMV_VF(fp_posOne, vlen);
        r = __riscv_vfmsac(r, vx, B, vlen);

        // with A = n in float format, r, and ind we can carry out floating-point computations
        // (A + T)  + log_e(1+r) * (1/log_e(2))
        // compute log_e(1+r) by a polynomial approximation. To obtian an accurate pow(x,y)
        // in the end, we must obtain at least 10 extra bits of precision over FP64.
        // So log_e(1+r) is approximated by a degree-9 polynomial
        // r - r^2/2 + r^3[ (p6 + r p5 + r^2 p4 ) + r^3 (p3 + r p2 + r^2 p1 + r^3 p0) ]
        // r - r^2/2 + poly; and furthermore, r - r^2/2 is computed as P + p,
        // 1/log(2) is stored as log2_inv_hi, log2_inv_lo, and T is broken into T_hi, T_lo
        // So, we need
        // (A + T_hi) + log2_inv_hi * P + 
        // log2_inv_hi * poly + T_lo  + log2_inv_lo*P
        // Note that log_2(|x|) needs be to represented in 2 FP64 variables as
        // we need to have log_2(|x|) in extra precision.
        //
        VFLOAT rcube = __riscv_vfmul(r, r, vlen);
        rcube = __riscv_vfmul(rcube, r, vlen);

        VFLOAT poly_right = PSTEP( -0x1.555555483d731p-3, r,
                            PSTEP(  0x1.2492453584b8ep-3, r,
                            PSTEP( -0x1.0005fa6ef2342p-3, 0x1.c7fe32d120e6bp-4, r,
                            vlen), vlen), vlen);

        VFLOAT poly_left = PSTEP( 0x1.5555555555555p-2, r,
                           PSTEP(-0x1.000000000003cp-2, 0x1.99999999a520ep-3, r,
                           vlen), vlen);

        VFLOAT poly = __riscv_vfmadd(poly_right, rcube, poly_left, vlen);
        // poly is (p6 + r p5 + r^2 p4 ) + r^3 (p3 + r p2 + r^2 p1 + r^3 p0)  

        VFLOAT r_prime = __riscv_vfmul(r, -0x1.0p-1, vlen); // exact product
        VFLOAT P = __riscv_vfmadd(r_prime, r, r, vlen);
        VFLOAT p = __riscv_vfsub(r, P, vlen);
        p = __riscv_vfmacc(p, r_prime, r, vlen);
        // P + p is r - r^2/2 to extra precision
        poly = __riscv_vfmadd(poly, rcube, p, vlen);
        // Now P + poly is log_e(1+r) to extra precision
        
        // Load table values and get n_flt + T to be A + a
        VFLOAT T_hi_flt = __riscv_vluxei64(logtbl_4_powD_128_hi_lo, ind, vlen);
        ind = __riscv_vadd(ind, 8, vlen);
        VFLOAT T_lo_flt = __riscv_vluxei64(logtbl_4_powD_128_hi_lo, ind, vlen);

        A = __riscv_vfadd(A, T_hi_flt, vlen);
        // (A + T_hi) + log2_inv_hi * P  + log2_inv_hi * poly + log2_inv_lo*P + T_lo
        // is log2(|x|) to extra precision
        VFLOAT log2x_hi = __riscv_vfmadd(P, log2_inv_hi, A, vlen);
        VFLOAT log2x_lo = __riscv_vfsub(A, log2x_hi, vlen);
        log2x_lo = __riscv_vfmacc(log2x_lo, log2_inv_hi, P, vlen);

        T_lo_flt = __riscv_vfmacc(T_lo_flt, log2_inv_lo, P, vlen);
        log2x_lo = __riscv_vfadd(log2x_lo, T_lo_flt, vlen);
        log2x_lo = __riscv_vfmacc(log2x_lo, log2_inv_hi, poly, vlen);
        VFLOAT log2x = __riscv_vfadd(log2x_hi, log2x_lo, vlen);
        T_lo_flt = __riscv_vfsub(log2x_hi, log2x, vlen);
        log2x_lo = __riscv_vfadd(T_lo_flt, log2x_lo, vlen);
        // log2x + log2x_lo is log2(|x|) to extra precision

        // The final stage involves computing 2^(y * log2x)
        VFLOAT vy_tmp = __riscv_vfmin(vy, 0x1.0p53, vlen);
        vy_tmp = __riscv_vfmax(vy_tmp, -0x1.0p53, vlen);
        VINT y_to_int = __riscv_vfcvt_x(vy_tmp, vlen);
        VFLOAT vy_rnd_int = __riscv_vfcvt_f(y_to_int, vlen);
        VBOOL y_is_int = __riscv_vmfeq(vy_tmp, vy_rnd_int, vlen);
        y_to_int = __riscv_vsll(y_to_int, 63, vlen);
        // if y is of integer value, y_to_int is the parity of y in the sign bit position
        // To compute y * (log2x + log2x_lo)
        // we first clip y to +-2^65
        vy = __riscv_vfmin(vy, two_to_65 , vlen);
        vy = __riscv_vfmax(vy, negtwo_to_65, vlen);
        vy_tmp = __riscv_vfmul(vy, log2x, vlen);
        r = __riscv_vfmsub(vy, log2x, vy_tmp, vlen);
        r = __riscv_vfmacc(r, vy, log2x_lo, vlen);
        // vy_tmp + r is the product, clip at +-1100
        vy_tmp = __riscv_vfmin(vy_tmp, 0x1.13p10, vlen);
        vy_tmp = __riscv_vfmax(vy_tmp, -0x1.13p10, vlen);
        r = __riscv_vfmin(r, 0x1.0p-35, vlen);
        r = __riscv_vfmax(r, -0x1.0p-35, vlen);

        // Argument reduction
        VFLOAT n_flt = __riscv_vfmul(vy_tmp, 0x1.0p6, vlen);
        n = __riscv_vfcvt_x(n_flt, vlen);
        n_flt = __riscv_vfcvt_f(n, vlen);

        vy_tmp = __riscv_vfnmsac(vy_tmp, 0x1.0p-6, n_flt, vlen);
        r = __riscv_vfadd(vy_tmp, r, vlen);
        r = __riscv_vfmul(r, log2_hi, vlen);

        // Polynomial computation, we have a degree 5
        // We break this up into 2 pieces
        // Ideally the compiler will interleave the computations of the segments
        poly_right = PSTEP( 0x1.5555722e87735p-5, 0x1.1107f5fc29bb7p-7, r, vlen);
        poly_left = PSTEP( 0x1.fffffffffe1f5p-2, 0x1.55555556582a8p-3, r, vlen);

        VFLOAT r_sq = __riscv_vfmul(r, r, vlen);
        poly = __riscv_vfmadd(poly_right, r_sq, poly_left, vlen);

        poly = __riscv_vfmadd(poly, r_sq, r, vlen);
        poly = __riscv_vfmul(poly, two_to_63, vlen);
        VINT P_fixedpt = __riscv_vfcvt_x(poly, vlen);

        VINT j = __riscv_vand(n, 0x3f, vlen);
        j = __riscv_vsll(j, 3, vlen);
        VINT T = __riscv_vluxei64(expD_tbl64_fixedpt, I_AS_U(j), vlen);

        P_fixedpt = __riscv_vsmul(P_fixedpt, T, 1, vlen);
        P_fixedpt = __riscv_vsadd(P_fixedpt, T, vlen);
        vz = __riscv_vfcvt_f(P_fixedpt, vlen);
        // at this point, vz ~=~ 2^62 * exp(r)

        n = __riscv_vsra(n, 6, vlen);
        // Need to compute 2^(n-62) * exp(r). 
        // Although most of the time, it suffices to add n to the exponent field of exp(r)
        // this will fail n is just a bit too positive or negative, corresponding to
        // 2^n * exp(r) causing over or underflow.
        // So we have to decompose n into n1 + n2  where n1 = n >> 1
        // 2^n1 * exp(r) can be performed by adding n to exp(r)'s exponent field
        // But we need to create the floating point value scale = 2^n2 
        // and perform a multiplication to finish the task.

        n = __riscv_vsub(n, 62, vlen);
        FAST_LDEXP(vz, n, vlen);

        VBOOL invalid = __riscv_vmsne(sign_x, 0, vlen);
        invalid = __riscv_vmandn(invalid, y_is_int, vlen);
        vz = __riscv_vfmerge(vz, fp_sNaN, invalid, vlen);
        vz = __riscv_vfadd(vz, fp_posZero, vlen);
        
        sign_x = __riscv_vand(sign_x, y_to_int, vlen);
        vz = __riscv_vfsgnj_mu(y_is_int, vz, vz, I_AS_F(sign_x), vlen);
        
        vz = __riscv_vmerge(vz, vz_special, special_args, vlen);

        VFSTORE_OUTARG1(vz, vlen);

        //VSE(z, vz, vlen);
        //x += vlen; y += vlen, z+= vlen;
        INCREMENT_INARG1(vlen);
        INCREMENT_INARG2(vlen);
        INCREMENT_OUTARG1(vlen);
    }
    RESTORE_FRM;
}

