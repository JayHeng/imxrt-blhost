/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FILESYSTEM_ALT_H__
#define __FILESYSTEM_ALT_H__
#if defined(__cplusplus)
extern "C" {
#endif

void* fopen_alt(const char *filename, const char *mode);

int fseek_alt(void *stream, int offset, int whence);


int fread_alt(void *ptr, int size, int nmemb, void *stream);

int fwrite_alt(const void *ptr, int size, int nmemb, void *stream);

int ftell_alt(void *stream);

void fclose_alt(void *stream);

int feof_alt(void *stream);

#if defined(__cplusplus)
}
#endif
#endif /* __FILESYSTEM_ALT_H__ */

