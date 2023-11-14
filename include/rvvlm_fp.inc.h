#ifndef LMUL
static_assert(false, "Must assign an LMUL before including " __FILE__);
#endif
#ifndef BIT_WIDTH
static_assert(false, "Must assign BIT_WIDTH before including " __FILE__);
#endif

#define __PASTE2_BASE(A,B) A##B
#define __PASTE2(A,B) __PASTE2_BASE(A,B)
#define __PASTE3_BASE(A,B,C) A##B##C
#define __PASTE3(A,B,C) __PASTE3_BASE(A,B,C)
#define __PASTE4_BASE(A,B,C,D) A##B##C##D
#define __PASTE4(A,B,C,D) __PASTE4_BASE(A,B,C,D)
#define __PASTE5_BASE(A,B,C,D,E) A##B##C##D##E
#define __PASTE5(A,B,C,D,E) __PASTE5_BASE(A,B,C,D,E)

#define MAKE_VTYPE(A) __PASTE3(A,BIT_WIDTH,__PASTE3(m,LMUL,_t))
#define MAKE_TYPE(A) __PASTE3(A,BIT_WIDTH,_t)
#define MAKE_FUNC(A) __PASTE3(A,BIT_WIDTH,__PASTE2(m,LMUL))
#define MAKE_VLOAD(A) __PASTE3(__PASTE3(__riscv_vle,BIT_WIDTH,_v_),A,__PASTE3(BIT_WIDTH,m,LMUL))
// For MAKE_VBOOL, the value is 64/LMUL
#if (LMUL == 1)
#define MAKE_VBOOL(A) __PASTE3(A,64,_t)
#elif (LMUL == 2)
#define MAKE_VBOOL(A) __PASTE3(A,32,_t)
#elif (LMUL == 4)
#define MAKE_VBOOL(A) __PASTE3(A,16,_t)
#elif (LMUL == 8)
#define MAKE_VBOOL(A) __PASTE3(A,8,_t)
#endif
#define VSET __PASTE2(__riscv_vsetvl_e,__PASTE3(BIT_WIDTH,m,LMUL))
#define VSE __PASTE2(__riscv_vse,BIT_WIDTH)
#define MAKE_REINTERPRET(A,B) __PASTE5(__riscv_vreinterpret_v_,A,__PASTE4(BIT_WIDTH,m,LMUL,_),B,__PASTE3(BIT_WIDTH,m,LMUL))

#define FLOAT MAKE_TYPE(float)
#define VFLOAT MAKE_VTYPE(vfloat)
#define INT MAKE_TYPE(int)
#define VINT MAKE_VTYPE(vint)
#define UINT MAKE_TYPE(uint)
#define VUINT MAKE_VTYPE(vuint)
#define VBOOL MAKE_VBOOL(vbool)

#define F_AS_I MAKE_REINTERPRET(f,i)
#define F_AS_U MAKE_REINTERPRET(f,u)
#define I_AS_F MAKE_REINTERPRET(i,f)
#define U_AS_F MAKE_REINTERPRET(u,f)
#define I_AS_U MAKE_REINTERPRET(i,u)
#define U_AS_I MAKE_REINTERPRET(u,i)

#define VFLOAD MAKE_VLOAD(f)
#define VILOAD MAKE_VLOAD(i)
#define VULOAD MAKE_VLOAD(u)
#define VMVI_VX MAKE_FUNC(__riscv_vmv_v_x_i)
#define VMVU_VX MAKE_FUNC(__riscv_vmv_v_x_u)
#define VFMV_VF MAKE_FUNC(__riscv_vfmv_v_f_f)


const INT int_Zero = 0;
const UINT uint_Zero = 0;

#if (BIT_WIDTH == 64) 
#define EXP_BIAS 1023
#define MAN_LEN 52
static const uint64_t class_sNaN = 0x100;
static const uint64_t class_qNaN = 0x200;
static const uint64_t class_NaN = 0x300;
static const uint64_t class_negInf = 0x1;
static const uint64_t class_posInf = 0x80;
static const uint64_t class_Inf = 0x81;
static const uint64_t class_negZero = 0x8;
static const uint64_t class_posZero = 0x10;
static const uint64_t class_Zero = 0x18;
static const uint64_t class_negDenorm = 0x4;
static const uint64_t class_posDenorm = 0x20;
static const uint64_t class_Denorm = 0x24;
static const uint64_t class_negNormal = 0x2;
static const uint64_t class_posNormal = 0x40;
static const uint64_t class_Normal = 0x42;
static const uint64_t class_negative = 0x7;
static const uint64_t class_positive = 0xe0;

static const union sui64_fp64 SNAN = {(int64_t) 0x7ff4000000000000};
static const union sui64_fp64 QNAN = {(int64_t) 0x7ff8000000000000};
static const union sui64_fp64 PINF = {(int64_t) 0x7ff0000000000000};
static const union sui64_fp64 NINF = {(int64_t) 0xfff0000000000000};
static const union sui64_fp64 NEGZERO = {(int64_t) 0x8000000000000000};

static const double fp_sNaN = SNAN.f;
static const double fp_qNaN = QNAN.f;
static const double fp_posInf = PINF.f;
static const double fp_negInf = NINF.f;
static const double fp_negZero = NEGZERO.f;
static const double fp_posZero = 0.0;
static const double fp_posOne = 0x1.0p0;
static const double fp_negOne = -0x1.0p0;
static const double fp_posHalf = 0x1.0p-1;
static const double fp_negHalf = -0x1.0p-1;
#endif


