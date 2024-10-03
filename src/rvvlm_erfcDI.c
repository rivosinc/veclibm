// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE GENERAL_STRIDE

#include RVVLM_ERFCDI_VSET_CONFIG

#define COMPILE_FOR_ERFC
#include "rvvlm_errorfuncsD.h"

#include "rvvlm_erfcD.inc.h"
