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

  COMMENT("exp: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_exp, "Special Value handling of this function");

  x_start = -0.34;
  x_end = 0.34;
  nb_tests = 30000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);

  x_start = -3.0;
  x_end = 3.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);

  x_start = 10.0;
  x_end = 15.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);

  x_start = 700.0;
  x_end = 709.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);

  x_start = -740.0;
  x_end = -709.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);

  return 0;
}
