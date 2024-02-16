// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(erf, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erf: current chosen algorithm; reduced argument in FP64 only")

  // show_special_fp64(rvvlm_erf, "Special Value handling of this function");

  x_start = 0x1.0p-40;
  x_end = 0x1.0p-20;
  nb_tests = 40000;
  report_err_fp64(rvvlm_erf, erfl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-20;
  x_end = 0x1.0p1;
  nb_tests = 40000;
  report_err_fp64(rvvlm_erf, erfl, x_start, x_end, nb_tests);

  x_start = 0x1.0p1;
  x_end = 0x1.0p2;
  nb_tests = 40000;
  report_err_fp64(rvvlm_erf, erfl, x_start, x_end, nb_tests);

  x_start = 0x1.0p2;
  x_end = 7.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_erf, erfl, x_start, x_end, nb_tests);

  x_start = -6.5;
  x_end = 6.5;
  nb_tests = 40000;
  report_err_fp64(rvvlm_erf, erfl, x_start, x_end, nb_tests);
}
