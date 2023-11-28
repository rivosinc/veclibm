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

  COMMENT("log1p: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_log1p, "Special Value handling of this function");

  x_start = -0x1.0p-4;
  x_end = 0x1.0p-4;
  nb_tests = 400000;
  report_err_fp64(rvvlm_log1p, log1pl, x_start, x_end, nb_tests);

  x_start = -0.999999;
  x_end = 0.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_log1p, log1pl, x_start, x_end, nb_tests);

  x_start = 0x1.0p1023;
  x_end = 0x1.ffffffp1023;
  nb_tests = 4000;
  report_err_fp64(rvvlm_log1p, log1pl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1074;
  x_end = 0x1.0p-1022;
  nb_tests = 4000;
  report_err_fp64(rvvlm_log1p, log1pl, x_start, x_end, nb_tests);

  return 0;
}
