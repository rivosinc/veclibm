// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(lgamma, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("lgamma: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_lgamma, "Special Value handling of this function");
}

TEST(lgamma, lgamma_neg) {
  unsigned long nb_tests;
  double x_start, x_end;

  x_start = -0.999;
  x_end = -0.0001;
  nb_tests = 5000;
  report_mixederr_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = -1.99;
  x_end = -1.01;
  nb_tests = 5000;
  report_mixederr_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = -2.99;
  x_end = -2.01;
  nb_tests = 5000;
  report_mixederr_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = -3.99;
  x_end = -3.01;
  nb_tests = 5000;
  report_mixederr_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = -4.99;
  x_end = -4.01;
  nb_tests = 5000;
  report_mixederr_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = -5.99;
  x_end = -5.01;
  nb_tests = 5000;
  report_mixederr_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = -6.9999;
  x_end = -6.00001;
  nb_tests = 10000;
  report_mixederr_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = -1000.9999;
  x_end = -1000.0001;
  nb_tests = 10000;
  report_mixederr_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);
}
TEST(lgamma, lgamma_left) {
  unsigned long nb_tests;
  double x_start, x_end;

  x_start = 0x1.0p-1070;
  x_end = 0x1.0p-1065;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 0x1.0p-62;
  x_end = 0x1.0p-58;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 0x1.0p-58;
  x_end = 0x1.0p-40;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 0x1.0p-4;
  x_end = 0x1.0p-1;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1;
  x_end = 0x1.ffffffffp-1;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 1.0;
  x_end = 0x1.ffffffffp0;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 0x1.0p1;
  x_end = 0x1.ffffffffp1;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 0x1.0p2;
  x_end = 0x1.dfffffffp-1;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);
}

TEST(lgamma, lgamma_right) {
  unsigned long nb_tests;
  double x_start, x_end;

  x_start = 2.25;
  x_end = 10.0;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 10.0;
  x_end = 100.0;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);
}

TEST(lgamma, lgamma_large) {
  unsigned long nb_tests;
  double x_start, x_end;

  x_start = 0x1.0p800;
  x_end = 0x1.0p802;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);

  x_start = 0x1.1p1012;
  x_end = 0x1.fffffp1012;
  nb_tests = 5000;
  report_err_fp64(rvvlm_lgamma, lgammal, x_start, x_end, nb_tests);
}
