// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(erfinv, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erfinv: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_erfinv, "Special Value handling of this function");
}

TEST(erfinv, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erfinv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-40;
  ;
  x_end = 0x1.0p-30;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);

  x_start = 0x1.0p-30;
  ;
  x_end = 0x1.0p-10;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);

  x_start = 0x1.0p-10;
  ;
  x_end = 0x1.0p-4;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);
}

TEST(erfinv, middle_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erfinv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-4;
  ;
  x_end = 0x1.0p-2;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);

  x_start = 0x1.0p-2;
  ;
  x_end = 0x1.0p-1;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);

  x_start = 0x1.0p-1;
  ;
  x_end = 0x1.78p-1;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);

  x_start = 0x1.6p-1;
  ;
  x_end = 0x1.7cp-1;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);

  x_start = 0x1.71p-1;
  ;
  x_end = 0x1.8p-1;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);

  x_start = -0.8;
  x_end = 0.8;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);

  x_start = 0.25;
  x_end = 0.9;
  nb_tests = 2;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);
}

TEST(erfinv, close_to_1) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erfinv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 1.0 - 0x1.0p-53;
  x_end = 0x1.ffp-1;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfinv, erfl, erfl_prime, x_start, x_end,
                        nb_tests);
}
