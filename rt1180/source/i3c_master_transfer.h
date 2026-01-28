/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _I3C_MASTER_TRANSFER_H_
#define _I3C_MASTER_TRANSFER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "fsl_i3c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* I3C Master Configuration */
#define EXAMPLE_MASTER                 I3C1
#define I3C_MASTER_CLOCK_FREQUENCY     CLOCK_GetRootClockFreq(kCLOCK_Root_I3c1)
#define EXAMPLE_I2C_BAUDRATE           400000U
#define EXAMPLE_I3C_OD_BAUDRATE        400000U   /* Open-drain baudrate */
#define EXAMPLE_I3C_PP_BAUDRATE        1000000U  /* Push-pull baudrate */
#define WAIT_TIME_US                   10000U

/* System Configuration */
#define I3C_TARGET_NUM                 5U        /* Number of slave nodes */
#define EXAMPLE_DMA                    DMA3
#define I3C_DATA_LENGTH                12U       /* Frame size in bytes */

/* I3C Addressing */
#define I3C_MASTER_SLAVE_ADDR_7BIT     0x30U     /* Static slave address */
#define I3C_BROADCAST_ADDR             0x7EU     /* Broadcast address */

/* I3C Common Command Codes (CCC) */
#define CCC_RSTDAA                     0x06U     /* Reset dynamic address */
#define CCC_SETDASA                    0x87U     /* Set dynamic address from static */
#define CCC_SETAASA                    0x29U     /* Set all addresses to static */

/* Address Mode Selection */
#define STATIC_I3C_ADDR                          /* Use static address (recommended) */
// #define DYNAMIC_I3C_ADDR                      /* Use dynamic address assignment */

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * @brief I3C communication frame structure
 * @note Total size: 12 bytes (aligned for DMA transfer)
 */
typedef struct _i3c_comm_tx_frame
{
  uint16_t eAppSwitch;
  uint16_t eControlModeSel;
  union
  {
	  int32_t i32_value;
      uint8_t u8_value[4];
  } a32PositionCmd;
  uint8_t i3c_tx_frame_followed;
  uint8_t reserved[2];//插入占位符，避免每次接收的数据帧耗尽DMA Majorloop
} i3c_commm_tx_frame;

typedef struct _i3c_comm_rx_frame
{
  int32_t a32Position;
  int16_t f16SpeedFilt;
  uint16_t sFaultIdPending;
  uint8_t reserved[2];//插入占位符，避免每次发送的数据帧耗尽DMA Majorloop
} i3c_commm_rx_frame;


/*******************************************************************************
 * External Variables
 ******************************************************************************/
extern i3c_commm_tx_frame       g_master_txBuff[I3C_TARGET_NUM];  /* TX buffers */
extern i3c_commm_rx_frame       g_master_rxBuff[I3C_TARGET_NUM];  /* RX buffers */
extern i3c_master_transfer_t masterXfer;                       /* Transfer handle */

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
void I3C_Master_Init(void);   /* Initialize I3C master */
void I3C_SendBytes(void);     /* Send data to slaves */

#endif /* _I3C_MASTER_TRANSFER_H_ */
