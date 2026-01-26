/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define BLHOST_USE_SPI (0)
#define BLHOST_USE_I2C (1)

#define DEVICE_IN_3b111_SERIAL_MASTER_BOOT (0)
#define DEVICE_IN_3b110_SERIAL_ISP_BOOT    (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BLHOST_SPI_ARGC0 (6)
char *blhost_spi_args0[] = {
    "blhost",
    "-s",           // -s means spi
    "5,4000",       // means dev/spi-5,4000kHz(,1,1,msb)
    "--",
    "get-property",
    "1"
};

#define BLHOST_I2C_ARGC0 (6)
char *blhost_i2c_args0[] = {
    "blhost",
    "-i",           // -i means i2c
    "2,0x10",       // means dev/i2c-2,0x10(,100kHz)
    "--",
    "get-property",
    "1"
};

#define BLHOST_SPI_ARGC1 (7)
char *blhost_spi_args1[] = {
    "blhost",
    "-s",
    "5,4000",
    "--",
    "set-property",
    "0x1C",
    "0x80000110"
};

#define BLHOST_SPI_ARGC2 (8)
char *blhost_spi_args2[] = {
    "blhost",
    "-s",
    "2,0x10",
    "--",
    "write-memory",
    "0x80000",
    "0x200000",
    "0x80000"
};

#define BLHOST_I2C_ARGC2 (8)
char *blhost_i2c_args2[] = {
    "blhost",
    "-i",
    "5,4000",
    "--",
    "write-memory",
    "0x80000",
    "0x200000",
    "0x80000"
};

#define BLHOST_I2C_ARGC3 (8)
char *blhost_i2c_args3_1[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "fill-memory",
    "0x10c000",
    "4",
    "0xc1503051"
};
char *blhost_i2c_args3_2[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "fill-memory",
    "0x10c004",
    "4",
    "0x20000014"
};

#define BLHOST_I2C_ARGC4 (7)
char *blhost_i2c_args4[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "configure-memory",
    "0x9",
    "0x10c000"
};

#define BLHOST_I2C_ARGC5 (8)
char *blhost_i2c_args5[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "flash-erase-region",
    "0x08010000",
    "0x10000",
    "0x9"
};

#define BLHOST_I2C_ARGC6 (8)
char *blhost_i2c_args6[] = {
    "blhost",
    "-i",
    "5,4000",
    "--",
    "write-memory",
    "0x08010000",
    "0x200000",
    "0x4000"
};

#define BLHOST_I2C_ARGC7 (8)
// arg for sdk20-app_rt600.bin
char *blhost_i2c_args7[] = {
    "blhost",
    "-i",
    "2,0x10",
    "--",
    "execute",
    "0x00082D09",
    "0x0",
    "0x20200000"
};

#ifdef RT700_BLINKY_IMAGE
#define BLHOST_SPI_ARGC3 (8)
/* RT700 IMAGE */
char *blhost_spi_args3[] = {
    "blhost",
    "-s",
    "5,4000",
    "--",
    "execute",
    "0x8238D",
    "0x80000",
    "0x20200000"
};
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    gpio_pin_config_t gpio_pin_config;
    gpio_pin_config.pinDirection = kGPIO_DigitalOutput;
    gpio_pin_config.outputLogic = 1;
	
    GPIO_PortInit(GPIO, 0);
    GPIO_PinInit(GPIO, 0, 6, &gpio_pin_config);

    /* Init board hardware. */
    BOARD_InitHardware();

    PRINTF("BLHOST.\r\n");

#if BLHOST_USE_SPI
    blhost_main(BLHOST_SPI_ARGC0, blhost_spi_args0, NULL);
    blhost_main(BLHOST_SPI_ARGC2, blhost_spi_args2, NULL);
#ifdef RT700_BLINKY_IMAGE
    blhost_main(BLHOST_SPI_ARGC3, blhost_spi_args3, NULL);
#endif
#elif BLHOST_USE_I2C
    blhost_main(BLHOST_I2C_ARGC0, blhost_i2c_args0, NULL);
    
    blhost_main(BLHOST_I2C_ARGC3, blhost_i2c_args3_1, NULL);
    blhost_main(BLHOST_I2C_ARGC3, blhost_i2c_args3_2, NULL);
    blhost_main(BLHOST_I2C_ARGC4, blhost_i2c_args4, NULL);
    blhost_main(BLHOST_I2C_ARGC5, blhost_i2c_args5, NULL);
    blhost_main(BLHOST_I2C_ARGC6, blhost_i2c_args6, NULL);
    
    //blhost_main(BLHOST_I2C_ARGC2, blhost_i2c_args2, NULL);
#if DEVICE_IN_3b110_SERIAL_ISP_BOOT
    //blhost_main(BLHOST_I2C_ARGC7, blhost_i2c_args7, NULL);
#endif
#endif
    PRINTF("Done\r\n");
    while (1)
    {

    }
}
