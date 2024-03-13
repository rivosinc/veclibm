// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(cdfnorminvI, test) {
  unsigned long nb_tests;
  double x_start, x_end;

  COMMENT(
      "cdfnorminvI: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0.25;
  x_end = 0.75;
  nb_tests = 30;
  int stride_x = 21;
  int stride_y = 30;
  report_err_byinv_fp64(rvvlm_cdfnorminvI, cdfnorml, cdfnorml_prime, x_start,
                        x_end, nb_tests, stride_x, stride_y);
}
