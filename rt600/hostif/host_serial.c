/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_usart.h"

#define BUFFER_SIZE (65*1024)

int serial_setup(int fd, int speed)
{
    usart_config_t config;
    uint32_t srcFreq               = 0;

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.loopback = false;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 57600;
    config.enableTx     = true;
    config.enableRx     = true;

    srcFreq = CLOCK_GetFlexCommClkFreq(0U);

    USART_Init(USART0, &config, srcFreq);

    return 0;
}
int serial_set_read_timeout(int fd, uint32_t timeoutMs)
{
    return 0;
}

int serial_write(int fd, char *buf, int size)
{
    if(size > BUFFER_SIZE)
	    while(1);

    USART_WriteBlocking(USART0, buf, size);
#if 0
    int loops = size;
    PRINTF("\r\nTP Send: ");
    while(loops--)
    {
        PRINTF(" 0x%x", *buf);
        buf++;
    }
#endif

    return size;
}

int serial_read(int fd, char *buf, int size)
{
    if(size > BUFFER_SIZE)
	    while(1);
    
    USART_ReadBlocking(USART0, buf, size);
#if 0
    int loops = size;
    PRINTF("\r\nTP Recv: ");
    while(loops--)
    {
        PRINTF(" 0x%x", *buf);
        buf++;
    }
    PRINTF("\r\n");
#endif

    return size;
}

int serial_open(char *port)
{
    CLOCK_SetFRGClock((&(const clock_frg_clk_config_t){0, kCLOCK_FrgPllDiv, 255, 0}));
    CLOCK_AttachClk(kFRG_to_FLEXCOMM0);
    return 0;
}

int serial_close(int fd)
{
    return 0;
}