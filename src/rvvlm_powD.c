// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>
#include <stdio.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_21
#define STRIDE UNIT_STRIDE

#include RVVLM_POWD_VSET_CONFIG

#include "rvvlm_powD.inc.h"
