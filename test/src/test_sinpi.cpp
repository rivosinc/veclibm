// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(sinpi, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("sinpi: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_sinpi, "Special Value handling of this function");

  x_start = -0.5;
  x_end = 0.5;
  nb_tests = 100000;
  report_err_fp64(rvvlm_sinpi, sinpil, x_start, x_end, nb_tests);

  x_start = -0x1.0p3;
  x_end = 0x1.0p3;
  nb_tests = 100000;
  report_err_fp64(rvvlm_sinpi, sinpil, x_start, x_end, nb_tests);

  x_start = 0x1.0p3;
  x_end = 0x1.0p60;
  nb_tests = 100000;
  report_err_fp64(rvvlm_sinpi, sinpil, x_start, x_end, nb_tests);
}
