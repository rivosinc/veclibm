// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(expint1, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("expint1: current chosen algorithm")

  show_special_fp64(rvvlm_expint1, "Special Value handling of this function");

  x_start = 0x1.0p-10;
  x_end = 0x1.8p0;
  nb_tests = 4;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1070;
  x_end = 0x1.0p-1060;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0x1.0p-4;
  x_end = 0x1.0p-3;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0x1.0p-3;
  x_end = 0x1.0p-2;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0.25;
  x_end = 0.3;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0.3;
  x_end = 0.4;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0.4;
  x_end = 0.5;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0.5;
  x_end = 0.6;
  nb_tests = 16;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0.6;
  x_end = 0.7;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0.7;
  x_end = 0.8;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0.8;
  x_end = 0.9;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0.9;
  x_end = 0x1.ffffffffffffp-1;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 0x1.0p0 - 0x1.0p-53;
  x_end = 0x1.ffffp-1;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 1.0;
  x_end = 8.0;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 8.0;
  x_end = 100.0;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 100.0;
  x_end = 700.0;
  nb_tests = 1000;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);

  x_start = 700.;
  x_end = 735.0;
  nb_tests = 100;
  report_err_fp64(rvvlm_expint1, expint1, x_start, x_end, nb_tests);
}
