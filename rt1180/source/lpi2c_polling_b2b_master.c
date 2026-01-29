/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"
#include "app.h"
#include "fsl_i3c.h"
#include "pin_mux.h"
#include "i3c_master_transfer.h"
#include "rom_isp_i2c_firmware_update.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define EXAMPLE_I2C_MASTER ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)

#define SLAVE_COUNT 5
#define MAX_LOOPS 3
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static status_t I3C_MasterInitialize(void);
static status_t I3C_MasterAssignDynamicAddress(void);
static status_t I3C_MasterSendData(uint8_t slaveAddr, uint8_t *txData, size_t dataSize);
static status_t I3C_MasterReceiveData(uint8_t slaveAddr, uint8_t *rxData, size_t dataSize);
static status_t I3C_NormalCommunicationAndTermination(void);

extern int ota_main(uint8_t tgtIdx);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t i2c_g_master_txBuff[LPI2C_DATA_LENGTH];
uint8_t i2c_g_master_rxBuff[LPI2C_DATA_LENGTH];
AT_NONCACHEABLE_SECTION(i3c_commm_tx_frame g_master_txBuff[I3C_TARGET_NUM]);
AT_NONCACHEABLE_SECTION(i3c_commm_rx_frame g_master_rxBuff[I3C_TARGET_NUM]);
static uint8_t g_slaveAddresses[SLAVE_COUNT] = {0x30U, 0x31U, 0x32U, 0x33U, 0x34U};

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Initialize I3C Master */
static status_t I3C_MasterInitialize(void)
{
    i3c_master_config_t masterConfig;

    PRINTF("\r\nI3C Master Initialization\r\n");

    I3C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz.i2cBaud          = EXAMPLE_I2C_BAUDRATE;
    masterConfig.baudRate_Hz.i3cPushPullBaud  = EXAMPLE_I3C_PP_BAUDRATE;
    masterConfig.baudRate_Hz.i3cOpenDrainBaud = EXAMPLE_I3C_OD_BAUDRATE;
    masterConfig.enableOpenDrainStop          = false;
    I3C_MasterInit(EXAMPLE_MASTER, &masterConfig, I3C_MASTER_CLOCK_FREQUENCY);
    EXAMPLE_MASTER->MIBIRULES |= 1<<30;
    PRINTF("MIBIRULES:%8X\r\n", EXAMPLE_MASTER->MIBIRULES);

    PRINTF("I3C master initialized successfully\r\n");
    return kStatus_Success;
}

static status_t I3C_MasterAssignDynamicAddress(void)
{
    status_t result;
    i3c_master_transfer_t masterXfer;

    PRINTF("Starting Dynamic Address Assignment for %d slaves...\r\n", SLAVE_COUNT);

    /* Reset all dynamic addresses */
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = I3C_BROADCAST_ADDR;
    masterXfer.subaddress     = CCC_RSTDAA;
    masterXfer.subaddressSize = 1U;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI3CSdr;
    masterXfer.flags          = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse    = kI3C_IbiRespAckMandatory;
    result = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
    if (result != kStatus_Success)
    {
        PRINTF("RSTDAA broadcast failed\r\n");
        return result;
    }
    PRINTF("RSTDAA: All dynamic addresses reset\r\n");

    /* Assign dynamic address to each slave */
    for (uint8_t i = 0; i < SLAVE_COUNT; i++)
    {
        /* Send SETDASA command */
        memset(&masterXfer, 0, sizeof(masterXfer));
        masterXfer.slaveAddress   = I3C_BROADCAST_ADDR;
        masterXfer.subaddress     = CCC_SETDASA;
        masterXfer.subaddressSize = 1U;
        masterXfer.direction      = kI3C_Write;
        masterXfer.busType        = kI3C_TypeI3CSdr;
        masterXfer.flags          = kI3C_TransferNoStopFlag;
        masterXfer.ibiResponse    = kI3C_IbiRespAckMandatory;
        result = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
        if (result != kStatus_Success)
        {
            PRINTF("SETDASA command failed for slave[%d]\r\n", i);
            return result;
        }

        /* Assign address */
        memset(&masterXfer, 0, sizeof(masterXfer));
        masterXfer.slaveAddress   = g_slaveAddresses[i];
        masterXfer.subaddress     = g_slaveAddresses[i] << 1U;
        masterXfer.subaddressSize = 1U;
        masterXfer.direction      = kI3C_Write;
        masterXfer.busType        = kI3C_TypeI3CSdr;
        masterXfer.flags          = kI3C_TransferDefaultFlag;
        masterXfer.ibiResponse    = kI3C_IbiRespAckMandatory;
        result = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);

        if (result != kStatus_Success)
        {
            PRINTF("Failed to assign address 0x%02X to slave[%d]\r\n", g_slaveAddresses[i], i);
            return result;
        }

        PRINTF("Slave[%d]: Dynamic address 0x%02X assigned successfully\r\n", i, g_slaveAddresses[i]);
        SDK_DelayAtLeastUs(5000, SystemCoreClock);
    }

    PRINTF("All %d slaves configured successfully\r\n\r\n", SLAVE_COUNT);
    return kStatus_Success;
}

