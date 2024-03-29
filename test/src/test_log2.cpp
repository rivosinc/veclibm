// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(log2, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("log2: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_log2, "Special Value handling of this function");
}

TEST(log2, around_1) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("log2: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0.7;
  x_end = 1.5;
  nb_tests = 10000;
  report_err_fp64(rvvlm_log2, log2l, x_start, x_end, nb_tests);

  x_start = 1.0;
  x_end = 4.0;
  nb_tests = 10000;
  report_err_fp64(rvvlm_log2, log2l, x_start, x_end, nb_tests);

  x_start = 0.25;
  x_end = 1.0;
  nb_tests = 10000;
  report_err_fp64(rvvlm_log2, log2l, x_start, x_end, nb_tests);
}

TEST(log2, extreme_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("log2: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p1023;
  x_end = 0x1.ffffffp1023;
  nb_tests = 10000;
  report_err_fp64(rvvlm_log2, log2l, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1074;
  x_end = 0x1.0p-1022;
  nb_tests = 10000;
  report_err_fp64(rvvlm_log2, log2l, x_start, x_end, nb_tests);
}
