// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>
#include <stdbool.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(powI, test) {
  unsigned long nb_pts_x, nb_pts_y, stride_x, stride_y, stride_z;
  double x_start, x_end, y_start, y_end;
  bool swap_xy;

  COMMENT("powI: current chosen algorithm; reduced argument in FP64 only")

  x_start = 1.0;
  x_end = 10.0;
  y_start = -3.0;
  y_end = 3.0;
  nb_pts_x = 6;
  nb_pts_y = 7;
  stride_x = 21;
  stride_y = 39;
  stride_z = 11;
  swap_xy = 0;
  report_err2_fp64(rvvlm_powI, powl, x_start, x_end, nb_pts_x, stride_x,
                   y_start, y_end, nb_pts_y, stride_y, stride_z, swap_xy);
}
