// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(tgamma, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("tgamma: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_tgamma, "Special Value handling of this function");
}

TEST(tgamma, gamma_tiny) {
  unsigned long nb_tests;
  double x_start, x_end;

  x_start = 0x1.0p-120;
  x_end = 0x1.0p-118;
  nb_tests = 1000;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);

  x_start = 0x1.0p-50;
  x_end = 0x1.0p-40;
  nb_tests = 1000;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);

  x_start = -0x1.0p-118;
  x_end = -0x1.0p-120;
  nb_tests = 1000;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);

  x_start = -0x1.0p-40;
  x_end = -0x1.0p-50;
  nb_tests = 1000;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);
}

TEST(tgamma, gamma_moderate) {
  unsigned long nb_tests;
  double x_start, x_end;

  x_start = 0.5;
  x_end = 4.0;
  nb_tests = 1000;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);

  x_start = 4.0;
  x_end = 10.0;
  nb_tests = 1000;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);

  x_start = -0.9;
  x_end = -0.2;
  nb_tests = 1000;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);

  x_start = -10.1;
  x_end = -0.9;
  nb_tests = 1231;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);
}

TEST(tgamma, gamma_large) {
  unsigned long nb_tests;
  double x_start, x_end;

  x_start = 100.0;
  x_end = 171.0;
  nb_tests = 1000;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);

  x_start = -171.1;
  x_end = -100.3;
  nb_tests = 1317;
  report_err_fp64(rvvlm_tgamma, tgammal, x_start, x_end, nb_tests);
}
