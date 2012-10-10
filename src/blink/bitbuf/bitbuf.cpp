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

#include <blink/bitbuf.h>

//------------------------------------------------------------------------------
void bl_bitbuf_clear(BLBitBuf* buf) {
  BL_ASSERT_DBG(buf);
  uint32_t* __restrict data     = buf->data;
  uint32_t* __restrict data_end = buf->data + buf->int_count;
  for (; data < data_end; data += 4) {
    *(data)     = 0;
    *(data + 1) = 0;
    *(data + 2) = 0;
    *(data + 3) = 0;
  }
}

//------------------------------------------------------------------------------
void bl_bitbuf_clear(BLBitBuf* buf, int bit) {
  BL_ASSERT_DBG(buf);
  BL_ASSERT_DBG(bit >= 0 && bit < buf->bit_count);
  int int_index   = bit / 32;
  int bit_offset  = bit - (int_index * 32);
  uint32_t* data  = buf->data + int_index;
  uint32_t val    = *data;
  val             &= ~(1 << bit_offset);
  *data           = val;
}

//------------------------------------------------------------------------------
void bl_bitbuf_init(BLBitBuf* __restrict buf, uint32_t* __restrict data, int bit_count, int byte_count) {
  BL_ASSERT(buf);
  BL_ASSERT(BL_IS_ALIGNED(byte_count, 16));
  BL_ASSERT(bit_count < byte_count * 4);

  buf->data       = data;
  buf->bit_count  = bit_count;
  buf->int_count  = byte_count / 4;
}

//------------------------------------------------------------------------------
bool bl_bitbuf_isset(BLBitBuf* __restrict buf, int bit) {
  BL_ASSERT_DBG(buf);
  BL_ASSERT_DBG(bit >= 0 && bit < buf->bit_count);
  int block_index     = bit / 32;
  int bit_offset      = bit - (block_index * 32);
  uint32_t* block     = buf->data + block_index;
  uint32_t block_val  = *block;
  uint32_t mask       = 1 << bit_offset;
  bool result         = (0 != (block_val & mask));
  return result;
}

//------------------------------------------------------------------------------
void bl_bitbuf_set(BLBitBuf* buf) {
  BL_ASSERT_DBG(buf);
  uint32_t* __restrict data     = buf->data;
  uint32_t* __restrict data_end = buf->data + buf->int_count;
  for (; data < data_end; data += 4) {
    *(data)     = 0xffffffff;
    *(data + 1) = 0xffffffff;
    *(data + 2) = 0xffffffff;
    *(data + 3) = 0xffffffff;
  }
}

//------------------------------------------------------------------------------
void bl_bitbuf_set(BLBitBuf* buf, int bit) {
  BL_ASSERT_DBG(buf);
  BL_ASSERT_DBG(bit >= 0 && bit < buf->bit_count);
  int block_index     = bit / 32;
  int bit_offset      = bit - (block_index * 32);
  uint32_t* block     = buf->data + block_index;
  uint32_t block_val  = *block;
  block_val           |= (1 << bit_offset);
  *block              = block_val;
}

//------------------------------------------------------------------------------
void bl_bitbuf_set(BLBitBuf* buf, int bit, bool val) {
  BL_ASSERT_DBG(buf);
  BL_ASSERT_DBG(bit >= 0 && bit < buf->bit_count);
  int block_index     = bit / 32;
  int bit_offset      = bit - (block_index * 32);
  uint32_t* block     = buf->data + block_index;
  uint32_t block_val  = *block;
  uint32_t mask       = ~(1 << bit_offset);
  uint32_t set        = (int)val << bit_offset;
  block_val           = (block_val & mask) | set;
  *block              = block_val;
}
