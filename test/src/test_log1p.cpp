// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(log1p, special) {
  int nb_tv;
  int64_t tv_in_out[2 * (NB_TV_FOR_LOG1P)] = TV_FOR_LOG1P;

  nb_tv = NB_TV_FOR_LOG1P;

  test_vectors_fp64(rvvlm_log1p, tv_in_out, nb_tv);
}

TEST(log1p, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("log1p: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0x1.0p-4;
  x_end = 0x1.0p-4;
  nb_tests = 40000;
  report_err_fp64(rvvlm_log1p, log1pl, x_start, x_end, nb_tests);

  x_start = -0.999999;
  x_end = 0.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_log1p, log1pl, x_start, x_end, nb_tests);
}

TEST(log1p, large_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("log1p: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p1023;
  x_end = 0x1.ffffffp1023;
  nb_tests = 40000;
  report_err_fp64(rvvlm_log1p, log1pl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1074;
  x_end = 0x1.0p-1022;
  nb_tests = 40000;
  report_err_fp64(rvvlm_log1p, log1pl, x_start, x_end, nb_tests);
}
