/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t s_maxCount = 0;
   
/*******************************************************************************
 * Code
 ******************************************************************************/
void SysTick_Handler(void)
{
    s_maxCount++;
    if (s_maxCount == 5)
    {
        /* Toggle pin connected to LED */
        GPIO_PortToggle(BOARD_LED_GPIO, 1u << BOARD_LED_GPIO_PIN);
        s_maxCount = 0;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin init */
#if BOARD_IS_FRDM
    BOARD_InitHardwareFRDM();
#elif BOARD_IS_EVB
    BOARD_InitHardwareEVB();
#endif

    while (1)
    {
    }
}
