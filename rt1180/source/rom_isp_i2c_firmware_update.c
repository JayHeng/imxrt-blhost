/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "fsl_debug_console.h"
#include "fsl_rgpio.h"
#include "rom_isp_i2c_firmware_update.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* LPI2C buffers */
static uint8_t g_i2c_master_txBuff[18];
static uint8_t g_i2c_master_rxBuff[16];

/*******************************************************************************
 * Code
 ******************************************************************************/

void ROM_ISP_InitUserI2CPins(void)
{
    PRINTF("\r\nSwitching pins from I3C to LPI2C...\r\n");

    CLOCK_EnableClock(kCLOCK_Iomuxc2);          /* Turn on LPCG: LPCG is ON. */


    IOMUXC_SetPinMux(
        IOMUXC_GPIO_AON_16_LPI2C2_SCL,          /* GPIO_AON_16 is configured as LPI2C2_SCL */
        1U);                                    /* Software Input On Field: Input Path is determined by functionality */
    IOMUXC_SetPinMux(
    	IOMUXC_GPIO_AON_15_LPI2C2_SDA,          /* GPIO_AON_15 is configured as LPI2C2_SDA */
        1U);                                    /* Software Input On Field: Input Path is determined by functionality */

    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_AON_16_LPI2C2_SCL,           /* GPIO_AON_16 PAD functional properties : */
        0x1EU);                                 /* Slew Rate Field: Fast Slew Rate
                                                   Drive Strength Field: high driver
                                                   Pull / Keep Select Field: Pull Enable
                                                   Pull Up / Down Config. Field: Weak pull up
                                                   Open Drain Field: Enabled
                                                   Force ibe off Field: Disabled */
    IOMUXC_SetPinConfig(
    	IOMUXC_GPIO_AON_15_LPI2C2_SDA,           /* GPIO_AON_15 PAD functional properties : */
        0x1EU);                                 /* Slew Rate Field: Fast Slew Rate
                                                   Drive Strength Field: high driver
                                                   Pull / Keep Select Field: Pull Enable
                                                   Pull Up / Down Config. Field: Weak pull up
                                                   Open Drain Field: Enabled
                                                   Force ibe off Field: Disabled */

    PRINTF("Pins switched to LPI2C successfully\r\n");
}
        
void ROM_ISP_InitUserI2CPins_deinit(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc2);          /* Turn on LPCG: LPCG is ON. */

  rgpio_pin_config_t gpio_pin_config = {
      .pinDirection = kRGPIO_DigitalInput,
      .outputLogic = 0U,
  };
  /* Initialize GPIO functionality on GPIO_AD_18 (pin L13) */
  RGPIO_PinInit(RGPIO1, 15U, &gpio_pin_config);
  RGPIO_SetPinInterruptConfig(RGPIO1, 15U, kRGPIO_InterruptOutput0, kRGPIO_InterruptOrDMADisabled);
  RGPIO_PinInit(RGPIO1, 16U, &gpio_pin_config);
  RGPIO_SetPinInterruptConfig(RGPIO1, 16U, kRGPIO_InterruptOutput0, kRGPIO_InterruptOrDMADisabled);

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_16_GPIO1_IO16,
      0U);
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_15_GPIO1_IO15,
      0U);
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_16_GPIO1_IO16,
      0x08U);
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_15_GPIO1_IO15,
      0x08U);
}

status_t ROM_ISP_LPI2C_MasterInitialize(void)
{
    lpi2c_master_config_t masterConfig;

    PRINTF("\r\nLPI2C Master Initialization\r\n");

    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = LPI2C_BAUDRATE;
    LPI2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig, LPI2C_MASTER_CLOCK_FREQUENCY);

    PRINTF("LPI2C master initialized successfully\r\n");
    return kStatus_Success;
}

status_t ROM_ISP_I2C_FirmwareUpdate(void)
{
    status_t reVal;
    size_t txCount = 0xFFU;

    PRINTF("\r\n========================================\r\n");
    PRINTF("Phase 3: LPI2C Communication\r\n");
    PRINTF("========================================\r\n");

    /* Transaction 1: Write 2 bytes (0x5A, 0xA6) */
    PRINTF("\r\n--- LPI2C Write Transaction ---\r\n");
    g_i2c_master_txBuff[0] = 0x5A;
    g_i2c_master_txBuff[1] = 0xA6;

    if (kStatus_Success == LPI2C_MasterStart(EXAMPLE_I2C_MASTER, LPI2C_MASTER_SLAVE_ADDR_7BIT, kLPI2C_Write))
    {
        LPI2C_MasterGetFifoCounts(EXAMPLE_I2C_MASTER, NULL, &txCount);
        while (txCount)
        {
            LPI2C_MasterGetFifoCounts(EXAMPLE_I2C_MASTER, NULL, &txCount);
        }

        if (LPI2C_MasterGetStatusFlags(EXAMPLE_I2C_MASTER) & kLPI2C_MasterNackDetectFlag)
        {
            PRINTF("LPI2C Write NACK\r\n");
            return kStatus_LPI2C_Nak;
        }

        reVal = LPI2C_MasterSend(EXAMPLE_I2C_MASTER, g_i2c_master_txBuff, LPI2C_DATA_LENGTH);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            PRINTF("LPI2C Write Failed\r\n");
            return reVal;
        }

        reVal = LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
        if (reVal != kStatus_Success)
        {
            PRINTF("LPI2C Write Stop Failed\r\n");
            return reVal;
        }

        PRINTF("LPI2C Write OK: 0x%02X 0x%02X\r\n",
               g_i2c_master_txBuff[0], g_i2c_master_txBuff[1]);
    }
    else
    {
        PRINTF("LPI2C Write Start Failed\r\n");
        return kStatus_Fail;
    }

    SDK_DelayAtLeastUs(5000, SystemCoreClock);

    /* Transaction 2: Read 11 bytes */
    PRINTF("\r\n--- LPI2C Read Transaction ---\r\n");
    memset(g_i2c_master_rxBuff, 0, sizeof(g_i2c_master_rxBuff));

    if (kStatus_Success == LPI2C_MasterStart(EXAMPLE_I2C_MASTER, LPI2C_MASTER_SLAVE_ADDR_7BIT, kLPI2C_Read))
    {
        reVal = LPI2C_MasterReceive(EXAMPLE_I2C_MASTER, g_i2c_master_rxBuff, LPI2C_READ_LENGTH);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            PRINTF("LPI2C Read Failed\r\n");
            return reVal;
        }

        reVal = LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
        if (reVal != kStatus_Success)
        {
            PRINTF("LPI2C Read Stop Failed\r\n");
            return reVal;
        }

        PRINTF("LPI2C Read OK - Received %d bytes:\r\n", LPI2C_READ_LENGTH);
        PRINTF("Data: ");
        for (uint8_t i = 0; i < LPI2C_READ_LENGTH; i++)
        {
            PRINTF("0x%02X ", g_i2c_master_rxBuff[i]);
        }
        PRINTF("\r\n");
    }
    else
    {
        PRINTF("LPI2C Read Start Failed\r\n");
        return kStatus_Fail;
    }

    SDK_DelayAtLeastUs(2000, SystemCoreClock);

    PRINTF("\r\n========================================\r\n");
    PRINTF("LPI2C Firmware Update Completed!\r\n");
    PRINTF("- Write: 2 bytes\r\n");
    PRINTF("- Read : %d bytes\r\n", LPI2C_READ_LENGTH);
    PRINTF("========================================\r\n");

    return kStatus_Success;
}
