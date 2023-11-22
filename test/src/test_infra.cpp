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

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

union sui32_fp32 {
  int32_t si;
  uint32_t ui;
  float f;
};
union sui64_fp64 {
  int64_t si;
  uint64_t ui;
  double f;
  uint32_t ui_hilo[2];
};

#define ui_hilo_HI 1
#define ui_hilo_LO 0
// so that union sui64_f64 X will have X.hilo[HI] as the high bits (containing
// expoent) and X.hilo[LO] has the lower order bits (containing the lsb for
// example)

#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define N_PTS_MAX 100000

#define VERBOSE 0

double ulp_32(double ref_value) {
  union sui64_fp64 ulp;
  int64_t expo;

  ulp.f = ABS(ref_value);
  expo = (ulp.si >> 52);
  expo = expo - 23;
  expo = MAX(expo, 1023 - 149);
  ulp.si = (expo << 52);
  return ulp.f;
}

double ulp_64(double ref_value) {
  union sui64_fp64 ulp;
  int64_t expo;

  ulp.f = ABS(ref_value);
  expo = (ulp.si >> 52);
  expo = expo - 52;
  if (expo >= 1) {
    ulp.si = (expo << 52);
  } else {
    expo = -expo;
    expo = MIN(expo, 51);
    expo = 0x8000000000000 >> expo;
    ulp.si = expo;
  }
  return ulp.f;
}

#define N_SPECIALS 17
union sui64_fp64 fp64_special_values[N_SPECIALS] = {
    (int64_t)0xfff0000000000000, (int64_t)0xffefffffffffffff,
    (int64_t)0xbff0000000000000, (int64_t)0x8010000000000000,
    (int64_t)0x800fffffffffffff, (int64_t)0x8000000000000001,
    (int64_t)0x8000000000000000, (int64_t)0x0000000000000000,
    (int64_t)0x0000000000000001, (int64_t)0x000fffffffffffff,
    (int64_t)0x0010000000000000, (int64_t)0x3ff0000000000000,
    (int64_t)0xbff0000000000000, (int64_t)0x7fefffffffffffff,
    (int64_t)0x7ff0000000000000, (int64_t)0x7ff4000000000000,
    (int64_t)0x7ff8000000000000};

void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double),
                     const double *test_args, int nb_test_args) {

  long double y_ref;
  double *x, *y, delta;
  double abs_err, rel_err, ulp_err;
  double max_abs_err, max_rel_err, max_ulp_err;

  y = (double *)malloc(nb_test_args * sizeof(double));

  max_abs_err = 0.0;
  max_rel_err = 0.0;
  max_ulp_err = 0.0;

  // call function under test
  test_func((size_t)nb_test_args, test_args, y);

  // now for each point we compute error and log the max
  for (int j = 0; j < nb_test_args; j++) {
    y_ref = ref_func((long double)test_args[j]);
    abs_err = (double)((long double)y[j] - y_ref);
    abs_err = ABS(abs_err);
    if (ABS((double)y_ref) > 0.0) {
      rel_err = abs_err / ABS((double)y_ref);
    } else {
      rel_err = abs_err / 0x1.0p-1074;
    }
    ulp_err = abs_err / ulp_64((double)y_ref);

    max_abs_err = MAX(max_abs_err, abs_err);
    max_rel_err = MAX(max_rel_err, rel_err);
    max_ulp_err = MAX(max_ulp_err, ulp_err);

    if (VERBOSE) {
      union sui64_fp64 xxx, yyy;
      xxx.f = test_args[j];
      yyy.f = y[j];
      printf("--input %24.17le, 0x%016lx, output %24.17le, 0x%016lx \n", xxx.f,
             xxx.ui, yyy.f, yyy.ui);
      printf("  reference %24.17le\n\n", (double)y_ref);
    }
  }
  printf("----------------------------\n");
  printf("Tested %d special test arguments\n", nb_test_args);
  printf("Maximum observed absolute error is %8.3le\n", max_abs_err);
  printf("Maximum observed relative error is %8.3le\n", max_rel_err);
  if (max_rel_err > 0.0) {
    printf("                          which is 2^(%3.3lf)\n",
           log2(max_rel_err));
  }
  printf("Maximum observed ULP      error is %3.3lf\n", max_ulp_err);

  free(y);

  return;
}

