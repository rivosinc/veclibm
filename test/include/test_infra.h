// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), double, double, int);

void report_err_fp64(void (*test_func)(size_t, const double *, double *,
                                       double *),
                     long double (*ref_func)(long double), int, double, double,
                     int);

void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), const double *, int);

void report_err_fp64(void (*test_func)(size_t, const double *, size_t, double *,
                                       size_t),
                     long double (*ref_func)(long double), double, double, int,
                     int, int);

void report_err_fp64(void (*test_func)(size_t, const double *, size_t, double *,
                                       size_t, double *, size_t),
                     long double (*ref_func)(long double), int, double, double,
                     int, int, int, int);

void report_err2_fp64(void (*test_func)(size_t, const double *, const double *,
                                        double *),
                      long double (*ref_func)(long double, long double), double,
                      double, int, double, double, int, bool);

void report_err2_fp64(void (*test_func)(size_t, const double *, size_t,
                                        const double *, size_t, double *,
                                        size_t),
                      long double (*ref_func)(long double, long double), double,
                      double, int, int, double, double, int, int, int, bool);

void report_err_pow_fp64(void (*test_func)(size_t, const double *,
                                           const double *, double *),
                         long double (*ref_func)(long double, long double),
                         double, double, double, int);

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
