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

  COMMENT("cdfnorm: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_cdfnorm, "Special Value handling of this function");

  x_start = -0x1.0p0;
  x_end = 0x1.0p0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, x_start, x_end, nb_tests);

  x_start = 0x1.0p-2;
  x_end = 0x1.0p0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, x_start, x_end, nb_tests);
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, -x_start, -x_end, nb_tests);

  x_start = 0x1.0p0;
  x_end = 0x1.0p3;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, x_start, x_end, nb_tests);
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, -x_start, -x_end, nb_tests);

  x_start = 0x1.0p3;
  x_end = 30.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, x_start, x_end, nb_tests);
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, -x_start, -x_end, nb_tests);

  x_start = 0x1.0p-80;
  x_end = 0x1.0p-50;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, x_start, x_end, nb_tests);
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, -x_start, -x_end, nb_tests);

  x_start = 0x1.0p-50;
  x_end = 0x1.0p-10;
  nb_tests = 400000;
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, x_start, x_end, nb_tests);
  report_err_fp64(rvvlm_cdfnorm, cdfnorml, -x_start, -x_end, nb_tests);

  return 0;
}
