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

  COMMENT("log10: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_log10, "Special Value handling of this function");

  x_start = 0.7;
  x_end = 1.5;
  nb_tests = 1000000;
  report_err_fp64(rvvlm_log10, log10l, x_start, x_end, nb_tests);

  x_start = 1.0;
  x_end = 4.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_log10, log10l, x_start, x_end, nb_tests);

  x_start = 0.25;
  x_end = 1.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_log10, log10l, x_start, x_end, nb_tests);

  x_start = 0x1.0p1023;
  x_end = 0x1.ffffffp1023;
  nb_tests = 40000;
  report_err_fp64(rvvlm_log10, log10l, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1074;
  x_end = 0x1.0p-1022;
  nb_tests = 400000;
  report_err_fp64(rvvlm_log10, log10l, x_start, x_end, nb_tests);

  return 0;
}
