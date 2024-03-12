// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(cdfnorminv, special) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cdfnorminv: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_cdfnorminv,
                    "Special Value handling of this function");
}

TEST(cdfnorminv, tiny_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cdfnorminv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-1074;
  x_end = 0x1.0p-1000;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_cdfnorminv, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests);

  x_start = 0x1.0p-1000;
  x_end = 0x1.0p-500;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_cdfnorminv, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests);

  x_start = 0x1.0p-55;
  x_end = 0x1.0p-53;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_cdfnorminv, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests);
}

TEST(cdfnorminv, small_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cdfnorminv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-50;
  x_end = 0x1.0p-20;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_cdfnorminv, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests);

  x_start = 0x1.0p-20;
  x_end = 0x1.0p-4;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_cdfnorminv, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests);
}

TEST(cdfnorminv, medium_args) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("cdfnorminv: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-5;
  x_end = 0x1.0p-2;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_cdfnorminv, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests);

  x_start = 0x1.0p-2;
  x_end = 0x1.8p-1;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_cdfnorminv, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests);

  x_start = 1.0 - 0x1.0p-53;
  x_end = 0x1.8p-1;
  nb_tests = 40000;
  report_err_byinv_fp64(rvvlm_cdfnorminv, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests);
}
