/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"    //! @name I2C
#include "fsl_debug_console.h"
#include "fsl_i2c.h"

#define BUFFER_SIZE 1040

#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)

int i2c_setup(int fd, uint32_t speed, uint8_t address)
{
    i2c_master_config_t masterConfig;
    uint32_t srcFreq               = 0;

    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kI2C_2PinOpenDrain;
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    srcFreq = CLOCK_GetFlexCommClkFreq(2U);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = 100000u;

    /* Initialize the I2C master peripheral */
    I2C_MasterInit(I2C2, &masterConfig, srcFreq);
    return 0;
}
 
int i2c_set_timeout(int fd, uint32_t miliseconds)
{
    return 0;
}

int i2c_write(int fd, char *buf, int size)
{
    uint8_t deviceAddress = 0x10U;

    i2c_master_transfer_t xfer;
    memset(&xfer, 0, sizeof(xfer));

    if(size > BUFFER_SIZE)
        while(1);

    xfer.slaveAddress   = deviceAddress;//I2C_MASTER_SLAVE_ADDR_7BIT;
    xfer.direction      = kI2C_Write;
    //xfer.subaddress     = (uint32_t)deviceAddress;
    //xfer.subaddressSize = 1;
    xfer.data           = buf;
    xfer.dataSize       = size;
    xfer.flags          = kI2C_TransferDefaultFlag;
    I2C_MasterTransferBlocking(I2C2, &xfer);

    return size;
}

int i2c_read(int fd, char *buf, int size)
{
    uint8_t deviceAddress = 0x10U;

    i2c_master_transfer_t xfer;
    memset(&xfer, 0, sizeof(xfer));

    if(size > BUFFER_SIZE)
        while(1);

    xfer.slaveAddress   = deviceAddress;//I2C_MASTER_SLAVE_ADDR_7BIT;
    xfer.direction      = kI2C_Read;
    //xfer.subaddress     = (uint32_t)deviceAddress;
    //xfer.subaddressSize = 1;
    xfer.data           = buf;
    xfer.dataSize       = size;
    xfer.flags          = kI2C_TransferDefaultFlag;
    I2C_MasterTransferBlocking(I2C2, &xfer);

    return size;
}

int i2c_open(char *port)
{
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM2);

    return 0;
}

int i2c_close(int fd)
{
    return 0;
}

