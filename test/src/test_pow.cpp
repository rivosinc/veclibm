// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <math.h>
#include <stdbool.h>

#include "rvvlm.h"
#include "test_infra.h"

#define NB_TV_POW 60
#define DBL_NEG_171 (int64_t)0xc065600000000000
#define DBL_NEG_888 (int64_t)0xc08bc00000000000
#define DBL_NEG_NONINT (int64_t)0xc052466666666666
#define DBL_POS_171 (int64_t)0x4065600000000000
#define DBL_POS_888 (int64_t)0x408bc00000000000

TEST(pow, special) {
  int nb_tv = NB_TV_POW;
  int swap_xy = 0;
  int64_t pow_tv[3 * NB_TV_POW] = {
      DBL_POS_ZERO,   DBL_NEG_171,  DBL_POS_INF,  DBL_NEG_ZERO, DBL_NEG_171,
      DBL_NEG_INF,    DBL_POS_ZERO, DBL_NEG_888,  DBL_POS_INF,  DBL_NEG_ZERO,
      DBL_NEG_NONINT, DBL_POS_INF,  DBL_POS_ZERO, DBL_NEG_INF,  DBL_POS_INF,
      DBL_NEG_ZERO,   DBL_NEG_INF,  DBL_POS_INF,  DBL_POS_ZERO, DBL_NEG_INF,
      DBL_POS_INF,    DBL_NEG_ZERO, DBL_NEG_INF,  DBL_POS_INF,  DBL_NEG_ONE,
      DBL_POS_INF,    DBL_POS_ONE,  DBL_NEG_ONE,  DBL_NEG_INF,  DBL_POS_ONE,
      DBL_POS_ONE,    DBL_POS_GT1,  DBL_POS_ONE,  DBL_POS_ONE,  DBL_POS_INF,
      DBL_POS_ONE,    DBL_POS_ONE,  DBL_NEG_INF,  DBL_POS_ONE,  DBL_POS_ONE,
      DBL_QNAN,       DBL_POS_ONE,  DBL_POS_ONE,  DBL_SNAN,     DBL_POS_ONE,
      DBL_POS_INF,    DBL_POS_ZERO, DBL_POS_ONE,  DBL_NEG_INF,  DBL_POS_ZERO,
      DBL_POS_ONE,    DBL_POS_GT1,  DBL_POS_ZERO, DBL_POS_ONE,  DBL_POS_INF,
      DBL_POS_ZERO,   DBL_POS_ONE,  DBL_NEG_INF,  DBL_POS_ZERO, DBL_POS_ONE,
      DBL_QNAN,       DBL_POS_ZERO, DBL_POS_ONE,  DBL_SNAN,     DBL_POS_ZERO,
      DBL_POS_ONE,    DBL_POS_INF,  DBL_NEG_ZERO, DBL_POS_ONE,  DBL_NEG_INF,
      DBL_NEG_ZERO,   DBL_POS_ONE,  DBL_POS_GT1,  DBL_NEG_ZERO, DBL_POS_ONE,
      DBL_POS_INF,    DBL_NEG_ZERO, DBL_POS_ONE,  DBL_NEG_INF,  DBL_NEG_ZERO,
      DBL_POS_ONE,    DBL_QNAN,     DBL_NEG_ZERO, DBL_POS_ONE,  DBL_SNAN,
      DBL_NEG_ZERO,   DBL_POS_ONE,  DBL_NEG_GT1,  DBL_POS_GT1,  DBL_QNAN,
      DBL_NEG_LT1,    DBL_NEG_INF,  DBL_POS_INF,  DBL_POS_LT1,  DBL_NEG_INF,
      DBL_POS_INF,    DBL_NEG_GT1,  DBL_NEG_INF,  DBL_POS_ZERO, DBL_POS_GT1,
      DBL_NEG_INF,    DBL_POS_ZERO, DBL_NEG_LT1,  DBL_POS_INF,  DBL_POS_ZERO,
      DBL_POS_LT1,    DBL_POS_INF,  DBL_POS_ZERO, DBL_NEG_GT1,  DBL_POS_INF,
      DBL_POS_INF,    DBL_POS_GT1,  DBL_POS_INF,  DBL_POS_INF,  DBL_NEG_INF,
      DBL_NEG_171,    DBL_NEG_ZERO, DBL_NEG_INF,  DBL_NEG_888,  DBL_POS_ZERO,
      DBL_NEG_INF,    DBL_NEG_GT1,  DBL_POS_ZERO, DBL_NEG_INF,  DBL_POS_171,
      DBL_NEG_INF,    DBL_NEG_INF,  DBL_POS_888,  DBL_POS_INF,  DBL_NEG_INF,
      DBL_POS_GT1,    DBL_POS_INF,  DBL_POS_INF,  DBL_NEG_171,  DBL_POS_ZERO,
      DBL_POS_INF,    DBL_NEG_888,  DBL_POS_ZERO, DBL_POS_INF,  DBL_NEG_GT1,
      DBL_POS_ZERO,   DBL_QNAN,     DBL_POS_LT1,  DBL_QNAN,     DBL_SNAN,
      DBL_POS_GT1,    DBL_QNAN,     DBL_QNAN,     DBL_NEG_LT1,  DBL_QNAN,
      DBL_SNAN,       DBL_POS_LT1,  DBL_QNAN,     DBL_QNAN,     DBL_POS_TINY,
      DBL_QNAN,       DBL_SNAN,     DBL_NEG_TINY, DBL_QNAN,     DBL_POS_LT1,
      DBL_QNAN,       DBL_QNAN,     DBL_POS_GT1,  DBL_SNAN,     DBL_QNAN,
      DBL_NEG_LT1,    DBL_QNAN,     DBL_QNAN,     DBL_POS_LT1,  DBL_SNAN,
      DBL_QNAN,       DBL_POS_TINY, DBL_QNAN,     DBL_QNAN,     DBL_NEG_TINY,
      DBL_SNAN,       DBL_QNAN,     DBL_POS_GT1,  DBL_POS_INF,  DBL_POS_INF};
  test_vectors2_fp64(rvvlm_pow, pow_tv, nb_tv, swap_xy);
}

