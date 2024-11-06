// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdio.h>

#define DBL_NEG_INF (int64_t)0xFFF0000000000000
#define DBL_NEG_GT1 (int64_t)0xBFF0000000000001
#define DBL_NEG_ONE (int64_t)0xBFF0000000000000
#define DBL_NEG_LT1 (int64_t)0xBFEFFFFFFFFFFFFF
#define DBL_NEG_TINY (int64_t)0x800FFFFFFFFFFFFF
#define DBL_NEG_ZERO (int64_t)0x8000000000000000
#define DBL_POS_ZERO (int64_t)0x0000000000000000
#define DBL_POS_TINY (int64_t)0x000FFFFFFFFFFFFF
#define DBL_POS_LT1 (int64_t)0x3FEFFFFFFFFFFFFF
#define DBL_POS_ONE (int64_t)0x3FF0000000000000
#define DBL_POS_GT1 (int64_t)0x3FF0000000000001
#define DBL_POS_INF (int64_t)0x7FF0000000000000
#define DBL_QNAN (int64_t)0x7FF8000000000000
#define DBL_SNAN (int64_t)0x7FF4000000000000

#define NB_TV_FOR_TRIG 4
#define TV_FOR_TRIG                                                            \
  {DBL_QNAN,    DBL_QNAN, DBL_SNAN,    DBL_QNAN,                               \
   DBL_NEG_INF, DBL_QNAN, DBL_POS_INF, DBL_QNAN}

#define NB_TV_FOR_EXP 4
#define TV_FOR_EXP                                                             \
  {DBL_QNAN,    DBL_QNAN,     DBL_SNAN,    DBL_QNAN,                           \
   DBL_NEG_INF, DBL_POS_ZERO, DBL_POS_INF, DBL_POS_INF}

#define NB_TV_FOR_EXPM1 5
#define TV_FOR_EXPM1                                                           \
  {DBL_QNAN,    DBL_QNAN,    DBL_SNAN,    DBL_QNAN,     DBL_NEG_INF,           \
   DBL_NEG_ONE, DBL_POS_INF, DBL_POS_INF, DBL_POS_ZERO, DBL_POS_ZERO}

#define NB_TV_FOR_LOG 8
#define TV_FOR_LOG                                                             \
  {DBL_QNAN,     DBL_QNAN,    DBL_SNAN,     DBL_QNAN,                          \
   DBL_NEG_INF,  DBL_QNAN,    DBL_NEG_GT1,  DBL_QNAN,                          \
   DBL_NEG_TINY, DBL_QNAN,    DBL_NEG_ZERO, DBL_NEG_INF,                       \
   DBL_POS_ZERO, DBL_NEG_INF, DBL_POS_INF,  DBL_POS_INF}

#define NB_TV_FOR_LOG1P 7
#define TV_FOR_LOG1P                                                           \
  {DBL_QNAN,     DBL_QNAN,     DBL_SNAN,    DBL_QNAN,    DBL_NEG_INF,          \
   DBL_QNAN,     DBL_NEG_GT1,  DBL_QNAN,    DBL_NEG_ONE, DBL_NEG_INF,          \
   DBL_POS_ZERO, DBL_POS_ZERO, DBL_POS_INF, DBL_POS_INF}

#define COMMENT(comment)                                                       \
  {                                                                            \
    printf("\n=====\t" comment "\n");                                          \
  }

// Most common interface: testing on 1 interval
// for 1-in-1-out unit-stride function
void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), double, double, int,
                     double = 1.0);

void report_err_fp64(void (*test_func)(size_t, const double *, double *,
                                       double *),
                     long double (*ref_func)(long double), int, double, double,
                     int, double = 1.0);

void report_err_fp64(void (*test_func)(size_t, const double *, double *,
                                       double *),
                     long double (*ref_func)(long double), int, const double *,
                     int, double = 1.0);

void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), const double *, int,
                     double = 1.0);

void report_mixederr_fp64(void (*test_func)(size_t, const double *, double *),
                          long double (*ref_func)(long double), double, double,
                          int, double = 1.0);

// Second most common interface: testing on 1 interval
// for 1-in-1-out general-stride function
void report_err_fp64(void (*test_func)(size_t, const double *, size_t, double *,
                                       size_t),
                     long double (*ref_func)(long double), double, double, int,
                     int, int, double = 1.0);

void report_err_fp64(void (*test_func)(size_t, const double *, size_t, double *,
                                       size_t, double *, size_t),
                     long double (*ref_func)(long double), int, double, double,
                     int, int, int, int);

void report_err2_fp64(void (*test_func)(size_t, const double *, const double *,
                                        double *),
                      long double (*ref_func)(long double, long double), double,
                      double, int, double, double, int, bool, double = 1.0);

void report_err2_fp64(void (*test_func)(size_t, const double *, size_t,
                                        const double *, size_t, double *,
                                        size_t),
                      long double (*ref_func)(long double, long double), double,
                      double, int, int, double, double, int, int, int, bool,
                      double = 1.0);

// Testing a function where we only have a reference version for its inverse
// Testing on 1 interval for 1-in-1-out and unit-stride
void report_err_byinv_fp64(void (*test_func)(size_t, const double *, double *),
                           long double (*ref_inv_func)(long double),
                           long double (*ref_inv_func_prime)(long double),
                           double, double, int, double = 1.0);

// Testing a function where we only have a reference version for its inverse
// Testing on 1 interval for 1-in-1-out and general-stride
void report_err_byinv_fp64(void (*test_func)(size_t, const double *, size_t,
                                             double *, size_t),
                           long double (*ref_inv_func)(long double),
                           long double (*ref_inv_func_prime)(long double),
                           double, double, int, int, int, double = 1.0);

void report_err_pow_fp64(void (*test_func)(size_t, const double *,
                                           const double *, double *),
                         long double (*ref_func)(long double, long double),
                         double, double, double, int, double = 1.0);

void report_err_fp80(void (*test_func)(size_t, const double *, const double *,
                                       double *, double *),
                     long double (*ref_func)(long double), double, double, int);

void show_special_fp64(void (*test_func)(size_t, const double *, double *),
                       const char *);

void show_special12_fp64(void (*test_func)(size_t, const double *, double *,
                                           double *),
                         char *);

void show_special2_fp64(void (*test_func)(size_t, const double *,
                                          const double *, double *),
                        int, char *);

void test_vectors_fp64(void (*test_func)(size_t, const double *, double *),
                       int64_t *, int);

void test_vectors2_fp64(void (*test_func)(size_t, const double *,
                                          const double *, double *),
                        int64_t *, int, int);

void trig_2pi_reduction(long double, long double *, int64_t *);

long double acospil(long double);
long double asinpil(long double);
long double atanpil(long double);
long double atan2pil(long double, long double);

long double cospil(long double);
long double sinpil(long double);
long double tanpil(long double);

long double cdfnorml(long double);

long double exp_neg_rsq(long double);
long double x_transform(long double);
long double recip_scale(long double);
long double erfl_prime(long double);
long double erfcl_prime(long double);
long double cdfnorml_prime(long double);
long double log_4_stirling(long double);
long double stirling_power(long double);
long double stirling_correction(long double);
long double tgammal_mod(long double);
long double sinpix_by_pi(long double);
long double lgammap1l(long double);
long double log_stirling(long double);
long double log_stirling_correction(long double);
long double expint1_01(long double);
long double expint1_01_plus_logx(long double);
long double neg_logl(long double);
long double expint1_trans_rat(long double);
long double exp_negx(long double);
long double expint1(long double);
