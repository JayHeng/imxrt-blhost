/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"   
#include "fsl_clock.h"

void mcu_sleep(uint32_t microseconds)
{
    SDK_DelayAtLeastUs(microseconds, CLOCK_GetFreq(kCLOCK_CoreSysClk));
}