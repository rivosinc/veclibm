// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_21
#define STRIDE GENERAL_STRIDE

#include RVVLM_POWDI_VSET_CONFIG

#include "rvvlm_powD.inc.h"
