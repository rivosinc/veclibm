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

void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), double, double, int);

void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), const double *, int);

void report_err_fp64(
  void (*test_func)(size_t, const double *, size_t, double *, size_t), 
  long double (*ref_func)(long double),
  double,
  double,
  int,
  int,
  int
);

void report_err_powD_fp64(void (*test_func)(size_t, const double *,
                                            const double *, double *),
                          long double (*ref_func)(long double, long double),
                          double, double, double, int);

void report_err_fp80(void (*test_func)(size_t, const double *, const double *,
                                       double *, double *),
                     long double (*ref_func)(long double), double, double, int);

void show_special_fp64(void (*test_func)(size_t, const double *, double *),
                       const char *);

void show_special2_fp64(void (*test_func)(size_t, const double *,
                                          const double *, double *),
                        char *);

void trig_2pi_reduction(long double, long double *, int64_t *);

long double acospil(long double);
long double asinpil(long double);
long double atanpil(long double);

long double cospil(long double);
long double sinpil(long double);
long double tanpil(long double);
