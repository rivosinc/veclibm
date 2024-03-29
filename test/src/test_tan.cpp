// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(tan, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("tan: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_tan, "Special Value handling of this function");
}

TEST(tan, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("tan: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0.7;
  x_end = 0.7;
  nb_tests = 10000;
  report_err_fp64(rvvlm_tan, tanl, x_start, x_end, nb_tests);

  x_start = -6.0;
  x_end = 6.0;
  nb_tests = 10000;
  report_err_fp64(rvvlm_tan, tanl, x_start, x_end, nb_tests);
}

TEST(tan, large_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("tan: current chosen algorithm; reduced argument in FP64 only")

  x_start = 1.0;
  x_end = 0x1.0p23;
  nb_tests = 30000;
  report_err_fp64(rvvlm_tan, tanl, x_start, x_end, nb_tests);

  x_start = 0x1.0p25;
  x_end = 0x1.0p100;
  nb_tests = 30000;
  report_err_fp64(rvvlm_tan, tanl, x_start, x_end, nb_tests);

  x_start = 0x1.ffffp1023;
  x_end = 0x1.0p1000;
  nb_tests = 30000;
  report_err_fp64(rvvlm_tan, tanl, x_start, x_end, nb_tests);
}
