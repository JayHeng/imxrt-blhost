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

#define BLHOST_I2C_ARGC0 (6)
char *blhost_i2c_args0[] = {
    "blhost",
    "-i",           // -i means i2c
    "2,0x10",       // means dev/i2c-2,0x10(,100kHz)
    "--",
    "get-property",
    "1"
};

#define BLHOST_I2C_ARGC1 (8)
char *blhost_i2c_args1[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "flash-erase-region",
    "0x08010000",
    "0x10000",
    "0x9"
};

#define BLHOST_I2C_ARGC2 (8)
char *blhost_i2c_args2[] = {
    "blhost",
    "-i",
    "5,4000",
    "--",
    "write-memory",
    "0x08010000",
    "0x200000",
    "0x4000"
};

#define BLHOST_I2C_ARGC3 (8)
char *blhost_i2c_args3[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "execute",
    "0x00082D09",
    "0x0",
    "0x20200000"
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int ota_main(uint8_t tgtIdx)
{
    PRINTF("BLHOST.\r\n");
    blhost_main(BLHOST_I2C_ARGC0, blhost_i2c_args0, NULL);
    //blhost_main(BLHOST_I2C_ARGC1, blhost_i2c_args1, NULL);
    //blhost_main(BLHOST_I2C_ARGC2, blhost_i2c_args2, NULL);
    //blhost_main(BLHOST_I2C_ARGC3, blhost_i2c_args3, NULL);
    PRINTF("Done\r\n");
    return 0;
}
