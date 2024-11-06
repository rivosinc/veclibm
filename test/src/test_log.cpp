// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(log, special) {
  int nb_tv;
  int64_t tv_in_out[2 * (NB_TV_FOR_LOG)] = TV_FOR_LOG;

  nb_tv = NB_TV_FOR_LOG;

  test_vectors_fp64(rvvlm_log, tv_in_out, nb_tv);
}

TEST(log, around_1) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("log: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0.7;
  x_end = 1.5;
  nb_tests = 10000;
  report_err_fp64(rvvlm_log, logl, x_start, x_end, nb_tests);

  x_start = 1.0;
  x_end = 4.0;
  nb_tests = 10000;
  report_err_fp64(rvvlm_log, logl, x_start, x_end, nb_tests);

  x_start = 0.25;
  x_end = 1.0;
  nb_tests = 100000;
  report_err_fp64(rvvlm_log, logl, x_start, x_end, nb_tests);
}

TEST(log, extreme_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("log: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p1023;
  x_end = 0x1.ffffffp1023;
  nb_tests = 100000;
  report_err_fp64(rvvlm_log, logl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1074;
  x_end = 0x1.0p-1022;
  nb_tests = 100000;
  report_err_fp64(rvvlm_log, logl, x_start, x_end, nb_tests);
}
