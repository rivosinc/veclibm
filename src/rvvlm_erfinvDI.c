// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>
#include <stdio.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE GENERAL_STRIDE

#include RVVLM_ERFINVDI_VSET_CONFIG

#define COMPILE_FOR_ERFINV
#include "rvvlm_inverrorfuncsD.h"

#include "rvvlm_erfinvD.inc.h"
