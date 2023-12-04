// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "rvvlm.h"
#include "test_infra.h"

#define COMMENT(comment)                                                       \
  { printf("\n=====\t" comment "\n"); }

int main() {
  unsigned long nb_tests, nb_pts_x, nb_pts_y;
  double x_start, x_end, y_start, y_end;
  double target_start, target_end, delta, target;
  unsigned long nb_targets;
  bool swap_xy = 0;

  COMMENT("pow: current chosen algorithm; reduced argument in FP64 only")

  show_special2_fp64(rvvlm_pow, "Special Value handling of this function");

  x_start = 0x1.0p-3;
  x_end = 0x1.0p4;
  nb_pts_x = 300;
  y_start = 0x1.0p-3;
  y_end = 0x1.0p4;
  nb_pts_y = 300;
  report_err2_fp64(rvvlm_pow, powl, x_start, x_end, nb_pts_x, y_start, y_end,
                   nb_pts_y, swap_xy);

  nb_targets = 10;
  x_start = 0x1.0p-10;
  x_end = 0x1.0p10;
  ;
  nb_tests = 40000;

  target_start = -1070.0 * log(2.0);
  target_end = -1020.0 * log(2.0);
  delta = (target_end - target_start) / (double)(nb_targets - 1);
  for (int j = 0; j < nb_targets; j++) {
    target = target_start + (double)j * delta;
    report_err_pow_fp64(rvvlm_pow, powl, target, x_start, x_end, nb_tests);
  }

  target_start = -32.0 * log(2.0);
  target_end = 32.0 * log(2.0);
  delta = (target_end - target_start) / (double)(nb_targets - 1);
  for (int j = 0; j < nb_targets; j++) {
    target = target_start + (double)j * delta;
    report_err_pow_fp64(rvvlm_pow, powl, target, x_start, x_end, nb_tests);
  }

  target_start = 1010.0 * log(2.0);
  target_end = 1023.5 * log(2.0);
  delta = (target_end - target_start) / (double)(nb_targets - 1);
  for (int j = 0; j < nb_targets; j++) {
    target = target_start + (double)j * delta;
    report_err_pow_fp64(rvvlm_pow, powl, target, x_start, x_end, nb_tests);
  }

  target_start = -100.0 * log(2.0);
  target_end = -80.0 * log(2.0);
  delta = (target_end - target_start) / (double)(nb_targets - 1);
  for (int j = 0; j < nb_targets; j++) {
    target = target_start + (double)j * delta;
    report_err_pow_fp64(rvvlm_pow, powl, target, x_start, x_end, nb_tests);
  }

  return 0;
}
