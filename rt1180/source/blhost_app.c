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
#include <stdio.h>
#include <stdint.h>
   
#if (defined(__ICCARM__))
#pragma section = ".intvec"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_TARGET_FLASH_SIZE (128*1024UL)

#define SBL_MAGIC (0x4C425354) //'TSBL'

typedef struct _tota_sbl_header
{
    uint32_t reserved[8];
    uint32_t slot0StartAddr;
    uint32_t slot1StartAddr;
    uint32_t magic;
} tota_sbl_header_t;

#define APP_MAGIC (0x50504154) //'TAPP'

typedef struct _tota_app_header
{
    uint32_t reserved0[8];
    uint32_t length;
    uint16_t authType;
    uint16_t version;
    uint32_t authRes;
    uint32_t reserved1[2];
    uint32_t appLoadAddr;
} tota_app_header_t;

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
    "0x04080000",
    "0x10000"
};

#define BLHOST_I2C_ARGC3 (8)
static char s_pc_buf[12];
static char s_sp_buf[12];

static char *blhost_i2c_args3[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "execute",
    s_pc_buf,
    "0x0",
    s_sp_buf
};

/*******************************************************************************
 * Code
 ******************************************************************************/

void update_blhost_args_pc_sp(uint32_t pc, uint32_t sp)
{
    snprintf(s_pc_buf, sizeof(s_pc_buf), "0x%08X", pc);
    snprintf(s_sp_buf, sizeof(s_sp_buf), "0x%08X", sp);
}

status_t ROM_ISP_LPI2C_MasterInitialize(void);
status_t ROM_ISP_I2C_FirmwareUpdate(void);
/*!
 * @brief Main function
 */
uint8_t g_appIndex = 0;
uint32_t g_appStart[SLAVE_COUNT];
uint32_t g_appSize[SLAVE_COUNT];

void ota_prepare(void)
{
    uint32_t vectorStart = (uint32_t)__section_begin(".intvec");
    PRINTF("OTA app vector addr = 0x%x.\r\n", vectorStart);
    tota_sbl_header_t *sblHeader = (tota_sbl_header_t *)vectorStart;
    if (sblHeader->magic == SBL_MAGIC)
    {
        PRINTF("slot 0 start: 0x%x \r\n", sblHeader->slot0StartAddr);
        for (uint32_t i = 0; i < SLAVE_COUNT; i++)
        {
            g_appStart[i] = FlexSPI2_AMBA_BASE + sblHeader->slot0StartAddr + i * APP_TARGET_FLASH_SIZE;
            PRINTF("TARGET app%d vector addr = 0x%x.\r\n", i, g_appStart[i]);
            tota_app_header_t *appHeader = (tota_app_header_t *)g_appStart[i];
            g_appSize[i] = appHeader->length;
            PRINTF("TARGET app%d length (bytes) = 0x%x.\r\n", i, g_appSize[i]);
        }
        PRINTF("slot 1 start: 0x%x \r\n", sblHeader->slot1StartAddr);
    }
    else
    {
        PRINTF("OTA app doesn't contain magic.\r\n");
    }
}

int ota_main(uint8_t tgtIdx)
{
    status_t status = kStatus_Success;
    PRINTF("BLHOST.\r\n");
    g_appIndex = tgtIdx;

    uint32_t sp = *((uint32_t *)g_appStart[tgtIdx]);
    uint32_t pc = *((uint32_t *)(g_appStart[tgtIdx] + 4));

    status = blhost_main(BLHOST_I2C_ARGC0, blhost_i2c_args0, NULL);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = blhost_main(BLHOST_I2C_ARGC1, blhost_i2c_args1, NULL);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = blhost_main(BLHOST_I2C_ARGC2, blhost_i2c_args2, NULL);
    if (status != kStatus_Success)
    {
        return status;
    }
    //pc = 0x000012c9;
    //sp = 0x20006000;
    update_blhost_args_pc_sp(pc, sp);
    status = blhost_main(BLHOST_I2C_ARGC3, blhost_i2c_args3, NULL);
    if (status != kStatus_Success)
    {
        return status;
    }
    PRINTF("Done\r\n");

    return kStatus_Success;
}

/*
void BOARD_InitUserI2cPins(void);
int main(void)
{
    // Init board hardware.
    BOARD_InitHardware();
    BOARD_InitUserI2cPins();
    
    ota_prepare();
    for (uint32_t i = 0; i < SLAVE_COUNT; i++)
    {
        ota_main(1);
        SDK_DelayAtLeastUs(5000000, SystemCoreClock);
    }

    while (1);
}
*/

