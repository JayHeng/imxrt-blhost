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

#define BLHOST_I2C_ARGC1 (5)
char *blhost_i2c_args1[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "flash-erase-all"
};

#define BLHOST_I2C_ARGC2 (8)
char *blhost_i2c_args2[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "write-memory",
    "0x0",
    "0x200000",
    "0x10000"
};

#define BLHOST_I2C_ARGC3 (8)
char *blhost_i2c_args3[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "execute",
    "0x000012c9",
    "0x0",
    "0x20006000"
};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t ROM_ISP_LPI2C_MasterInitialize(void);
status_t ROM_ISP_I2C_FirmwareUpdate(void);
/*!
 * @brief Main function
 */
//int ota_main(uint8_t tgtIdx)
int main(void)
{
    /* Init board hardware. */
    BOARD_InitHardware();
    PRINTF("BLHOST.\r\n");

    /*
    {
        status_t result;
        // Initialize LPI2C master
        result = ROM_ISP_LPI2C_MasterInitialize();
        if (result != kStatus_Success)
        {
            PRINTF("LPI2C master initialization failed\r\n");
            return -1;
        }

        // Perform firmware update via LPI2C
        result = ROM_ISP_I2C_FirmwareUpdate();
        if (result != kStatus_Success)
        {
            PRINTF("LPI2C firmware update failed\r\n");
            return -1;
        }
    }
    */

    blhost_main(BLHOST_I2C_ARGC0, blhost_i2c_args0, NULL);
    blhost_main(BLHOST_I2C_ARGC1, blhost_i2c_args1, NULL);
    blhost_main(BLHOST_I2C_ARGC2, blhost_i2c_args2, NULL);
    blhost_main(BLHOST_I2C_ARGC3, blhost_i2c_args3, NULL);
    PRINTF("Done\r\n");
    return 0;
}
