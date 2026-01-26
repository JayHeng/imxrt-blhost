/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"    //! @name SPI
#include "fsl_debug_console.h"

int spi_setup(int fd, uint32_t speed, uint32_t mode, uint32_t bits_per_word)
{
    return 0;
}

int spi_set_timeout(int fd, uint32_t miliseconds)
{
    return 0;
}

int spi_write(int fd, char *buf, int size)
{
    return 0;
}

int spi_read(int fd, char *buf, int size)
{
    return 0;
}

int spi_open(char *port)
{
    return 0;
}

int spi_close(int fd)
{
    return 0;
}

