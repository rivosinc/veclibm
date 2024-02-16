// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(tanI, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT("sinI: current chosen algorithm; reduced argument in FP64 only")

  x_start = -0.34;
  x_end = 0.34;
  nb_tests = 30;
  int stride_x = 21;
  int stride_y = 39;
  report_err_fp64(rvvlm_tanI, tanl, x_start, x_end, nb_tests, stride_x,
                  stride_y);

  return 0;
}
