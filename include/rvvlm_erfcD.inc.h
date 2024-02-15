// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#if defined(COMPILE_FOR_ERFC)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_ERFCD_STD
#else
#define F_VER1 RVVLM_ERFCDI_STD
#endif
#elif defined(COMPILE_FOR_CDFNORM)
#if (STRIDE == UNIT_STRIDE)
#define F_VER1 RVVLM_CDFNORMD_STD
#else
#define F_VER1 RVVLM_CDFNORMDI_STD
#endif
#else
static_assert(false,
              "Must compile for ERFC or CDFNORM when including " __FILE__);
#endif

#if defined(COMPILE_FOR_ERFC)
// polynomial coefficients Q62
#define P_0 0x4f33682d757709e8
#define P_1 -0x95970864bc25c71
#define P_2 0xa377a56796fd6f
#define P_3 0x5ea2d221c412d2d
#define P_4 -0x8f0caa24847e2a3
#define P_5 0x8ac6781d49af506
#define P_6 -0x67476ebb9bc1f58
#define P_7 0x3d0ed00f93b86cb
#define P_8 -0x1c36fb9d9556ac0
#define P_9 0x96c3f45eaad23b
#define P_10 -0x1a6d434ab9ada1
#define P_11 -0x4dd9356c9c3f8
#define P_12 0x4bb31b11d0a1a
#define P_13 -0xf2d325083d5b
#define P_14 -0x52720383749f
#define P_15 0x33f7f3f6cb7d
#define P_16 0x4ed13a394f
#define P_17 -0x770e9d9af50
#define P_18 0x108f3f3cf59
#define P_19 0x101b7f3c485
#define P_20 -0x3ab6fb75ad
#define P_21 -0x237088721c
#define P_22 0x6ed93407e
#define P_23 0x3dbfb2c72
#define NEG_A_SCALED -0x1.ep+64
#define B_FOR_TRANS 0x1.04p+2
#define MIN_CLIP 0x1.0p-60
#define MAX_CLIP 0x1.cp4
#else
// polynomial coefficients in Q63
#define P_0 0x6c25c9f6cfd132e7
#define P_1 -0x5abb8f458c7895f
#define P_2 -0x5ea2dcf3956792c
#define P_3 0xdd22963d83fa7d8
#define P_4 -0x107d667db8b90c84
#define P_5 0xea0acc44786d840
#define P_6 -0xa5e5b52ef29e23a
#define P_7 0x5ef73d5784d9dc6
#define P_8 -0x2acb1deb9208ae5
#define P_9 0xdf0d75186479cf
#define P_10 -0x25493132730985
#define P_11 -0x7daed4327549c
#define P_12 0x6ff2fb205b4f9
#define P_13 -0x15242feefcc0f
#define P_14 -0x7f14d7432d2b
#define P_15 0x4b2791427dab
#define P_16 0x17d0499cfa7
#define P_17 -0xae9fb960b85
#define P_18 0x15d4aa6975c
#define P_19 0x17cff734612
#define P_20 -0x505ad971f3
#define P_21 -0x34366c3ea9
#define P_22 0x97dfa0691
#define P_23 0x591d3b55a
#define NEG_A_SCALED -0x1.536p+65
#define B_FOR_TRANS 0x1.6fap+2
#define MIN_CLIP 0x1.0p-60
#define MAX_CLIP 0x1.4p5
#endif

