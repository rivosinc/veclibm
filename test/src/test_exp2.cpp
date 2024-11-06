// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(exp2, special) {
  int nb_tv;
  int64_t tv_in_out[2 * (NB_TV_FOR_EXP)] = TV_FOR_EXP;

  nb_tv = NB_TV_FOR_EXP;

  test_vectors_fp64(rvvlm_exp2, tv_in_out, nb_tv);
}

TEST(exp2, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("exp2: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0.34;
  x_end = 0.34;
  nb_tests = 80000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);

  x_start = -3.0;
  x_end = 3.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);
}

TEST(exp2, medium_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("exp2: current chosen algorithm; reduced argument in FP64 only")

  x_start = -14.0;
  x_end = -10.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);

  x_start = 10.0;
  x_end = 15.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);
}

TEST(exp2, large_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("exp2: current chosen algorithm; reduced argument in FP64 only")

  x_start = 1010.0;
  x_end = 1023.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);

  x_start = -1070.;
  x_end = -1020.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_exp2, exp2l, x_start, x_end, nb_tests);
}