static status_t I3C_MasterSendData(uint8_t slaveAddr, uint8_t *txData, size_t dataSize)
{
    i3c_master_transfer_t masterXfer;

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.data         = txData;
    masterXfer.dataSize     = dataSize;
    masterXfer.direction    = kI3C_Write;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;

    return I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
}

static status_t I3C_MasterReceiveData(uint8_t slaveAddr, uint8_t *rxData, size_t dataSize)
{
    i3c_master_transfer_t masterXfer;

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.data         = rxData;
    masterXfer.dataSize     = dataSize;
    masterXfer.direction    = kI3C_Read;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;

    return I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
}

/**
 * @brief Perform normal I3C communication and send termination command
 *
 * This function performs:
 * - Phase 1: Normal communication loops (3 times)
 * - Phase 2: Send termination command to slave 0x34
 *
 * @return Status of the operation
 */
static status_t I3C_NormalCommunication(void)
{
    status_t result;
    uint32_t loopCount = 0;

    PRINTF("\r\n========================================\r\n");
    PRINTF("Phase 1: Normal communication (%d loops)\r\n", MAX_LOOPS);
    PRINTF("========================================\r\n");

    while (loopCount < MAX_LOOPS)
    {
        SDK_DelayAtLeastUs(500000, SystemCoreClock);
        loopCount++;
        PRINTF("\r\n========== Loop %u/%u ==========\r\n", loopCount, MAX_LOOPS);

        /* Send to all I3C slaves */
        for (uint8_t i = 0; i < SLAVE_COUNT; i++)
        {
            PRINTF("\r\n--- I3C Slave[%d] (0x%02X) ---\r\n", i, g_slaveAddresses[i]);

            result = I3C_MasterSendData(g_slaveAddresses[i],
                                       (uint8_t *)&g_master_txBuff[i],
                                       sizeof(i3c_commm_tx_frame));
            if (result != kStatus_Success)
            {
                PRINTF("TX failed for slave[%d] (0x%02X)\r\n", i, g_slaveAddresses[i]);
                continue;
            }
        }

        /* Receive from all I3C slaves */
        for (uint8_t i = 0; i < SLAVE_COUNT; i++)
        {
            if (g_master_txBuff[i].i3c_tx_frame_followed == 1)
            {
                SDK_DelayAtLeastUs(1000, SystemCoreClock);

                memset(&g_master_rxBuff[i], 0, sizeof(i3c_commm_rx_frame));
                result = I3C_MasterReceiveData(g_slaveAddresses[i],
                                              (uint8_t *)&g_master_rxBuff[i],
                                              8);
                if (result != kStatus_Success)
                {
                    PRINTF("RX failed for slave[%d] (0x%02X)\r\n", i, g_slaveAddresses[i]);
                    continue;
                }

                PRINTF("RX OK - Data:\r\n");
                i3c_commm_rx_frame *pFrame = (i3c_commm_rx_frame *)&g_master_rxBuff[i];

                PRINTF("  a32Position      : 0x%08X\r\n",
                       pFrame->a32Position, pFrame->a32Position);
                PRINTF("  f16SpeedFilt     : 0x%04X\r\n",
                       pFrame->f16SpeedFilt, pFrame->f16SpeedFilt);
                PRINTF("  sFaultIdPending  : 0x%04X\r\n",
                       pFrame->sFaultIdPending, pFrame->sFaultIdPending);
                PRINTF("\r\n");
            }
        }

        PRINTF("\r\n========== End Loop %u/%u ==========\r\n", loopCount, MAX_LOOPS);
    }

    return kStatus_Success;
}