// When COMPILE_FOR_ERFC
// The main computation is for erfc(|x|) and exploits the symmetry
// erfc(-|x|) = 2 - erfc(|x|)
// When COMPILE_FOR_CDFNORM
// The main compputation is for cdfnorm(-|x|) and exploits the symmetry
// cdfnorm(|x|) = 1 - cdfnorm(-|x|)
void F_VER1(API) {
  size_t vlen;
  VFLOAT vx, vx_orig, vy, vy_special;
  VBOOL special_args;

  SET_ROUNDTONEAREST;
  // stripmining over input arguments
  for (; _inarg_n > 0; _inarg_n -= vlen) {
    vlen = VSET(_inarg_n);
    vx = VFLOAD_INARG1(vlen);
    vx_orig = vx;

    // Handle Inf and NaN
    EXCEPTION_HANDLING(vx, special_args, vy_special, vlen);

    // suffices to focus on |x| clipped to [2^-60, 28]
    vx = __riscv_vfsgnj(vx, fp_posOne, vlen);
    vx = __riscv_vfmin(vx, MAX_CLIP, vlen);
    vx = __riscv_vfmax(vx, MIN_CLIP, vlen);

    VINT R;
    // Compute (x-a)/(x+b) as Q63 fixed-point
    X_TRANSFORM(vx, NEG_A_SCALED, B_FOR_TRANS, R, vlen);

    VINT n, A;
#if defined(COMPILE_FOR_ERFC)
    VFLOAT vy = vx;
#else
    VFLOAT vy = __riscv_vfmul(vx, 0x1.0p-1, vlen);
#endif
    // Compute exp(-x*x) or exp(-x*x/2) as 2^n a
    // but return a as Q62 fixed-point A
    EXP_negAB(vx, vy, n, A, vlen);

    // Approximate exp(x*x)*(1+2x)*erfc(x)
    // or exp(x*x/2)*(1+2x)*cdfnorm(-x)
    // using a polynomial in r = (x-a)/(x+b)
    // We use fixed-point computing
    // -1 < r < 1, thus using Q63 fixed-point for r
    // All coefficients are scaled the same and thus
    // the final value is in this scaling.
    // Scale is 2^62 for erfc and 2^63 for cdfnorm
    VINT P_RIGHT = PSTEP_I(
        P_16, R,
        PSTEP_I(
            P_17, R,
            PSTEP_I(P_18, R,
                    PSTEP_I(P_19, R,
                            PSTEP_I(P_20, R,
                                    PSTEP_I(P_21, R,
                                            PSTEP_I(P_22, P_23, R, vlen), vlen),
                                    vlen),
                            vlen),
                    vlen),
            vlen),
        vlen);

    VINT R8 = __riscv_vsmul(R, R, 1, vlen);
    R8 = __riscv_vsmul(R8, R8, 1, vlen);
    R8 = __riscv_vsmul(R8, R8, 1, vlen);

    VINT P_MID = PSTEP_I(
        P_8, R,
        PSTEP_I(
            P_9, R,
            PSTEP_I(P_10, R,
                    PSTEP_I(P_11, R,
                            PSTEP_I(P_12, R,
                                    PSTEP_I(P_13, R,
                                            PSTEP_I(P_14, P_15, R, vlen), vlen),
                                    vlen),
                            vlen),
                    vlen),
            vlen),
        vlen);

    P_RIGHT = __riscv_vsmul(R8, P_RIGHT, 1, vlen);
    P_RIGHT = __riscv_vadd(P_RIGHT, P_MID, vlen);
    P_RIGHT = __riscv_vsmul(R8, P_RIGHT, 1, vlen);

    VINT P_LEFT = PSTEP_I(
        P_0, R,
        PSTEP_I(
            P_1, R,
            PSTEP_I(P_2, R,
                    PSTEP_I(P_3, R,
                            PSTEP_I(P_4, R,
                                    PSTEP_I(P_5, R, PSTEP_I(P_6, P_7, R, vlen),
                                            vlen),
                                    vlen),
                            vlen),
                    vlen),
            vlen),
        vlen);

    VINT P = __riscv_vadd(P_LEFT, P_RIGHT, vlen);

    VINT m, B;
    RECIP_SCALE(vx, B, m, vlen);

#if defined(COMPILE_FOR_ERFC)
    // exp(-x^2) is 2^n * 2^(-62) * A
    // 1/(1+2x)  is 2^(-m) * B, m >= 62
    // exp(x^2)(1+2x)erfc(x) is 2^(-62) * P
    P = __riscv_vsmul(P, A, 1, vlen); // Q61
    P = __riscv_vsmul(P, B, 1, vlen); // Q(m-2)
    n = __riscv_vsub(n, m, vlen);
    n = __riscv_vadd(n, 2, vlen); // n <= -60

    VUINT ell = I_AS_U(__riscv_vrsub(n, -60, vlen));
    ell = __riscv_vminu(ell, 63, vlen);
    VINT PP = __riscv_vsra(P, ell, vlen);
    VINT Q = VMVI_VX(1, vlen);
    Q = __riscv_vsll(Q, 61, vlen);
    Q = __riscv_vsub(Q, PP, vlen);
    VFLOAT vz = __riscv_vfcvt_f(Q, vlen);
    vz = __riscv_vfmul(vz, 0x1.0p-60, vlen);

    vy = __riscv_vfcvt_f(P, vlen);
    FAST_LDEXP(vy, n, vlen);
    // vy is erfc(|x|) at this point

    VBOOL x_is_neg = __riscv_vmflt(vx_orig, fp_posZero, vlen);
    vy = __riscv_vmerge(vy, vz, x_is_neg, vlen);
#else
    // exp(-x^2/2) is 2^n * 2^(-62) * A
    // 1/(1+2x)  is 2^(-m) * B, m >= 62
    // exp(x^2/2)(1+2x)cdfnorm(-x) is 2^(-63) * P
    P = __riscv_vsmul(P, A, 1, vlen); // Q62
    P = __riscv_vsmul(P, B, 1, vlen); // Q(m-1)
    n = __riscv_vsub(n, m, vlen);
    n = __riscv_vadd(n, 1, vlen); // n <= -61

    VUINT ell = I_AS_U(__riscv_vrsub(n, -61, vlen));
    ell = __riscv_vminu(ell, 63, vlen);
    VINT PP = __riscv_vsra(P, ell, vlen);
    VINT Q = VMVI_VX(1, vlen);
    Q = __riscv_vsll(Q, 61, vlen);
    Q = __riscv_vsub(Q, PP, vlen);
    VFLOAT vz = __riscv_vfcvt_f(Q, vlen);
    vz = __riscv_vfmul(vz, 0x1.0p-61, vlen);

    vy = __riscv_vfcvt_f(P, vlen);
    FAST_LDEXP(vy, n, vlen);
    // vy is cdfnorm(-|x|) at this point

    VBOOL x_is_pos = __riscv_vmfgt(vx_orig, fp_posZero, vlen);
    vy = __riscv_vmerge(vy, vz, x_is_pos, vlen);
#endif

    vy = __riscv_vmerge(vy, vy_special, special_args, vlen);
    // copy vy into y and increment addr pointers
    VFSTORE_OUTARG1(vy, vlen);

    INCREMENT_INARG1(vlen);
    INCREMENT_OUTARG1(vlen);
  }
  RESTORE_FRM;
}
