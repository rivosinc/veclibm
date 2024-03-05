// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(cos, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cos: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_cos, "Special Value handling of this function");

  x_start = -0.78;
  x_end = 0.78;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_cos, cosl, x_start, x_end, nb_tests);

  x_start = -6.0;
  x_end = 6.0;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_cos, cosl, x_start, x_end, nb_tests);

  x_start = 1.0;
  x_end = 0x1.0p23;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_cos, cosl, x_start, x_end, nb_tests);

  x_start = 0x1.0p25;
  x_end = 0x1.0p100;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_cos, cosl, x_start, x_end, nb_tests);
}
