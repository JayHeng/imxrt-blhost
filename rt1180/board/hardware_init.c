/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*${header:start}*/
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "app.h"

#include "fsl_common.h"
#include "fsl_iomuxc.h"
/*${header:end}*/

void BOARD_InitUserI2cPins(void)
{
  CLOCK_EnableClock(kCLOCK_Iomuxc1);          /* Turn on LPCG: LPCG is ON. */
  CLOCK_EnableClock(kCLOCK_Iomuxc2);          /* Turn on LPCG: LPCG is ON. */

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_15_LPI2C2_SDA,           /* GPIO_AD_18 is configured as LPI2C3_SCL */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AD_18 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_16_LPI2C2_SCL,           /* GPIO_AD_19 is configured as LPI2C3_SDA */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AD_19 */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_15_LPI2C2_SDA,           /* GPIO_AD_18 PAD functional properties : */
      0x1EU);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Enable
                                                 Pull Up / Down Config. Field: Weak pull up
                                                 Open Drain Field: Enabled
                                                 Force ibe off Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_16_LPI2C2_SCL,           /* GPIO_AD_19 PAD functional properties : */
      0x1EU);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Enable
                                                 Pull Up / Down Config. Field: Weak pull up
                                                 Open Drain Field: Enabled
                                                 Force ibe off Field: Disabled */
}

/*${function:start}*/
void BOARD_InitHardware(void)
{
    BOARD_CommonSetting();
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitI3CPins();
    //BOARD_InitUserI2cPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
}
/*${function:end}*/
