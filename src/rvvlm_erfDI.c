// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>
#include <stdio.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE GENERAL_STRIDE

#include RVVLM_ERFDI_VSET_CONFIG

#define COMPILE_FOR_ERF
#include "rvvlm_errorfuncsD.h"

#include "rvvlm_erfD.inc.h"
