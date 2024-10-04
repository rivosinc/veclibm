// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE GENERAL_STRIDE

#include RVVLM_TGAMMADI_VSET_CONFIG

#include "rvvlm_gammafuncsD.h"

#include "rvvlm_tgammaD.inc.h"
