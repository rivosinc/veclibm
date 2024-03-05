// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>

#include "rvvlm.h"
#include "test_infra.h"

TEST(atan2pi, test) {
  unsigned long nb_x, nb_y;
  double x_start, x_end, y_start, y_end;
  int swap_xy = 1;

  COMMENT("atan2pi: current chosen algorithm; reduced argument in FP64 only")

  show_special2_fp64(rvvlm_atan2pi, swap_xy,
                     "Special Value handling of this function");

  x_start = 0x1.000000001p0;
  x_end = 0x1.ffffffffffp0;
  nb_x = 8;
  y_start = 0x1.01p0;
  y_end = 0x1.fffp0;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = 0x1.000000001p0;
  x_end = 0x1.ffffffffffp0;
  nb_x = 8;
  y_start = 0x1.01p1020;
  y_end = 0x1.ffffffffp1020;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = 0x1.000000001p0;
  x_end = 0x1.ffffffffffp0;
  nb_x = 8;
  y_start = 0x1.01p-1020;
  y_end = 0x1.ffffffffp-1020;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = -0x1.000000001p0;
  x_end = -0x1.ffffffffffp0;
  nb_x = 8;
  y_start = 0x1.01p0;
  y_end = 0x1.fffp0;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = -0x1.000000001p0;
  x_end = -0x1.ffffffffffp0;
  nb_x = 8;
  y_start = 0x1.01p1020;
  y_end = 0x1.ffffffffp1020;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = -0x1.000000001p0;
  x_end = -0x1.ffffffffffp0;
  nb_x = 8;
  y_start = 0x1.01p-1020;
  y_end = 0x1.ffffffffp-1020;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = 0x1.000000001p0;
  x_end = 0x1.ffffffffffp0;
  nb_x = 8;
  y_start = -0x1.01p0;
  y_end = -0x1.fffp0;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = 0x1.000000001p0;
  x_end = 0x1.ffffffffffp0;
  nb_x = 8;
  y_start = -0x1.01p1020;
  y_end = -0x1.ffffffffp1020;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = 0x1.000000001p0;
  x_end = 0x1.ffffffffffp0;
  nb_x = 8;
  y_start = -0x1.01p-1020;
  y_end = -0x1.ffffffffp-1020;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = -0x1.000000001p0;
  x_end = -0x1.ffffffffffp0;
  nb_x = 8;
  y_start = -0x1.01p0;
  y_end = -0x1.fffp0;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = -0x1.000000001p0;
  x_end = -0x1.ffffffffffp0;
  nb_x = 8;
  y_start = -0x1.01p1020;
  y_end = -0x1.ffffffffp1020;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = -0x1.000000001p0;
  x_end = -0x1.ffffffffffp0;
  nb_x = 8;
  y_start = -0x1.01p-1020;
  y_end = -0x1.ffffffffp-1020;
  nb_y = 1000000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);
}
