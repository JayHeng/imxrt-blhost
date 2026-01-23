/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "blfwk/hidapi.h"

hid_device * hid_open(unsigned short vendor_id,
                          unsigned short product_id,
                          const wchar_t *serial_number)
{
    return NULL;
}


hid_device * hid_open_path(const char *path)
{
    return NULL;
}

void hid_close(hid_device *device)
{

}

int hid_exit(void)
{
    return 0;
}

int hid_write_timeout(hid_device *device,
                            const unsigned char *data,
                            size_t length,
                            int milliseconds)
{
    return 0;
}

const wchar_t * hid_error(hid_device *device)
{
    return NULL;
}

int hid_read_timeout(hid_device *dev, unsigned char *data, 
	                       size_t length, int milliseconds)
{
    return 0;
}
