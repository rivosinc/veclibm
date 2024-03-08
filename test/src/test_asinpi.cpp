// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(asinpI, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("asinpi: current chosen algorithm; reduced argument in FP64 only")

  show_special_fp64(rvvlm_asinpi, "Special Value handling of this function");

  x_start = -0x1.0p-40;
  x_end = 0x1.0p-40;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinpi, asinpil, x_start, x_end, nb_tests);

  x_start = 0.0;
  x_end = 0.5;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinpi, asinpil, x_start, x_end, nb_tests);

  x_start = 0.5;
  x_end = 1.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinpi, asinpil, x_start, x_end, nb_tests);

  x_start = -1.0;
  x_end = 1.0;
  nb_tests = 40000;
  report_err_fp64(rvvlm_asinpi, asinpil, x_start, x_end, nb_tests);
}
