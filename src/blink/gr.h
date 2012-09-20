// Copyright (c) 2012, Ben Scott.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once
#ifndef BL_GR_H
#define BL_GR_H

#include <blink/base.h>

#define BL_GR_TEX_FILE_MAGIC            0x42544558u // BTEX
#define BL_GR_TEX_FILE_VERSION          0x00000000u
#define BL_GR_TEX_FILE_ENDIAN_BIG       0x00000000u
#define BL_GR_TEX_FILE_ENDIAN_LITTLE    0x00000001u
#define BL_GR_TEX_FILE_HEIGHT_MAX       (1 << 12)
#define BL_GR_TEX_FILE_WIDTH_MAX        (1 << 12)
#define BL_GR_TEX_FILE_LEVELS_MAX       (1 << 4)

#define BL_GR_TEX_FILE_FORMAT_DXT5      0
#define BL_GR_TEX_FILE_FORMAT_DXT1      1
// ...
#define BL_GR_TEX_FILE_FORMAT_MAX       2

#ifdef BL_BIG_ENDIAN
# define BL_GR_TEX_FILE_ENDIAN_CURRENT  BL_GR_TEX_FILE_ENDIAN_BIG
#else
# define BL_GR_TEX_FILE_ENDIAN_CURRENT  BL_GR_TEX_FILE_ENDIAN_LITTLE
#endif


struct BLGrTex;

struct BLGrTexFileHeader {
  uint32_t  magic;    // (big endian) BTEX
  uint32_t  version;  // (big endian)
  uint32_t  endian;   // (big endian)
  uint32_t  height;
  uint32_t  width;
  uint32_t  levels;
  uint32_t  format;   // dxt5, dxt1, rgba8888, ...

  uint32_t  reserved; // unused, forces 16-byte alignment
};


void bl_gr_lib_initialize();
void bl_gr_lib_finalize();

void bl_gr_process_commands();

void bl_gr_tex_bind(BLGrTex* tex);
void bl_gr_tex_unbind(BLGrTex* tex);
BLGrTex* bl_gr_tex_load(const char* buf, unsigned int buf_size);
void bl_gr_tex_unload(BLGrTex* tex);

#endif
