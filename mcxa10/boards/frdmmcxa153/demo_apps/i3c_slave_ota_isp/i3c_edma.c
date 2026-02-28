/*
 * i3c_edma.c
 *
 */
/*  Standard C Included Files */
#include <string.h>
#include "fsl_i3c_edma.h"
#include "i3c_edma.h"
#include "fsl_romapi.h"
#include "m1_sm_snsless_enc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SLAVE                  I3C0
#define EXAMPLE_DMA                    DMA0
#define EXAMPLE_I3C_TX_DMA_CHANNEL     (0U)
#define EXAMPLE_I3C_RX_DMA_CHANNEL     (1U)
#define EXAMPLE_I3C_TX_DMA_CHANNEL_MUX (kDma0RequestMuxI3c0Tx)
#define EXAMPLE_I3C_RX_DMA_CHANNEL_MUX (kDma0RequestMuxI3c0Rx)
/* 每个关节节点接收数据帧格式 */
typedef struct _i3c_comm_rx_frame
{
  uint16_t eAppSwitch;
  uint16_t eControlModeSel;
  union
  {
      int32_t i32_value;
      uint8_t u8_value[4];
  } a32PositionCmd;
  uint8_t i3c_tx_frame_followed;
  uint8_t reserved[4];//插入占位符，避免每次接收的数据帧耗尽DMA Majorloop
} i3c_commm_rx_frame;
/* 每个关节节点发送数据帧格式 */
typedef struct _i3c_comm_tx_frame
{
  int32_t a32Position;
  int16_t f16SpeedFilt;
  uint16_t sFaultIdPending;
  uint8_t reserved[4];//插入占位符，避免每次发送的数据帧耗尽DMA Majorloop
} i3c_commm_tx_frame;
/* 每个手指自由度的I3C从机地址 */
enum
{
  ThumbFinger=0x30,//第一个大拇指的I3C从机地址
  IndexFinger,
  MiddleFinger,
  RingFinger,
  LittleFinger
}i3c_finger_addr;

#define I3C_SLAVE_ADDR_7BIT IndexFinger//RingFinger//MiddleFinger//ThumbFinger//LittleFinger//////IndexFinger

i3c_slave_edma_handle_t g_i3c_s_handle;
volatile edma_transfer_config_t edma_tx_transferConfig;
volatile edma_transfer_config_t edma_rx_transferConfig;
AT_NONCACHEABLE_SECTION(i3c_commm_rx_frame g_slave_rxBuff);
AT_NONCACHEABLE_SECTION(i3c_commm_tx_frame g_slave_txBuff);
volatile bool vbool_flag_i3c_commm_frame_update = 0,bIc3ReceiveFlag;
volatile static uint32_t I3C_SlaveBusStopFlagTimes = 0;
volatile static uint32_t I3C_SlaveBusStop_tx_FlagTimes = 0;
volatile static uint32_t I3C_SlaveBusStop_0xa5_FlagTimes = 0;

volatile static uint32_t I3C_SlaveReceivedCCCFlagTimes = 0;
volatile static uint32_t I3C_SlaveCCCHandledFlagTimes = 0;
volatile  uint16_t motor_eControlModeSel = 0;
volatile  uint16_t motor_eAppSwitch = 0;
volatile  int32_t motor_a32PositionCmd = 0;


#define BOOT_ARG_TAG (0xEBu)

/* @brief Boot interface can be selected by user application
 * @note  These interfaces are invalid for ISP boot
 */
enum
{
    kUserAppBootPeripheral_FLASH   = 0u,
    kUserAppBootPeripheral_ISP     = 1u,
    kUserAppBootPeripheral_FLEXSPI = 2u,
    kUserAppBootPeripheral_AUTO    = 3u,
};

/* @brief Boot mode can be selected by user application
 */
enum
{
    kUserAppBootMode_MasterBoot = 0U,
    kUserAppBootMode_IspBoot    = 1U,
};

/* @brief ISP Peripheral definitions
 * @note  For ISP boot, valid boot interfaces for user application are USART I2C SPI USB-HID CAN
 */
//! ISP Peripheral definitions
enum isp_peripheral_constants
{
    kIspPeripheral_Auto     = 0,
    kIspPeripheral_UsbHid   = 1,
    kIspPeripheral_Uart     = 2,
    kIspPeripheral_SpiSlave = 3,
    kIspPeripheral_I2cSlave = 4,
    kIspPeripheral_Can      = 5,
};
user_app_boot_invoke_option_t arg = {.option = {.B = {
                                                    .tag            = BOOT_ARG_TAG,
                                                    .mode           = kUserAppBootMode_IspBoot,
                                                    .boot_interface = kIspPeripheral_Auto,
                                                }}}; // EB: represents Enter Boot; 12: represents enter ISP mode by
                                                     // UART only,13: represents enter ISP mode by SPI only

/*******************************************************************************
 * Code
 ******************************************************************************/
