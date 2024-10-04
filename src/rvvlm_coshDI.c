// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include <riscv_vector.h>

#include "rvvlm.h"
#define API_SIGNATURE API_SIGNATURE_11
#define STRIDE GENERAL_STRIDE

#include RVVLM_COSHDI_VSET_CONFIG

#define COMPILE_FOR_COSH
#include "rvvlm_hyperbolicsD.h"

#include "rvvlm_sinhcoshD.inc.h"
