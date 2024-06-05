// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdio.h>

#define COMMENT(comment)                                                       \
  { printf("\n=====\t" comment "\n"); }

// Most common interface: testing on 1 interval
// for 1-in-1-out unit-stride function
void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), double, double, int,
                     double = 1.0);

void report_err_fp64(void (*test_func)(size_t, const double *, double *,
                                       double *),
                     long double (*ref_func)(long double), int, double, double,
                     int, double = 1.0);

void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), const double *, int);

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
