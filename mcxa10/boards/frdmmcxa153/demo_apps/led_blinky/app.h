/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

#include "fsl_gpio.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define BOARD_IS_FRDM (0)
#define BOARD_IS_EVB  (1)
   
#if BOARD_IS_FRDM
#define BOARD_LED_GPIO     BOARD_LED_RED_GPIO
#define BOARD_LED_PORT     PORT3
#define BOARD_LED_GPIO_PIN BOARD_LED_RED_GPIO_PIN
#elif BOARD_IS_EVB
#define BOARD_LED_GPIO     GPIO2
#define BOARD_LED_PORT     PORT2
#define BOARD_LED_GPIO_PIN 13U
#endif
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardwareFRDM(void);
void BOARD_InitHardwareEVB(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