void report_err_fp64(void (*test_func)(size_t, const double *, double *),
                     long double (*ref_func)(long double), double start,
                     double end, int nb_pts) {

  long double y_ref;
  double *x, *y, delta;
  long double abs_err, rel_err, ulp_err;
  long double max_abs_err, max_rel_err, max_ulp_err;

  x = (double *)malloc(nb_pts * sizeof(double));
  y = (double *)malloc(nb_pts * sizeof(double));

  if (nb_pts <= 1) {
    delta = 0.0;
    nb_pts = 1;
  } else {
    delta = (end - start) / (double)(nb_pts - 1);
  }

  max_abs_err = 0.0;
  max_rel_err = 0.0;
  max_ulp_err = 0.0;

  // fill up the set of test points
  for (int i = 0; i < nb_pts; ++i) {
    x[i] = start + (double)i * delta;
  }

  // call function under test
  test_func((size_t)nb_pts, x, y);

  // now for each point we compute error and log the max
  for (int j = 0; j < nb_pts; j++) {
    y_ref = ref_func((long double)x[j]);
    abs_err = (long double)y[j] - y_ref;
    abs_err = ABS(abs_err);
    if (ABS((double)y_ref) > 0.0) {
      rel_err = abs_err / ABS((double)y_ref);
    } else {
      rel_err = abs_err / 0x1.0p-1074;
    }
    ulp_err = abs_err / ulp_64((double)y_ref);

    max_abs_err = MAX(max_abs_err, abs_err);
    max_rel_err = MAX(max_rel_err, rel_err);
    max_ulp_err = MAX(max_ulp_err, ulp_err);

    if (VERBOSE) {
      union sui64_fp64 xxx, yyy;
      xxx.f = x[j];
      yyy.f = y[j];
      printf("--input %24.17le, 0x%016lx, output %24.17le, 0x%016lx \n", xxx.f,
             xxx.ui, yyy.f, yyy.ui);
      printf("  reference %24.17le\n\n", (double)y_ref);
    }
  }
  printf("----------------------------\n");
  if ((ABS(start) > 100.) || (ABS(end) < 1.e-2)) {
    printf("Tested %d points in [%8.3le, %8.3le]\n", nb_pts, start, end);
  } else {
    printf("Tested %d points in [%3.3lf, %3.3lf]\n", nb_pts, start, end);
  }
  printf("Maximum observed absolute error is %8.3Le\n", max_abs_err);
  printf("Maximum observed relative error is %8.3Le\n", max_rel_err);
  if (max_rel_err > 0.0) {
    printf("                          which is 2^(%3.3Lf)\n",
           log2(max_rel_err));
  }
  printf("Maximum observed ULP      error is %3.3Lf\n", max_ulp_err);

  free(x);
  free(y);

  return;
}

void report_err_fp64(
  void (*test_func)(size_t, const double *, size_t,  double *, size_t), 
  long double (*ref_func)(long double),
  double start,
  double end,
  int nb_pts,
  int stride_in,
  int stride_out
) {

  long double y_ref;
  double *x, *y, delta;
  double abs_err, rel_err, ulp_err; 
  double max_abs_err, max_rel_err, max_ulp_err; 
  
  x = (double *)malloc(nb_pts * sizeof(double) * stride_in );
  y = (double *)malloc(nb_pts * sizeof(double) * stride_out );
 
  if (nb_pts <= 1){
    delta = 0.0;
    nb_pts = 1;
  } 
  else {
    delta = (end - start) / (double)(nb_pts - 1);
  }

  max_abs_err = 0.0;
  max_rel_err = 0.0;
  max_ulp_err = 0.0;

  // fill up the set of test points
  for (int i=0; i<nb_pts; ++i) {
    x[i * stride_in] = start + (double) i * delta;
  }
  
  // call function under test
  test_func( (size_t) nb_pts, x, (size_t) stride_in, y, (size_t) stride_out );

  // now for each point we compute error and log the max
  for (int j = 0; j < nb_pts; j++) {
    y_ref = ref_func((long double)x[j * stride_in]);
    abs_err = (double)((long double)y[j*stride_out] - y_ref);
    abs_err = ABS(abs_err);
    if (ABS((double)y_ref) > 0.0) {
      rel_err = abs_err / ABS((double)y_ref);
    } else {
      rel_err = abs_err / 0x1.0p-1074;
    }
    ulp_err = abs_err / ulp_64((double)y_ref);

    max_abs_err = MAX(max_abs_err, abs_err);
    max_rel_err = MAX(max_rel_err, rel_err);
    max_ulp_err = MAX(max_ulp_err, ulp_err);

    if (VERBOSE) {
      union sui64_fp64 xxx, yyy;
      xxx.f = x[j * stride_in];
      yyy.f = y[j * stride_out];
      printf("--input %24.17le, 0x%016lx, output %24.17le, 0x%016lx \n",
             xxx.f, xxx.ui, yyy.f, yyy.ui);
      printf("  reference %24.17le\n\n", (double)y_ref);
    }
  }
  printf("----------------------------\n");
  if ((ABS(start) > 100.) || (ABS(end)<1.e-2)){
    printf("Tested %d points in [%8.3le, %8.3le]\n", nb_pts, start, end);
  }
  else {
    printf("Tested %d points in [%3.3lf, %3.3lf]\n", nb_pts, start, end);
  }
  printf("Maximum observed absolute error is %8.3le\n", max_abs_err);
  printf("Maximum observed relative error is %8.3le\n", max_rel_err);
  if (max_rel_err > 0.0){
    printf("                          which is 2^(%3.3lf)\n", log2(max_rel_err));
  }
  printf("Maximum observed ULP      error is %3.3lf\n", max_ulp_err);

  free(x);
  free(y);
  
  return;
}

