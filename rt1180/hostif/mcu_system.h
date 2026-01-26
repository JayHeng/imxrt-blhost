/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCU_SYSTEM_H_
#define MCU_SYSTEM_H_

#include "MIMXRT1189_cm33.h"

#if defined(__cplusplus)
extern "C" {
#endif

void mcu_sleep(uint32_t milliseconds);

#if defined(__cplusplus)
}
#endif
#endif