// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE UNIT_STRIDE

#include RVVLM_ACOSPID_VSET_CONFIG

#define COMPILE_FOR_ACOSPI

#include "rvvlm_asincosD.inc.h"
