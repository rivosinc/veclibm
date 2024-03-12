// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(sinh, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("sinh: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_sinh, "Special Value handling of this function");
}

TEST(sinh, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("sinh: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0.346;
  x_end = 0.346;
  nb_tests = 40000;
  report_err_fp64(rvvlm_sinh, sinhl, x_start, x_end, nb_tests);

  x_start = -6.0;
  x_end = 6.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_sinh, sinhl, x_start, x_end, nb_tests);
}

TEST(sinh, medium_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("sinh: current chosen algorithm; reduced argument in FP64 only")

  x_start = 30.0;
  x_end = 40.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_sinh, sinhl, x_start, x_end, nb_tests);

  x_start = 50.0;
  x_end = 60.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_sinh, sinhl, x_start, x_end, nb_tests);
}

TEST(sinh, large_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("sinh: current chosen algorithm; reduced argument in FP64 only")

  x_start = 60.0;
  x_end = 70.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_sinh, sinhl, x_start, x_end, nb_tests);
}
