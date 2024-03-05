// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(atan2piI, test) {
  unsigned long nb_x, nb_y;
  double x_start, x_end, y_start, y_end;
  int swap_xy = 1;

  COMMENT("atan2piI: current chosen algorithm; reduced argument in FP64 only")

  x_start = -11.0;
  x_end = 20.0;
  nb_x = 8;
  y_start = 0.8;
  y_end = 10.4;
  nb_y = 10;

  int stride_x = 21;
  int stride_y = 39;
  int stride_z = 17;

  report_err2_fp64(rvvlm_atan2piI, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, stride_x, stride_y, stride_z, swap_xy);
}
