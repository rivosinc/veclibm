// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(sinpiI, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("sinI: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0.5;
  x_end = 0.5;
  nb_tests = 30;
  int stride_x = 21;
  int stride_y = 39;
  report_err_fp64(rvvlm_sinpiI, sinpil, x_start, x_end, nb_tests, stride_x,
                  stride_y);
}
