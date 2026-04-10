/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"    //! @name SPI
#include "fsl_debug_console.h"
#include "fsl_spi.h"

#define BUFFER_SIZE (65*1024)
static uint8_t txBuff[BUFFER_SIZE];
static uint8_t rxBuff[BUFFER_SIZE];

#define USED_SPI_INSTANCE (14)
#if USED_SPI_INSTANCE == 5
#define SPI_MASTER          SPI5
#define SPI_MASTER_CLK      kCLOCK_Flexcomm5Clk
#elif USED_SPI_INSTANCE == 14
#define SPI_MASTER          SPI14
#define SPI_MASTER_CLK      kCLOCK_Flexcomm14Clk
#endif

extern bool s_isImage;

int spi_setup(int fd, uint32_t speed, uint32_t mode, uint32_t bits_per_word)
{
    static bool isSpiDeinited = false;
    spi_master_config_t userConfig = {0};
    uint32_t srcFreq               = 0;

    SPI_MasterGetDefaultConfig(&userConfig);
    srcFreq = CLOCK_GetFreq(SPI_MASTER_CLK);

    if (s_isImage)
    {
        if (!isSpiDeinited)
        {
            SPI_Deinit(SPI_MASTER);
            isSpiDeinited = true;
            userConfig.baudRate_Bps = 12000000;
        }
        else
        {
            userConfig.baudRate_Bps = 12000000;
        }
    }
    else
    {
        userConfig.baudRate_Bps = 12000000;
    }
    PRINTF("spi_setup baudrate = %d, \r\n", userConfig.baudRate_Bps);
    userConfig.polarity                  = kSPI_ClockPolarityActiveLow;
    userConfig.phase                     = kSPI_ClockPhaseSecondEdge;	
    userConfig.sselNum = (spi_ssel_t)kSPI_Ssel0;
    userConfig.sselPol = (spi_spol_t)0;

    SPI_MasterInit(SPI_MASTER, &userConfig, srcFreq);	
    return 0;
}

int spi_set_timeout(int fd, uint32_t miliseconds)
{
//    PRINTF("spi_set_timeout miliseconds = %d\r\n", miliseconds);

    return 0;
}

int spi_write(int fd, char *buf, int size)
{
    spi_transfer_t xfer            = {0};

    if(size > BUFFER_SIZE)
	    while(1);

//    PRINTF("spi_write size = %d\r\n", size);
    xfer.txData      = buf;
    xfer.rxData      = rxBuff;
    xfer.dataSize    = size;
    xfer.configFlags = kSPI_FrameAssert;
    SPI_MasterTransferBlocking(SPI_MASTER, &xfer);

    return size;
}

int spi_read(int fd, char *buf, int size)
{
    spi_transfer_t xfer            = {0};

	if(size > BUFFER_SIZE)
		while(1);

//    PRINTF("spi_read size = %d\r\n", size);
    memset(txBuff, 0x0, size);

    xfer.txData      = txBuff;
    xfer.rxData      = buf;
    xfer.dataSize    = size;
    xfer.configFlags = kSPI_FrameAssert;
    SPI_MasterTransferBlocking(SPI_MASTER, &xfer);

    return size;
}

int spi_open(char *port)
{
//    PRINTF("spi_open %s\r\n", port);
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM5);
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM14);
	
    return 0;
}

int spi_close(int fd)
{
    return 0;
}

