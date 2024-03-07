// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(erfcinv, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erfcinv: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_erfcinv, "Special Value handling of this function");
}

TEST(erfcinv, tiny_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erfcinv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-1074;;
  x_end = 0x1.0p-1000;
  nb_tests = 40000;
  double thres = 3.5;
  report_err_byinv_fp64(rvvlm_erfcinv, erfcl, erfcl_prime, x_start, x_end, nb_tests, thres);

  x_start = 0x1.0p-1000;;
  x_end = 0x1.0p-500;
  nb_tests = 40000;
  thres = 2.5;
  report_err_byinv_fp64(rvvlm_erfcinv, erfcl, erfcl_prime, x_start, x_end, nb_tests, thres);

  x_start = 0x1.0p-500;;
  x_end = 0x1.0p-52;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfcinv, erfcl, erfcl_prime, x_start, x_end, nb_tests, thres);
}

TEST(erfcinv, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erfcinv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-50;;
  x_end = 0x1.0p-20;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfcinv, erfcl, erfcl_prime, x_start, x_end, nb_tests);

  x_start = 0x1.0p-20;;
  x_end = 0x1.0p-4;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfcinv, erfcl, erfcl_prime, x_start, x_end, nb_tests);
}

TEST(erfcinv, medium_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("erfcinv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-4;;
  x_end = 0x1.0p-1;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfcinv, erfcl, erfcl_prime, x_start, x_end, nb_tests);

  x_start = 0x1.0p-1;;
  x_end = 0x1.8p0;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfcinv, erfcl, erfcl_prime, x_start, x_end, nb_tests);

  x_start = 2.0 - 0x1.0p-52;
  x_end = 0x1.8p0;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_erfcinv, erfcl, erfcl_prime, x_start, x_end, nb_tests);
}

