// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>
#include <stdio.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE GENERAL_STRIDE

#include RVVLM_TANPIDI_VSET_CONFIG

#define COMPILE_FOR_TANPI
#include "rvvlm_trigD.h"

#include "rvvlm_tanD.inc.h"
