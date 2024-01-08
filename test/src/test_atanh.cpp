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

  COMMENT("atanh: current chosen algorithm; reduced argument in FP64 only")

  // show_special_fp64(rvvlm_atanh, "Special Value handling of this function");

  x_start = 0x1.0p-40;
  x_end = 0x1.0p-35;
  nb_tests = 40000;
  report_err_fp64(rvvlm_atanh, atanhl, x_start, x_end, nb_tests);

  x_start = -0x1.0p-35;
  x_end = -0x1.0p-40;
  nb_tests = 40000;
  report_err_fp64(rvvlm_atanh, atanhl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-20;
  x_end = 0x1.0p-10;
  nb_tests = 400000;
  report_err_fp64(rvvlm_atanh, atanhl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-10;
  x_end = 0x1.ffp-3;
  nb_tests = 400000;
  report_err_fp64(rvvlm_atanh, atanhl, x_start, x_end, nb_tests);

  x_start = 0x1.00p-9;
  x_end = 0x1.0p-2;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_atanh, atanhl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1;
  x_end = 0x1.0p0 - 0x1.0p-51;
  nb_tests = 400000;
  report_err_fp64(rvvlm_atanh, atanhl, x_start, x_end, nb_tests);

  x_start = -0x1.0p-1;
  x_end = -(0x1.0p0 - 0x1.0p-51);
  nb_tests = 400000;
  report_err_fp64(rvvlm_atanh, atanhl, x_start, x_end, nb_tests);

  x_start = -1.0 + 0x1.0p-53;
  x_end = 1.0 - 0x1.0p-51;
  nb_tests = 400000;
  report_err_fp64(rvvlm_atanh, atanhl, x_start, x_end, nb_tests);

  return 0;
}
