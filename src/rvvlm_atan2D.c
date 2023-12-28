// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>
#include <stdio.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_21
#define STRIDE UNIT_STRIDE

#include RVVLM_ATAN2D_VSET_CONFIG

#define COMPILE_FOR_ATAN2

#include "rvvlm_atan2D.inc.h"
