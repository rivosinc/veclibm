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

  COMMENT("atanpi: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_atanpi, "Special Value handling of this function");

  x_start = -0x1.0p-40;
  x_end = 0x1.0p-40;
  ;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_atanpi, atanpil, x_start, x_end, nb_tests);

  x_start = 0.0;
  x_end = 1.0;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_atanpi, atanpil, x_start, x_end, nb_tests);

  x_start = 1.0;
  x_end = 100.0;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_atanpi, atanpil, x_start, x_end, nb_tests);

  x_start = -10.0;
  x_end = 10.0;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_atanpi, atanpil, x_start, x_end, nb_tests);

  return 0;
}
