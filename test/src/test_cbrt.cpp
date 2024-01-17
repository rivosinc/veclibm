// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <math.h>
#include <stdio.h>

#include "rvvlm.h"
#include "test_infra.h"

#define COMMENT(comment)                                                       \
  { printf("\n=====\t" comment "\n"); }

int main() {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cbrt: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_cbrt, "Special Value handling of this function");

  x_start = 0x1.0p-1074;
  x_end = 0x1.0p-1064;
  nb_tests = 40000;
  report_err_fp64(rvvlm_cbrt, cbrtl, x_start, x_end, nb_tests);

  x_start = -0x1.0p-1064;
  x_end = -0x1.0p-1074;
  nb_tests = 40000;
  report_err_fp64(rvvlm_cbrt, cbrtl, x_start, x_end, nb_tests);

  x_start = 0x1.0p0;
  x_end = 0x1.0p8;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cbrt, cbrtl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-8;
  x_end = 0x1.0p-4;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cbrt, cbrtl, x_start, x_end, nb_tests);

  x_start = -0x1.0p0;
  x_end = -0x1.0p8;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cbrt, cbrtl, x_start, x_end, nb_tests);

  x_start = -0x1.0p-8;
  x_end = -0x1.0p-4;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cbrt, cbrtl, x_start, x_end, nb_tests);

  x_start = 0x1.0p1020;
  x_end = 0x1.fffffp1023;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_cbrt, cbrtl, x_start, x_end, nb_tests);

  x_start = -0x1.0p1020;
  x_end = -0x1.fffffp1023;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_cbrt, cbrtl, x_start, x_end, nb_tests);

  return 0;
}