static status_t I3C_Termination(uint8_t targetSlvAddr)
{
    status_t result;

    PRINTF("\r\n========================================\r\n");
    PRINTF("Phase 2: Send termination command to I3C slave\r\n");
    PRINTF("========================================\r\n");

    SDK_DelayAtLeastUs(500000, SystemCoreClock);

    /* Find slave 0x34 */
    uint8_t targetSlaveIndex = 0xFF;
    for (uint8_t i = 0; i < SLAVE_COUNT; i++)
    {
        if (g_slaveAddresses[i] == targetSlvAddr)
        {
            targetSlaveIndex = i;
            break;
        }
    }

    if (targetSlaveIndex != 0xFF)
    {
        /* Set followed flag to 0x5A */
        g_master_txBuff[targetSlaveIndex].i3c_tx_frame_followed = 0x5A;

        PRINTF("\r\n========== Final Transmission to I3C Slave 0x%02X ==========\r\n",targetSlvAddr);
        PRINTF("--- Slave[%d] (0x%02X) - Termination Command ---\r\n",
               targetSlaveIndex, g_slaveAddresses[targetSlaveIndex]);

        /* Send to slave 0x34 */
        result = I3C_MasterSendData(g_slaveAddresses[targetSlaveIndex],
                                   (uint8_t *)&g_master_txBuff[targetSlaveIndex],
                                   sizeof(i3c_commm_tx_frame));
        if (result != kStatus_Success)
        {
            PRINTF("TX failed for slave 0x34\r\n");
            return result;
        }
        else
        {
            PRINTF("TX OK - Termination command sent (followed = 0x5A)\r\n");
            PRINTF("No response expected from slave.\r\n");
        }
    }
    else
    {
        PRINTF("ERROR: Slave with address 0x34 not found!\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}


/*******************************************************************************

 ******************************************************************************
 *******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    status_t result;

    /* Initialize hardware */
    BOARD_InitHardware();
    PRINTF("\r\n========================================\r\n");
    PRINTF("I3C & LPI2C Master Multi-Slave Transfer Example\r\n");
    PRINTF("========================================\r\n");

    /* Initialize I3C master first */
    result = I3C_MasterInitialize();
    if (result != kStatus_Success)
    {
        PRINTF("I3C master initialization failed\r\n");
        return -1;
    }

    PRINTF("\r\nI3C Slaves: %d\r\n", SLAVE_COUNT);
    PRINTF("I3C Slave addresses: ");
    for (uint8_t i = 0; i < SLAVE_COUNT; i++)
    {
        PRINTF("0x%02X ", g_slaveAddresses[i]);
    }
    PRINTF("\r\n");
    PRINTF("LPI2C Slave address: 0x%02X\r\n", LPI2C_MASTER_SLAVE_ADDR_7BIT);
    PRINTF("========================================\r\n\r\n");

    /* Initialize I3C TX buffers */
    for (uint8_t i = 0; i < I3C_TARGET_NUM; i++)
    {
        g_master_txBuff[i].eAppSwitch            = 1;
        g_master_txBuff[i].eControlModeSel       = 3;
        g_master_txBuff[i].a32PositionCmd.i32_value        = 32768 + (i * 1000);
        g_master_txBuff[i].i3c_tx_frame_followed = 1;
    }

    /* Assign I3C dynamic addresses */
    result = I3C_MasterAssignDynamicAddress();
    if (result != kStatus_Success)
    {
        PRINTF("Dynamic address assignment failed\r\n");
        return -1;
    }

    /* Phase 1: Normal I3C communication (3 loops) */
    ////////////////////////////////////////////////////////////////////////////
    result = I3C_NormalCommunication();
    if (result != kStatus_Success)
    {
        PRINTF("I3C communication failed\r\n");
        return -1;
    }

    /* Phase 2.x: Send termination command to I3C slave */
    ////////////////////////////////////////////////////////////////////////////
    for (uint8_t i = 0; i < SLAVE_COUNT; i++)
    {
        SDK_DelayAtLeastUs(100000, SystemCoreClock);
        ROM_ISP_InitUserI2CPins_deinit();
        CLOCK_DisableClock(kCLOCK_Iomuxc2);
        BOARD_InitI3CPins();
        SDK_DelayAtLeastUs(100000, SystemCoreClock);

        result = I3C_Termination(g_slaveAddresses[i]);
        if (result != kStatus_Success)
        {
            PRINTF("I3C termination failed\r\n");
            return -1;
        }

        /* Phase 3.x: Switch to LPI2C and perform firmware update */
        ////////////////////////////////////////////////////////////////////////
        SDK_DelayAtLeastUs(100000, SystemCoreClock);
        BOARD_InitI3CPins_deinit();
        CLOCK_DisableClock(kCLOCK_Iomuxc2);
        ROM_ISP_InitUserI2CPins();
        SDK_DelayAtLeastUs(100000, SystemCoreClock);

        /*
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
        */
        ota_main(i);
    }

    PRINTF("\r\n========================================\r\n");
    PRINTF("All operations completed!\r\n");
    PRINTF("- I3C Normal loops: %u\r\n", MAX_LOOPS);
    PRINTF("- I3C Termination command sent to: all slaves\r\n");
    PRINTF("- LPI2C Firmware update completed\r\n");
    PRINTF("Program finished.\r\n");
    PRINTF("========================================\r\n");

    while(1)
    {
        /* Idle */
    }

    return 0;
}
