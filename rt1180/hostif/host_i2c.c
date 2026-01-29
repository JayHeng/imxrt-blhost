/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"    //! @name I2C
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"

#define BUFFER_SIZE 1040

#define I2C_MASTER_SLAVE_ADDR_7BIT 0x7EU

int i2c_setup(int fd, uint32_t speed, uint8_t address)
{
    lpi2c_master_config_t masterConfig;
    uint32_t srcFreq= CLOCK_GetRootClockFreq(kCLOCK_Root_Lpi2c0102);

    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * masterConfig.baudRate_Hz = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    LPI2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Hz = 100000u;

    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterInit(LPI2C2, &masterConfig, srcFreq);
}
 
int i2c_set_timeout(int fd, uint32_t miliseconds)
{
    return 0;
}

int i2c_write(int fd, char *buf, int size)
{
    lpi2c_master_transfer_t xfer = {0};

    if(size > BUFFER_SIZE)
        while(1);

    uint8_t deviceAddress = 0x10U;
    xfer.slaveAddress   = deviceAddress;//I2C_MASTER_SLAVE_ADDR_7BIT;
    xfer.direction      = kLPI2C_Write;
    //xfer.subaddress     = (uint32_t)deviceAddress;
    //xfer.subaddressSize = 1;
    xfer.data           = buf;
    xfer.dataSize       = size;
    xfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Send master blocking data to slave */
    LPI2C_MasterTransferBlocking(LPI2C2, &xfer);

    return size;
}

int i2c_read(int fd, char *buf, int size)
{
    lpi2c_master_transfer_t xfer = {0};

    if(size > BUFFER_SIZE)
        while(1);

    uint8_t deviceAddress = 0x10U;
    xfer.slaveAddress   = deviceAddress;//I2C_MASTER_SLAVE_ADDR_7BIT;
    xfer.direction      = kLPI2C_Read;
    //xfer.subaddress     = (uint32_t)deviceAddress;
    //xfer.subaddressSize = 1;
    xfer.data           = buf;
    xfer.dataSize       = size;
    xfer.flags          = kLPI2C_TransferDefaultFlag;

    LPI2C_MasterTransferBlocking(LPI2C2, &xfer);

    return size;
}

int i2c_open(char *port)
{
    return 0;
}

int i2c_close(int fd)
{
    return 0;
}

