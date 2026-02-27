/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*${header:start}*/
#include "pin_mux.h"
#include "peripherals.h"
#include "fsl_clock.h"
#include "fsl_reset.h"
#include "board.h"
#include "app.h"
#include <stdbool.h>
/*${header:end}*/

/*${function:start}*/
void BOARD_InitHardwareFRDM(void)
{
    BOARD_InitPinsFRDM();
    BOARD_BootClockFRO12M();
    BOARD_InitBootPeripherals();

    LED_RED_INIT(LOGIC_LED_OFF);
}

// X-MCXA10-48LQFP
void BOARD_InitHardwareEVB(void)
{
    BOARD_InitPinsEVB();
    BOARD_BootClockFRO12M();
    BOARD_InitBootPeripherals();
    
    GPIO_PinWrite(BOARD_LED_GPIO, BOARD_LED_GPIO_PIN, 1U);
    BOARD_LED_GPIO->PDDR |= (1U << BOARD_LED_GPIO_PIN);
}
/*${function:end}*/