TEST(pow, medium_args) {
  unsigned long nb_tests, nb_pts_x, nb_pts_y;
  double x_start, x_end, y_start, y_end;
  double target_start, target_end, delta, target;
  unsigned long nb_targets;
  bool swap_xy = 0;

  COMMENT("pow: current chosen algorithm; reduced argument in FP64 only")

  x_start = 0x1.0p-3;
  x_end = 0x1.0p4;
  nb_pts_x = 200;
  y_start = 0x1.0p-3;
  y_end = 0x1.0p4;
  nb_pts_y = 200;
  report_err2_fp64(rvvlm_pow, powl, x_start, x_end, nb_pts_x, y_start, y_end,
                   nb_pts_y, swap_xy);

  nb_targets = 5;
  nb_tests = 1000;

  target_start = -32.0 * log(2.0);
  target_end = 32.0 * log(2.0);
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
}

TEST(pow, large_args) {
  unsigned long nb_tests, nb_pts_x, nb_pts_y;
  double x_start, x_end, y_start, y_end;
  double target_start, target_end, delta, target;
  unsigned long nb_targets;
  bool swap_xy = 0;

  COMMENT("pow: current chosen algorithm; reduced argument in FP64 only")

  nb_targets = 5;
  nb_tests = 1000;

  x_start = 0x1.0p-10;
  x_end = 0x1.0p10;

  target_start = -1070.0 * log(2.0);
  target_end = -1020.0 * log(2.0);
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

  x_start = 0x1.0p0 - 0x1.0p-40;
  x_end = 0x1.0p0 + 0x1.0p-40;

  target_start = 1010.0 * log(2.0);
  target_end = 1023.5 * log(2.0);
  delta = (target_end - target_start) / (double)(nb_targets - 1);
  for (int j = 0; j < nb_targets; j++) {
    target = target_start + (double)j * delta;
    report_err_pow_fp64(rvvlm_pow, powl, target, x_start, x_end, nb_tests);
  }
}
