/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app.h"
#include "fsl_common.h"

#if (defined(__ICCARM__))
#pragma section = "__image_section"
#endif

#ifdef RT600_BLINKY_IMAGE
//#define IMAGE_SIZE 0x33DC
#define IMAGE_SIZE (__section_size("__image_section"))
#elif RT700_BLINKY_IMAGE
#define IMAGE_SIZE 11575 /* RT700 Blinky image */
#endif

typedef struct _FileImage
{
    uint32_t open;
    uint32_t pos;
}FileImage_t;

static volatile FileImage_t fileImage;

extern uint8_t g_appIndex;
extern uint32_t g_appStart[SLAVE_COUNT];
extern uint32_t g_appSize[SLAVE_COUNT];

uint32_t get_image_size(void)
{
    //return __section_size("__image_section");
    return g_appSize[g_appIndex];
}

uint32_t get_image_start(void)
{
    //return (uint32_t)__section_begin("__image_section");
    return g_appStart[g_appIndex];
}

void* fopen_alt(const char *filename, const char *mode)
{
    fileImage.pos = 0;
    fileImage.open = 1;
    return (void*)&fileImage;
}

int fseek_alt(void *stream, int offset, int whence)
{
    if(whence == 0) {
        if((offset >= 0) && (offset < get_image_size())) {
            fileImage.pos = offset;
        } else {
            return -1;
        }
    } else if (whence == 1) {
        if((fileImage.pos + offset < get_image_size()) && (fileImage.pos + offset > 0)) {
            fileImage.pos += offset;
        } else {
            return -1;
        }
    } else {
        if((offset <= 0) && (get_image_size() + offset >= 0))
        {
            fileImage.pos = get_image_size() + offset;
        } else {
            return -1;
        }
    }

    return 0;
}

int fread_alt(void *ptr, int size, int nmemb, void *stream)
{
    int copysize = size * nmemb;
    if(copysize + fileImage.pos > get_image_size())
    {
        copysize = get_image_size() - fileImage.pos;
    }

    memcpy(ptr, (void *)(get_image_start() + fileImage.pos), copysize);
    fileImage.pos += copysize;

    return copysize;
}

int fwrite_alt(const void *ptr, int size, int nmemb, void *stream)
{
    while(1);
    return 0;
}

int ftell_alt(void *stream)
{
    return fileImage.pos;
}

void fclose_alt(void *stream)
{
    fileImage.open = 0;
}

int feof_alt(void *stream)
{
    return (fileImage.pos == get_image_size());
}
