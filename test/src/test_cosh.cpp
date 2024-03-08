// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(cosh, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cosh: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_cosh, "Special Value handling of this function");
}

TEST(cosh, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cosh: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0.346;
  x_end = 0.346;
  nb_tests = 40000;
  report_err_fp64(rvvlm_cosh, coshl, x_start, x_end, nb_tests);

  x_start = -6.0;
  x_end = 6.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_cosh, coshl, x_start, x_end, nb_tests);
}

TEST(cosh, medium_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cosh: current chosen algorithm; reduced argument in FP64 only")

  x_start = 30.0;
  x_end = 40.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_cosh, coshl, x_start, x_end, nb_tests);

  x_start = 50.0;
  x_end = 60.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_cosh, coshl, x_start, x_end, nb_tests);
}

TEST(cosh, large_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cosh: current chosen algorithm; reduced argument in FP64 only")

  x_start = 60.0;
  x_end = 70.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_cosh, coshl, x_start, x_end, nb_tests);
}
