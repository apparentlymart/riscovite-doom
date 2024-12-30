//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//

#include <riscovite.h>
#include <string.h>

#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

typedef struct
{
    wad_file_t wad;
    uint64_t hnd;
} riscovite_wad_file_t;

extern wad_file_class_t riscovite_wad_file;

static wad_file_t *W_Riscovite_OpenFile(char *path)
{
    riscovite_wad_file_t *result;
    struct riscovite_result_uint64 r_u64;

    r_u64 = riscovite_open(RISCOVITE_HND_CWD, path, RISCOVITE_OPEN_FILE | RISCOVITE_OPEN_TO_READ);
    if (r_u64.error != 0) {
        return NULL;
    }

    result = Z_Malloc(sizeof(riscovite_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &riscovite_wad_file;
    result->wad.mapped = NULL;
    result->wad.length = 0; // TODO: Actually populate this, once RISCovite has an API for asking this question
    result->hnd = r_u64.value;
    
    printf("opened %s as file number %d\n", path, result->hnd);

    return &result->wad;
}

static void W_Riscovite_CloseFile(wad_file_t *wad)
{
    riscovite_wad_file_t *riscovite_wad;
    riscovite_wad = (riscovite_wad_file_t *) wad;

    riscovite_close(riscovite_wad->hnd);
    Z_Free(riscovite_wad);
}

// Read data from the specified position in the file into the 
// provided buffer.  Returns the number of bytes read.

size_t W_Riscovite_Read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
    struct riscovite_result_uint64 r_u64;
    riscovite_wad_file_t *riscovite_wad;
    riscovite_wad = (riscovite_wad_file_t *) wad;

    // Jump to the specified position in the file.
    r_u64 = riscovite_seek(riscovite_wad->hnd, offset, RISCOVITE_SEEK_SET);
    if (r_u64.error != 0) {
        fprintf(stderr, "failed to seek in WAD file: %s\n", strerror(r_u64.error));
        return 0;
    }
    if (r_u64.value != offset) {
        fprintf(stderr, "failed to seek in WAD file: wanted offset %d, but ended up at %d\n", offset, r_u64.value);
        return 0;
    }

    // Read into the buffer.
    r_u64 = riscovite_read(riscovite_wad->hnd, buffer, buffer_len);
    if (r_u64.error != 0) {
        fprintf(stderr, "failed to read from WAD file: %s\n", strerror(r_u64.error));
        return 0;
    }
    if (r_u64.value != buffer_len) {
        fprintf(stderr, "short read from WAD file: want %d but got %d\n", buffer_len, r_u64.value);
    }

    return (size_t)r_u64.value;
}


wad_file_class_t riscovite_wad_file = 
{
    W_Riscovite_OpenFile,
    W_Riscovite_CloseFile,
    W_Riscovite_Read,
};


