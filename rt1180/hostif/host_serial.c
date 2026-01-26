/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"

int serial_setup(int fd, int speed)
{
    return 0;
}
int serial_set_read_timeout(int fd, uint32_t timeoutMs)
{
    return 0;
}

int serial_write(int fd, char *buf, int size)
{
    return 0;
}

int serial_read(int fd, char *buf, int size)
{
    return 0;
}

int serial_open(char *port)
{
    return 0;
}

int serial_close(int fd)
{
    return 0;
}