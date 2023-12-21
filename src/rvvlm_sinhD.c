// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>
#include <stdio.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE UNIT_STRIDE

#include RVVLM_SINHD_VSET_CONFIG

#define COMPILE_FOR_SINH
#include "rvvlm_hyperbolicsD.h"

#include "rvvlm_sinhcoshD.inc.h"