void DMA_CH0_IRQHandler(void)
{
    I3C0->SWDATABE = g_slave_txBuff.reserved[3];
    EDMA_DisableChannelInterrupts(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, kEDMA_MajorInterruptEnable);
    EDMA_ClearChannelStatusFlags(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, kEDMA_InterruptFlag);
}

void I3C0_IRQHandler(void)
{
    status_t I3C_Status, I3C_ErrorStatus;
    I3C_Status = I3C_SlaveGetStatusFlags(I3C0);
    I3C_SlaveClearStatusFlags(I3C0, I3C_Status);
    I3C_ErrorStatus = I3C_SlaveGetErrorStatusFlags(I3C0);
    I3C_SlaveClearErrorStatusFlags(I3C0, I3C_ErrorStatus);
    if(kI3C_SlaveCCCHandledFlag&I3C_Status)
    {
        if((vbool_flag_i3c_commm_frame_update == true)&&((I3C_ErrorStatus & I3C_SERRWARN_SPAR_MASK) == 0))
        {
            vbool_flag_i3c_commm_frame_update = false;
            motor_a32PositionCmd  = g_slave_rxBuff.a32PositionCmd.i32_value;
            motor_eControlModeSel = g_slave_rxBuff.eControlModeSel;
            motor_eAppSwitch      = g_slave_rxBuff.eAppSwitch;
            bIc3ReceiveFlag =1 ;
        }
        I3C_SlaveClearStatusFlags(I3C0, kI3C_SlaveCCCHandledFlag);
        I3C_SlaveCCCHandledFlagTimes++;
    }
    else if(kI3C_SlaveReceivedCCCFlag&I3C_Status)
    {
        EDMA_TCD_DADDR(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL),0) = edma_rx_transferConfig.destAddr;
        EDMA_TCD_CITER(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL),0) = edma_rx_transferConfig.majorLoopCounts;
        EDMA_TCD_BITER(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL),0) = edma_rx_transferConfig.majorLoopCounts;
        I3C0->SDATACTRL |= I3C_SDATACTRL_FLUSHTB_MASK | I3C_SDATACTRL_FLUSHFB_MASK;//Clear FIFO
        I3C_SlaveClearStatusFlags(I3C0, kI3C_SlaveReceivedCCCFlag);
        I3C_SlaveReceivedCCCFlagTimes++;
    }
    else if(kI3C_SlaveBusStopFlag&I3C_Status)
    {
        EDMA_TCD_DADDR(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL),0) = edma_rx_transferConfig.destAddr;
        EDMA_TCD_CITER(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL),0) = edma_rx_transferConfig.majorLoopCounts;
        EDMA_TCD_BITER(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL),0) = edma_rx_transferConfig.majorLoopCounts;
        I3C0->SDATACTRL |= I3C_SDATACTRL_FLUSHTB_MASK | I3C_SDATACTRL_FLUSHFB_MASK;//Clear FIFO
        I3C_SlaveClearStatusFlags(I3C0, kI3C_SlaveBusStopFlag);
        vbool_flag_i3c_commm_frame_update = true;
        I3C_SlaveBusStopFlagTimes++;
        I3C_SlaveEnableDMA(I3C0, 0, 1, 1);  /* Re-enable RX DMA, disable TX DMA */
        /* === TX DMA Channel Setup (if response required) === */
        if (g_slave_rxBuff.i3c_tx_frame_followed == 1)
        {
            g_slave_txBuff.sFaultIdPending    = g_sM1Drive.sFaultIdPending;
            g_slave_txBuff.a32Position        = g_sM1Drive.sPosition.a32Position;
            g_slave_txBuff.f16SpeedFilt       = g_sM1Drive.sSpeed.f16SpeedFilt;
            EDMA_TCD_SADDR(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL), 0) =	(uint32_t)&g_slave_txBuff;
            EDMA_TCD_CITER(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL), 0) =	sizeof(g_slave_txBuff);
            EDMA_TCD_BITER(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL), 0) =	sizeof(g_slave_txBuff);
            /* Flush I3C TX FIFO and enable DMA */
            I3C0->SDATACTRL |= I3C_SDATACTRL_FLUSHTB_MASK;      /* Clear FIFO */
            I3C_SlaveClearStatusFlags(I3C0, kI3C_SlaveBusStopFlag);
            I3C_SlaveEnableDMA(I3C0, 1, 0, 1);                  /* Enable TX DMA */
            EDMA_EnableChannelRequest(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL);

            I3C_SlaveBusStop_tx_FlagTimes++;
            g_slave_rxBuff.i3c_tx_frame_followed = 0;
        }
    if (g_slave_rxBuff.i3c_tx_frame_followed == 0x5a)
    {
        ROM_API->run_bootloader(&arg);
                    I3C_SlaveBusStop_0xa5_FlagTimes++;

        g_slave_rxBuff.i3c_tx_frame_followed = 0;
    }
    }
    __DSB();
    __ISB();
}

