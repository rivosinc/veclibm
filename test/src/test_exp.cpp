// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(exp, special) {
  int nb_tv;
  int64_t tv_in_out[2 * (NB_TV_FOR_EXP)] = TV_FOR_EXP;

  nb_tv = NB_TV_FOR_EXP;

  test_vectors_fp64(rvvlm_exp, tv_in_out, nb_tv);
}

TEST(exp, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("exp: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0.34;
  x_end = 0.34;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);

  x_start = -3.0;
  x_end = 3.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);
}

TEST(exp, medium_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("exp: current chosen algorithm; reduced argument in FP64 only")

  x_start = -15.0;
  x_end = -10.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);

  x_start = 15.0;
  x_end = 10.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);
}

TEST(exp, large_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("exp: current chosen algorithm; reduced argument in FP64 only")

  x_start = 700.0;
  x_end = 709.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);

  x_start = -740.0;
  x_end = -709.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_exp, expl, x_start, x_end, nb_tests);
}
