// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE UNIT_STRIDE

#include RVVLM_COSD_VSET_CONFIG

#define COMPILE_FOR_COS
#include "rvvlm_trigD.h"

#include "rvvlm_sincosD.inc.h"
