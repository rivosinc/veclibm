// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(asinh, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("asinh: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_asinh, "Special Value handling of this function");

  x_start = 0x1.0p-40;
  x_end = 0x1.0p-35;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinh, asinhl, x_start, x_end, nb_tests);

  x_start = -0x1.0p-35;
  x_end = -0x1.0p-40;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinh, asinhl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-20;
  x_end = 0x1.0p-10;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinh, asinhl, x_start, x_end, nb_tests);

  x_start = 0x1.0p-6;
  x_end = 0x1.0p0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinh, asinhl, x_start, x_end, nb_tests);

  x_start = 0x1.0p0;
  x_end = 0x1.0p2;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinh, asinhl, x_start, x_end, nb_tests);

  x_start = -0x1.0p0;
  x_end = -0x1.0p2;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinh, asinhl, x_start, x_end, nb_tests);

  x_start = 0x1.0p490;
  x_end = 0x1.0p520;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinh, asinhl, x_start, x_end, nb_tests);

  x_start = 0x1.0p1020;
  x_end = 0x1.FFFFFFFFFFp1023;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinh, asinhl, x_start, x_end, nb_tests);
}