void report_err_powD_fp64(void (*test_func)(size_t, const double *,
                                            const double *, double *),
                          long double (*ref_func)(long double, long double),
                          double target, double start, double end, int nb_pts) {

  long double z_ref;
  double *x, *y, *z, delta;
  double abs_err, rel_err, ulp_err;
  double max_abs_err, max_rel_err, max_ulp_err;

  x = (double *)malloc(nb_pts * sizeof(double));
  y = (double *)malloc(nb_pts * sizeof(double));
  z = (double *)malloc(nb_pts * sizeof(double));

  if (nb_pts <= 1) {
    delta = 0.0;
    nb_pts = 1;
  } else {
    delta = (end - start) / (double)(nb_pts - 1);
  }

  max_abs_err = 0.0;
  max_rel_err = 0.0;
  max_ulp_err = 0.0;

  // fill up the set of test points
  for (int i = 0; i < nb_pts; ++i) {
    x[i] = start + (double)i * delta;
    y[i] = target / log(x[i]);
  }

  // call function under test
  test_func((size_t)nb_pts, x, y, z);

  // now for each point we compute error and log the max
  for (int j = 0; j < nb_pts; j++) {
    z_ref = ref_func((long double)x[j], (long double)y[j]);
    abs_err = (double)((long double)z[j] - z_ref);
    abs_err = ABS(abs_err);
    rel_err = abs_err / ABS((double)z_ref);
    ulp_err = abs_err / ulp_64((double)z_ref);

    max_abs_err = MAX(max_abs_err, abs_err);
    max_rel_err = MAX(max_rel_err, rel_err);
    max_ulp_err = MAX(max_ulp_err, ulp_err);

    if (VERBOSE) {
      union sui64_fp64 xxx, yyy, zzz;
      xxx.f = x[j];
      yyy.f = y[j];
      zzz.f = z[j];
      printf("--input %24.17le, 0x%016lx; %24.17le, 0x%016lx, output %24.17le, "
             "0x%016lx \n",
             xxx.f, xxx.ui, yyy.f, yyy.ui, zzz.f, zzz.ui);
      printf("  reference %24.17le\n\n", (double)z_ref);
    }
  }
  printf("----------------------------\n");
  if ((ABS(start) > 100.) || (ABS(end) < 1.e-2)) {
    printf("Tested %d points in [%8.3le, %8.3le]\n", nb_pts, start, end);
  } else {
    printf("Tested %d points in [%3.3lf, %3.3lf]\n", nb_pts, start, end);
  }
  printf("Maximum observed absolute error is %8.3le\n", max_abs_err);
  printf("Maximum observed relative error is %8.3le\n", max_rel_err);
  if (max_rel_err > 0.0) {
    printf("                          which is 2^(%3.3lf)\n",
           log2(max_rel_err));
  }
  printf("Maximum observed ULP      error is %3.3lf\n", max_ulp_err);

  free(x);
  free(y);
  free(z);
  return;
}