/**
 * @brief Calculate output position based on vendor ID
 * @param pos vendor ID position
 * @return decoded slave number
 */
int calculate_output(int pos) {
    if (pos % 2 == 1) {
        return (pos + 1) / 2;
    } else {
        return 13 - pos / 2;
    }
}

void I3C_InitSlave(void)
{
    i3c_slave_config_t slaveConfig;
    edma_config_t config;

    memset(&edma_tx_transferConfig, 0, sizeof(edma_transfer_config_t));
    g_slave_txBuff.a32Position        = 0x1234;
    g_slave_txBuff.f16SpeedFilt	      = 0x56;
    g_slave_txBuff.sFaultIdPending    = 0x78;
    /* 1st: EDMA Clock Init */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(EXAMPLE_DMA, &config);

    /* 2nd: EDMA Channel Mux Distribution */
    EDMA_ClearChannelStatusFlags(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, kEDMA_DoneFlag|kEDMA_ErrorFlag|kEDMA_InterruptFlag);
    EDMA_ClearChannelStatusFlags(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL, kEDMA_DoneFlag|kEDMA_ErrorFlag|kEDMA_InterruptFlag);
    EDMA_SetChannelMux(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, EXAMPLE_I3C_TX_DMA_CHANNEL_MUX);
    EDMA_SetChannelMux(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL, EXAMPLE_I3C_RX_DMA_CHANNEL_MUX);

    /* 3rd: Config EDMA TCD */
    /* (1) I3C Rx DMA Install */
    EDMA_PrepareTransfer(&edma_rx_transferConfig,
                        (void*)&I3C0->SRDATAB,
                        1,
                        (uint8_t*)&g_slave_rxBuff,
                        1,
                        1,
                        sizeof(g_slave_rxBuff),
                        kEDMA_PeripheralToMemory);
    EDMA_SetTransferConfig(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL, &edma_rx_transferConfig, NULL);
    EDMA_EnableChannelRequest(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL);

    /* (2) I3C Tx DMA Install */
    /* === Step 3: Configure TX DMA Channel (Memory -> Peripheral) === */
    EDMA_ClearChannelStatusFlags(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, kEDMA_DoneFlag|kEDMA_ErrorFlag|kEDMA_InterruptFlag);
    EDMA_SetChannelMux(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, EXAMPLE_I3C_TX_DMA_CHANNEL_MUX);
    EDMA_PrepareTransfer(&edma_tx_transferConfig,
                         (uint8_t *)&g_slave_txBuff,  /* Source: TX buffer in RAM */
                         1,                            /* Source address offset: 1 byte */
                         (void *)&I3C0->SWDATAB,      /* Destination: I3C TX data register */
                         1,                            /* Destination address offset: 1 byte */
                         1,                            /* Minor loop transfer size: 1 byte */
                         sizeof(g_slave_txBuff),    /* Major loop count: 16 bytes total */
                         kEDMA_MemoryToPeripheral);   /* Transfer direction */
    EDMA_SetTransferConfig(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, &edma_tx_transferConfig, NULL);
    EDMA_DisableChannelRequest(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL);  /* Disable until needed */


    /* 4th: Init I3C */
    I3C_SlaveGetDefaultConfig(&slaveConfig);
    slaveConfig.staticAddr = I3C_SLAVE_ADDR_7BIT;
    slaveConfig.vendorID   = 0U;
    slaveConfig.offline    = false;
    slaveConfig.matchSlaveStartStop    = true;
    I3C_SlaveInit(EXAMPLE_SLAVE, &slaveConfig, CLOCK_GetI3CFClkFreq());
    I3C_SlaveClearStatusFlags(I3C0, kI3C_SlaveClearFlags);
    I3C_SlaveEnableInterrupts(I3C0, kI3C_SlaveBusStopFlag|kI3C_SlaveReceivedCCCFlag|kI3C_SlaveCCCHandledFlag);
    NVIC_EnableIRQ(I3C0_IRQn);

    /* 5th: Link Rx I3C Request to DMA */
    I3C_SlaveEnableDMA(I3C0, 0, 1, 1);
}

int I3C_SendBytes(void)
{
    EDMA_TCD_SADDR(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL),0) = (uint32_t)(&g_slave_txBuff);
    EDMA_TCD_CITER(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL),0) = sizeof(i3c_commm_tx_frame)-1;
    EDMA_TCD_BITER(EDMA_TCD_BASE(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL),0) = sizeof(i3c_commm_tx_frame)-1;
    EDMA_EnableChannelRequest(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL);
    EDMA_EnableChannelInterrupts(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, kEDMA_MajorInterruptEnable);
//	I3C_SlaveClearStatusFlags(I3C0, kI3C_SlaveClearFlags);
//	I3C_SlaveEnableInterrupts(I3C0, kI3C_SlaveMatchedFlag);
//	/* Wait for slave transmit completed. */
//	while (!g_slaveCompletionFlag) {}
//	g_slaveCompletionFlag = false;
    return 0;
}
