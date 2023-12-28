// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <math.h>
#include <stdio.h>

#include "rvvlm.h"
#include "test_infra.h"

#define COMMENT(comment)                                                       \
  { printf("\n=====\t" comment "\n"); }

int main() {
  unsigned long nb_x, nb_y;
  double x_start, x_end, y_start, y_end;
  int swap_xy = 1;

  COMMENT("atan2pi: current chosen algorithm; reduced argument in FP64 only")

  show_special2_fp64(rvvlm_atan2pi, swap_xy,
                     "Special Value handling of this function");

  x_start = 0.5;
  x_end = 2.0;
  nb_x = 10;
  y_start = 0x1.0p-20;
  y_end = 0x1.0p-15;
  nb_y = 40;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = 0.5;
  x_end = 2.0;
  nb_x = 10;
  y_start = 0x1.0p-5;
  y_end = 0x1.0p5;
  nb_y = 40;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = -2.0;
  x_end = -0.5;
  nb_x = 10;
  y_start = 0x1.0p-5;
  y_end = 0x1.0p5;
  nb_y = 40;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  x_start = -10.0;
  x_end = -10.0;
  nb_x = 10;
  y_start = -1.0e3;
  y_end = 1.0e3;
  nb_y = 400000;
  report_err2_fp64(rvvlm_atan2pi, atan2pil, x_start, x_end, nb_x, y_start,
                   y_end, nb_y, swap_xy);

  return 0;
}
