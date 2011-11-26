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
#include <blink/hash.h>

SUITE(hash) {
  //------------------------------------------------------------------------------
  TEST(lookup3) {
    uint32_t hash;

    hash = bl_hash_lookup3("", 0, 0);
    CHECK_EQUAL(0xdeadbeef, hash);

    hash = bl_hash_lookup3("Four score and seven years ago", 30, 0);
    CHECK_EQUAL(0x17770551, hash);

    hash = bl_hash_lookup3("Four score and seven years ago", 30, 1);
    CHECK_EQUAL(0xcd628161, hash);
  }

  //------------------------------------------------------------------------------
  TEST(murmur3) {
    uint32_t hash;
    hash = bl_hash_murmur3("", 0, 0);
    CHECK_EQUAL(0x00000000, hash);

    hash = bl_hash_murmur3("Four score and seven years ago", 30, 0);
    CHECK_EQUAL(0xf790a4e0, hash);
    
    hash = bl_hash_murmur3("Four score and seven years ago", 30, 1);
    CHECK_EQUAL(0x657962e5, hash);
  }
}