void report_err_fp80(void (*test_func)(size_t, const double *, const double *,
                                       double *, double *),
                     long double (*ref_func)(long double), double start,
                     double end, int nb_pts) {

  long double y_ref;
  double x[N_PTS_MAX], y_hi[N_PTS_MAX], y_lo[N_PTS_MAX], delta;
  double abs_err, rel_err, ulp_err;
  double max_abs_err, max_rel_err, max_ulp_err;

  printf("\n=========INDSIDE ERROR FP80 \n");

  if (nb_pts <= 1) {
    delta = 0.0;
    nb_pts = 1;
  } else {
    delta = (end - start) / (double)(nb_pts - 1);
  }

  max_abs_err = 0.0;
  max_rel_err = 0.0;
  max_ulp_err = 0.0;

  // fill up the set of test points
  for (int i = 0; i < nb_pts; ++i) {
    x[i] = start + (double)i * delta;
    printf("x[i] is %e\n", x[i]);
  }
  printf("\n---finished filling up x---\n");

  // call function under test
  test_func((size_t)nb_pts, x, x, y_hi, y_lo);
  printf("\n---finished calling test_func\n");

  // now for each point we compute error and log the max
  for (int j = 0; j < nb_pts; j++) {
    y_ref = ref_func((long double)x[j]);
    abs_err = (double)((long double)y_hi[j] - y_ref);
    abs_err += (long double)y_lo[j];
    abs_err = ABS(abs_err);
    rel_err = abs_err / ABS((double)y_ref);
    ulp_err = abs_err / ulp_64((double)y_ref);
    ulp_err *= 2048.0;
    max_abs_err = MAX(max_abs_err, abs_err);
    max_rel_err = MAX(max_rel_err, rel_err);
    max_ulp_err = MAX(max_ulp_err, ulp_err);

    if (VERBOSE) {
      union sui64_fp64 xxx, yyy;
      xxx.f = x[j];
      yyy.f = y_hi[j];
      printf("--input %24.17le, 0x%016lx, output %24.17le, 0x%016lx \n", xxx.f,
             xxx.ui, yyy.f, yyy.ui);
      printf("  reference %24.17le\n\n", (double)y_ref);
    }
  }
  printf("----------------------------\n");
  if ((ABS(start) > 100.) || (ABS(end) < 1.e-2)) {
    printf("Tested %d points in [%8.3le, %8.3le]\n", nb_pts, start, end);
  } else {
    printf("Tested %d points in [%3.3lf, %3.3lf]\n", nb_pts, start, end);
  }
  printf("Maximum observed absolute error is %8.3le\n", max_abs_err);
  printf("Maximum observed relative error is %8.3le\n", max_rel_err);
  if (max_rel_err > 0.0) {
    printf("                          which is 2^(%3.3lf)\n",
           log2(max_rel_err));
  }
  printf("Maximum observed ULP      error is %3.3lf\n", max_ulp_err);

  return;
}

void report_err_fp64(double (*test_func)(double), double (*ref_func)(double),
                     double start, double end, int nb_pts) {

  double x, y, y_ref, delta;
  double abs_err, rel_err, ulp_err;
  double max_abs_err, max_rel_err, max_ulp_err;

  if (nb_pts <= 1) {
    delta = 0.0;
    nb_pts = 1;
  } else {
    delta = (end - start) / (double)(nb_pts - 1);
  }

  max_abs_err = 0.0;
  max_rel_err = 0.0;
  max_ulp_err = 0.0;
  for (int j = 0; j < nb_pts; j++) {
    x = start + (double)j * delta;
    y = test_func(x);
    y_ref = ref_func(x);
    abs_err = y - y_ref;
    abs_err = ABS(abs_err);
    rel_err = abs_err / ABS(y_ref);
    ulp_err = abs_err / ulp_64(y_ref);

    max_abs_err = MAX(max_abs_err, abs_err);
    max_rel_err = MAX(max_rel_err, rel_err);
    max_ulp_err = MAX(max_ulp_err, ulp_err);
  }

  printf("----------------------------\n");
  if (ABS(start) > 100.) {
    printf("Tested %d points in [%8.3le, %8.3le]\n", nb_pts, start, end);
  } else {
    printf("Tested %d points in [%3.3lf, %3.3lf]\n", nb_pts, start, end);
  }
  printf("Maximum observed absolute error is %8.3le\n", max_abs_err);
  printf("Maximum observed relative error is %8.3le\n", max_rel_err);
  if (max_rel_err > 0.0) {
    printf("                          which is 2^(%3.3lf)\n",
           log2(max_rel_err));
  }
  printf("Maximum observed ULP      error is %3.3lf\n", max_ulp_err);

  return;
}

void show_special_fp64(void (*test_func)(size_t, const double *, double *),
                       const char *title) {

  double x[N_SPECIALS], y[N_SPECIALS];
  union sui64_fp64 xxx, yyy;

  for (int i = 0; i < N_SPECIALS; ++i) {
    x[i] = fp64_special_values[i].f;
  }
  test_func((size_t)N_SPECIALS, x, y);
  printf("\n\n");
  printf("\t%s\n", title);
  for (int i = 0; i < N_SPECIALS; ++i) {
    xxx.f = x[i];
    yyy.f = y[i];
    printf("\tinput  x %24.17le,\t in hex 0x%016lx\n", xxx.f, xxx.ui);
    printf("\toutput y %24.17le,\t in hex 0x%016lx\n", yyy.f, yyy.ui);
    printf("---\n");
  }
}

