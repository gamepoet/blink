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
#ifndef BL_BITBUF_H
#define BL_BITBUF_H

#include <blink/base.h>

struct BLBitBuf {
  uint32_t* data;
  int       bit_count;
  int       int_count;
};

// sets up the given buffer but leaves the data uninitialized
void bl_bitbuf_init(BLBitBuf* __restrict buf, uint32_t* __restrict data, int bit_count, int byte_count);

// clears all bits in the buffer
void bl_bitbuf_clear(BLBitBuf* buf);

// clears the bit at the given offset
void bl_bitbuf_clear(BLBitBuf* buf, int bit);

// tests if the bit at the given offset is set
bool bl_bitbuf_isset(BLBitBuf* buf, int bit);

// sets all bits in the buffer
void bl_bitbuf_set(BLBitBuf* buf);

// sets the bit at the given offset
void bl_bitbuf_set(BLBitBuf* buf, int bit);

// sets the bit at the given offset to the given value
void bl_bitbuf_set(BLBitBuf* buf, int bit, bool val);

#endif
