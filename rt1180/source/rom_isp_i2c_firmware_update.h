/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ROM_ISP_I2C_FIRMWARE_UPDATE_H_
#define _ROM_ISP_I2C_FIRMWARE_UPDATE_H_

#include "fsl_common.h"
#include "fsl_lpi2c.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef EXAMPLE_I2C_MASTER_BASE
#define EXAMPLE_I2C_MASTER_BASE LPI2C2_BASE
#endif

#ifndef LPI2C_MASTER_CLOCK_FREQUENCY
#define LPI2C_MASTER_CLOCK_FREQUENCY CLOCK_GetRootClockFreq(kCLOCK_Root_Lpi2c0102)
#endif

#define EXAMPLE_I2C_MASTER ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)
#define LPI2C_BAUDRATE               100000U
#define LPI2C_DATA_LENGTH            2U
#define LPI2C_READ_LENGTH            11U
#define LPI2C_MASTER_SLAVE_ADDR_7BIT 0x10U


/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Switch pins from I3C to LPI2C mode
 */
void ROM_ISP_InitUserI2CPins(void);
void ROM_ISP_InitUserI2CPins_deinit(void);

/*!
 * @brief Initialize LPI2C Master
 * @return Status of initialization
 */
status_t ROM_ISP_LPI2C_MasterInitialize(void);

/*!
 * @brief Perform firmware update via LPI2C
 * @return Status of firmware update
 */
status_t ROM_ISP_I2C_FirmwareUpdate(void);

#if defined(__cplusplus)
}
#endif

#endif /* _ROM_ISP_I2C_FIRMWARE_UPDATE_H_ */
