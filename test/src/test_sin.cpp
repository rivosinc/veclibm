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

  COMMENT("sin: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_sin, "Special Value handling of this function");

  x_start = -0.78;
  x_end = 0.78;
  nb_tests = 400000;
  report_err_fp64(rvvlm_sin, sinl, x_start, x_end, nb_tests);

  x_start = -6.0;
  x_end = 6.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_sin, sinl, x_start, x_end, nb_tests);

  x_start = 1.0;
  x_end = 0x1.0p23;
  nb_tests = 400000;
  report_err_fp64(rvvlm_sin, sinl, x_start, x_end, nb_tests);

  x_start = 0x1.0p25;
  x_end = 0x1.0p100;
  nb_tests = 400000;
  report_err_fp64(rvvlm_sin, sinl, x_start, x_end, nb_tests);

  return 0;
}
