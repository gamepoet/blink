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

#include <unittest++/UnitTest++.h>
#include <blink/bitbuf.h>

SUITE(bitbuf) {
  //------------------------------------------------------------------------------
  TEST(clear_all) {
    uint32_t data[4];
    BLBitBuf buf;
    bl_bitbuf_init(&buf, data, 38, sizeof(data));
    bl_bitbuf_clear(&buf);

    uint32_t expected[4] = { 0 };
    CHECK_ARRAY_EQUAL(expected, data, 4);
  }

  //------------------------------------------------------------------------------
  TEST(set_all) {
    uint32_t data[4];
    BLBitBuf buf;
    bl_bitbuf_init(&buf, data, 38, sizeof(data));
    bl_bitbuf_set(&buf);

    uint32_t expected[4] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    CHECK_ARRAY_EQUAL(expected, data, 4);
  }

  //------------------------------------------------------------------------------
  TEST(set_and_clear) {
    uint32_t data[4];
    BLBitBuf buf;
    bl_bitbuf_init(&buf, data, 38, sizeof(data));
    bl_bitbuf_clear(&buf);

    // first block
    bl_bitbuf_set(&buf, 12);
    CHECK(bl_bitbuf_isset(&buf, 12));
    bl_bitbuf_clear(&buf, 12);
    CHECK(!bl_bitbuf_isset(&buf, 12));

    // second block
    bl_bitbuf_set(&buf, 37);
    CHECK(bl_bitbuf_isset(&buf, 37));
    bl_bitbuf_clear(&buf, 37);
    CHECK(!bl_bitbuf_isset(&buf, 37));
  }

  //------------------------------------------------------------------------------
  TEST(set_value) {
    uint32_t data[4];
    BLBitBuf buf;
    bl_bitbuf_init(&buf, data, 38, sizeof(data));
    bl_bitbuf_clear(&buf);

    // false over false
    bl_bitbuf_set(&buf, 32, false);
    CHECK(!bl_bitbuf_isset(&buf, 32));

    // true over false
    bl_bitbuf_set(&buf, 32, true);
    CHECK(bl_bitbuf_isset(&buf, 32));

    // true over true
    bl_bitbuf_set(&buf, 32, true);
    CHECK(bl_bitbuf_isset(&buf, 32));

    // false over true
    bl_bitbuf_set(&buf, 32, false);
    CHECK(!bl_bitbuf_isset(&buf, 32));
  }
}

