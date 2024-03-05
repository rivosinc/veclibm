// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(sincospiI, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("sincosI: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0.5;
  x_end = 0.5;
  nb_tests = 30;
  int stride_x = 21;
  int stride_y = 39;
  int stride_z = 17;

  report_err_fp64(rvvlm_sincospiI, sinpil, 1, x_start, x_end, nb_tests,
                  stride_x, stride_y, stride_z);

  report_err_fp64(rvvlm_sincospiI, cospil, 2, x_start, x_end, nb_tests,
                  stride_x, stride_y, stride_z);
}
