/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"

#define IMAGE_SECTION_ADDRESS 0x04100000

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

static FileImage_t fileImage;


void* fopen_alt(const char *filename, const char *mode)
{
    fileImage.pos = 0;
    fileImage.open = 1;
    return &fileImage;
}

int fseek_alt(void *stream, int offset, int whence)
{
    if(whence == 0) {
        if((offset >= 0) && (offset < IMAGE_SIZE)) {
            fileImage.pos = offset;
        } else {
            return -1;
        }
    } else if (whence == 1) {
        if((fileImage.pos + offset < IMAGE_SIZE) && (fileImage.pos + offset > 0)) {
            fileImage.pos += offset;
        } else {
            return -1;
        }
    } else {
        if((offset <= 0) && (IMAGE_SIZE + offset >= 0))
        {
            fileImage.pos = IMAGE_SIZE + offset;
        } else {
            return -1;
        }
    }

    return 0;
}

int fread_alt(void *ptr, int size, int nmemb, void *stream)
{
    int copysize = size * nmemb;
    if(copysize + fileImage.pos > IMAGE_SIZE)
    {
        copysize = IMAGE_SIZE - fileImage.pos;
    }

    memcpy(ptr, (void *)(IMAGE_SECTION_ADDRESS + fileImage.pos), copysize);
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
    return (fileImage.pos == IMAGE_SIZE);
}
