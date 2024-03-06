// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(expm1, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("expm1: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_expm1, "Special Value handling of this function");

  x_start = -0.01;
  x_end = 0.01;
  nb_tests = 300000;
  report_err_fp64(rvvlm_expm1, expm1l, x_start, x_end, nb_tests);

  x_start = -.3;
  x_end = 0.3;
  nb_tests = 100000;
  report_err_fp64(rvvlm_expm1, expm1l, x_start, x_end, nb_tests);
}

TEST(expm1, medium_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("expm1: current chosen algorithm; reduced argument in FP64 only")

  x_start = 1.0;
  x_end = 10.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_expm1, expm1l, x_start, x_end, nb_tests);

  x_start = -10.0;
  x_end = -1.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_expm1, expm1l, x_start, x_end, nb_tests);

  x_start = -40.0;
  x_end = -36.0;
  nb_tests = 10000;
  report_err_fp64(rvvlm_expm1, expm1l, x_start, x_end, nb_tests);

  x_start = 36.0;
  x_end = 40.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_expm1, expm1l, x_start, x_end, nb_tests);
}

TEST(expm1, large_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("expm1: current chosen algorithm; reduced argument in FP64 only")

  x_start = 700.0;
  x_end = 709.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_expm1, expm1l, x_start, x_end, nb_tests);

  x_start = -50.0;
  x_end = -40.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_expm1, expm1l, x_start, x_end, nb_tests);
}
