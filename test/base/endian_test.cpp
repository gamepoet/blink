// Copyright (c) 2011, Ben Scott.
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

#include <unittest++/UnitTest++.h>
#include <base/base.h>

SUITE(base) {
  //----------------------------------------------------------------------------
  TEST(endian_swap_u16) {
    CHECK_EQUAL((uint16_t)0x0201, bl_endian_swap((uint16_t)0x0102));

    uint16_t val = 0x3456;
    bl_endian_swap(&val);
    CHECK_EQUAL((uint16_t)0x5634, val);
    
    uint16_t vals[] = { 0x1234, 0x6789 };
    bl_endian_swap(vals, 2);
    CHECK_EQUAL((uint16_t)0x3412, vals[0]);
    CHECK_EQUAL((uint16_t)0x8967, vals[1]);
  }

  //----------------------------------------------------------------------------
  TEST(endian_swap_u32) {
    CHECK_EQUAL(0x04030201U, bl_endian_swap(0x01020304U));
    
    uint32_t val = 0x87654321U;
    bl_endian_swap(&val);
    CHECK_EQUAL(0x21436587U, val);
    
    uint32_t vals[] = { 0x12345678U, 0xabcd4321U };
    bl_endian_swap(vals, 2);
    CHECK_EQUAL(0x78563412U, vals[0]);
    CHECK_EQUAL(0x2143cdabU, vals[1]);
  }

  //----------------------------------------------------------------------------
  TEST(endian_swap_u64) {
    CHECK_EQUAL(0x0807060504030201ULL, bl_endian_swap(0x0102030405060708ULL));
    
    uint64_t val = 0x0123456789abcdefULL;
    bl_endian_swap(&val);
    CHECK_EQUAL(0xefcdab8967452301, val);
    
    uint64_t vals[] = { 0x0102030405060708ULL, 0x8091a2b3c4d5e6f7 };
    bl_endian_swap(vals, 2);
    CHECK_EQUAL(0x0807060504030201ULL, vals[0]);
    CHECK_EQUAL(0xf7e6d5c4b3a29180ULL, vals[1]);
  }

  //----------------------------------------------------------------------------
  TEST(endian_swap_float) {
    union F2I {
      float     f;
      uint32_t  i;
    };
    F2I val, expected;
    val.f = 1.0f;
    expected.i = 0x0000803f;
    val.f = bl_endian_swap(val.f);

    CHECK_EQUAL(expected.i, val.i);
    CHECK_EQUAL(1.0f, bl_endian_swap(bl_endian_swap(1.0f)));
    
    val.f = 1.0f;
    bl_endian_swap(&val.f);
    CHECK_EQUAL(expected.i, val.i);
    bl_endian_swap(&val.f);
    CHECK_EQUAL(1.0f, val.f);
    
    float vals[] = { 1.0f, 1.0f };
    F2I conv;
    bl_endian_swap(vals, 2);
    conv.f = vals[0];
    CHECK_EQUAL(expected.i, conv.i);
    conv.f = vals[1];
    CHECK_EQUAL(expected.i, conv.i);
    bl_endian_swap(vals, 2);
    CHECK_EQUAL(1.0f, vals[0]);
    CHECK_EQUAL(1.0f, vals[1]);
  }
}