void show_special2_fp64(void (*test_func)(size_t, const double *,
                                          const double *, double *),
                        char *title) {

  double x[N_SPECIALS * N_SPECIALS], y[N_SPECIALS * N_SPECIALS],
      z[N_SPECIALS * N_SPECIALS];
  union sui64_fp64 xxx, yyy, zzz;

  int cnt = 0;
  for (int i = 0; i < N_SPECIALS; ++i) {
    for (int j = 0; j < N_SPECIALS; ++j) {
      x[cnt] = fp64_special_values[i].f;
      y[cnt] = fp64_special_values[j].f;
      cnt++;
    }
  }
  test_func((size_t)N_SPECIALS * N_SPECIALS, x, y, z);
  printf("\n\n");
  printf("\t%s\n", title);
  for (int i = 0; i < N_SPECIALS * N_SPECIALS; ++i) {
    xxx.f = x[i];
    yyy.f = y[i];
    zzz.f = z[i];
    printf("\tinput  x %24.17le,\t in hex 0x%016lx\n", xxx.f, xxx.ui);
    printf("\tinput  y %24.17le,\t in hex 0x%016lx\n", yyy.f, yyy.ui);
    printf("\toutput z %24.17le,\t in hex 0x%016lx\n", zzz.f, zzz.ui);
    printf("---\n");
  }
}

void trig_2pi_reduction(long double x, long double *rem, int64_t *n) {
  long double two_neg70 = 0x1.0p-70;
  long double two_70 = 0x1.0p70;
  long double two_neg20 = 0x1.0p-20;
  long double two_20 = 0x1.0p20;
  long double n_flt;
  long double zero = 0.0;
  long double r;

  if ((x > 0x1.0p120) || (x < -0x1.0p120)) {
    *rem = 0.0;
    *n = 0;
  } else {
    n_flt = roundl(x * two_neg70);
    r = x - two_70 * n_flt;
    n_flt = roundl(r * two_neg20);
    r = r - two_20 * n_flt;
    n_flt = roundl(r * 0x1.0p-1);
    r = r - 2.0 * n_flt;
    n_flt = roundl(r * 0x1.0p1);
    r = r - n_flt * 0x1.0p-1;
    *rem = r;
    *n = (int64_t)n_flt;
  }
}

long double acospil(long double x) {
  long double pi_inv = 0x1.45f3'06dc'9c88'2a53'f84e'afa3'ea6ap-2L;
  long double result;
  result = acosl(x) * pi_inv;
  return result;
}

long double asinpil(long double x) {
  long double pi_inv = 0x1.45f306dc9c882a53f84eafa3ea6ap-2L;
  long double result;
  result = asinl(x) * pi_inv;
  return result;
}

long double atanpil(long double x) {
  long double pi_inv = 0x1.45f3'06dc'9c88'2a53'f84e'afa3'ea6ap-2L;
  long double result;
  result = atanl(x) * pi_inv;
  return result;
}

long double sinpil(long double x) {
  long double pi = 0x1.921f'b5444'2d18'4698'98cc'5170'1b83'9p1L;
  long double rem, result;
  int64_t n;
  trig_2pi_reduction(x, &rem, &n);
  rem *= pi;
  result = ((n & 0x1) == 0) ? sinl(rem) : cosl(rem);
  if (result != 0.0) {
    result = (((n >> 1) & 0x1) == 0) ? result : -result;
  }
  return result;
}

long double cospil(long double x) {
  long double pi = 0x1.921f'b5444'2d18'4698'98cc'5170'1b83'9p1L;
  long double rem, result;
  int64_t n;
  trig_2pi_reduction(x, &rem, &n);
  rem *= pi;
  result = ((n & 0x1) == 0) ? cos(rem) : -sin(rem);
  if (result != 0.0) {
    result = (((n >> 1) & 0x1) == 0) ? result : -result;
  }
  return result;
}

long double tanpil(long double x) {
  long double pi = 0x1.921f'b5444'2d18'4698'98cc'5170'1b83'9p1L;
  long double rem, result;
  int64_t n;
  long double abs_x = (x >= 0.0) ? x : -x;
  trig_2pi_reduction(abs_x, &rem, &n);
  rem *= pi;
  result = ((n & 0x1) == 0) ? tanl(rem) : -(long double)1.0 / tanl(rem);
  if ((rem == 0.0) && ((n & 0x1) == 1)) {
    result = (result > 0.0) ? result : -result;
  }
  result = (x >= 0.0) ? result : -result;
  return result;
}
