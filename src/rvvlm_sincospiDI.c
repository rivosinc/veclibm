// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_12
#define STRIDE GENERAL_STRIDE

#include RVVLM_SINCOSPIDI_VSET_CONFIG

#define COMPILE_FOR_SINCOSPI
#include "rvvlm_trigD.h"

#include "rvvlm_sinandcosD.inc.h"
