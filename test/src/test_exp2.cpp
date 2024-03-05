// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(exp2, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("exp2: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_exp2, "Special Value handling of this function");

  x_start = -0.34;
  x_end = 0.34;
  nb_tests = 3000000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);

  x_start = -3.0;
  x_end = 3.0;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);

  x_start = 10.0;
  x_end = 15.0;
  nb_tests = 4000000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);

  x_start = 1010.0;
  x_end = 1023.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);

  x_start = -1070.;
  x_end = -1020.0;
  nb_tests = 400000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);
}
