/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
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
#define BLHOST_ARGC0 (6)
char *blhost_args0[] = {
    "blhost",
    "-s",  // -s means spi
    "5,4000",
    "--",
    "get-property",
    "1"
};

#define BLHOST_ARGC1 (7)
char *blhost_args1[] = {
    "blhost",
    "-s",
    "5,4000",
    "--",
    "set-property",
    "0x1C",
    "0x80000110"
};

#define BLHOST_ARGC2 (8)
char *blhost_args2[] = {
    "blhost",
    "-s",
    "5,4000",
    "--",
    "write-memory",
    "0x80000",
    "0x200000",
    "0x80000"
};

#ifdef RT700_BLINKY_IMAGE
#define BLHOST_ARGC3 (8)
/* RT700 IMAGE */
char *blhost_args3[] = {
    "blhost",
    "-s",
    "5,4000",
    "--",
    "execute",
    "0x8238D",
    "0x80000",
    "0x20200000"
};
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    char ch;

    gpio_pin_config_t gpio_pin_config;
    gpio_pin_config.pinDirection = kGPIO_DigitalOutput;
    gpio_pin_config.outputLogic = 1;
	
    GPIO_PortInit(GPIO, 0);
    GPIO_PinInit(GPIO, 0, 6, &gpio_pin_config);

    /* Init board hardware. */
    BOARD_InitHardware();

    PRINTF("BLHOST.\r\n");

    blhost_main(BLHOST_ARGC0, blhost_args0, NULL);
//    blhost_main(BLHOST_ARGC1, blhost_args1, NULL);
    blhost_main(BLHOST_ARGC2, blhost_args2, NULL);
#ifdef RT700_BLINKY_IMAGE
    blhost_main(BLHOST_ARGC3, blhost_args3, NULL);
#endif
    PRINTF("Done\r\n");
    while (1)
    {

    }
}
