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

/*! @brief Ring buffer size (Unit: Byte). */
#define UART_RING_BUFFER_SIZE 512

/*
  Ring buffer for data input and output, in this example, input data are saved
  to ring buffer in IRQ handler. The main function polls the ring buffer status,
  if there are new data, then send them out.
  Ring buffer full: (((rxIndex + 1) % UART_RING_BUFFER_SIZE) == fetchIndex)
  Ring buffer empty: (rxIndex == fetchIndex)
*/
uint8_t uartRingBuffer[UART_RING_BUFFER_SIZE];
volatile uint16_t fetchIndex; /* Index of the data to fetch. */
volatile uint16_t recvIndex;  /* Index of the memory to save new arrived data. */

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
    USART_EnableInterrupts(USART0, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    EnableIRQ(FLEXCOMM0_IRQn);

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

void FLEXCOMM0_IRQHandler(void)
{
    uint8_t data;

    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(USART0))
    {
        data = USART_ReadByte(USART0);
        /* If ring buffer is not full, add data to ring buffer. */
        if (((recvIndex + 1) % UART_RING_BUFFER_SIZE) != fetchIndex)
        {
            uartRingBuffer[recvIndex] = data;
            recvIndex++;
            recvIndex %= UART_RING_BUFFER_SIZE;
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

int serial_read(int fd, char *buf, int size)
{
    if(size > BUFFER_SIZE)
	    while(1);
    
    //USART_ReadBlocking(USART0, buf, size);
    int remainingBytes = size;
    char * tmpBuf = buf;
    while (remainingBytes > 0)
    {
        if (recvIndex != fetchIndex)
        {
            *tmpBuf = uartRingBuffer[fetchIndex];
            tmpBuf++;
            remainingBytes--;
            fetchIndex++;
            fetchIndex %= UART_RING_BUFFER_SIZE;
        }
    }

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