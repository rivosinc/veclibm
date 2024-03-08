// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(tanpi, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("tanpi: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_tanpi, "Special Value handling of this function");

  x_start = -0.4999;
  x_end = 0.49999;
  nb_tests = 100000;
  report_err_fp64(rvvlm_tanpi, tanpil, x_start, x_end, nb_tests);

  x_start = -0x1.0p3;
  x_end = 0x1.0p3;
  nb_tests = 100000;
  report_err_fp64(rvvlm_tanpi, tanpil, x_start, x_end, nb_tests);

  x_start = 0x1.0p3;
  x_end = 0x1.0p60;
  nb_tests = 100000;
  report_err_fp64(rvvlm_tanpi, tanpil, x_start, x_end, nb_tests);
}
