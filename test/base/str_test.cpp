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
  TEST(strlen) {
    CHECK_EQUAL(0, bl_strlen("", 1));
    CHECK_EQUAL(1, bl_strlen("1", 2));
    CHECK_EQUAL(5, bl_strlen("hello", 6));
    CHECK_EQUAL(11, bl_strlen("hello world", 12));
  }

  //----------------------------------------------------------------------------
  TEST(strcmp) {
    CHECK(bl_strcmp("a", "b", 1) < 0);
    CHECK(bl_strcmp("a", "bb", 1) < 0);
    CHECK(bl_strcmp("hello", "hello", 4) == 0);
    CHECK(bl_strcmp("hello world", "abcd", 4) > 0);
  }

  //----------------------------------------------------------------------------
  TEST(stricmp) {
    CHECK(bl_stricmp("a", "b", 1) < 0);
    CHECK(bl_stricmp("a", "bb", 1) < 0);
    CHECK(bl_stricmp("hello", "HELLO", 4) == 0);
    CHECK(bl_stricmp("hello world", "abcd", 4) > 0);
  }

  //----------------------------------------------------------------------------
  TEST(strcpy) {
    char dest[8];
    size_t ret;

    // empty string
    ret = bl_strcpy(dest, "", sizeof(dest));
    CHECK_EQUAL(0, ret);
    CHECK_EQUAL('\0', dest[0]);
    CHECK_EQUAL("", dest);

    // within bounds
    ret = bl_strcpy(dest, "hello", sizeof(dest));
    CHECK_EQUAL(5, ret);
    CHECK_EQUAL('\0', dest[5]);
    CHECK_EQUAL("hello", dest);

    // filled buffer
    ret = bl_strcpy(dest, "1234567", sizeof(dest));
    CHECK_EQUAL(7, ret);
    CHECK_EQUAL('\0', dest[7]);
    CHECK_EQUAL("1234567", dest);

    // buffer overflow
    ret = bl_strcpy(dest, "7654321", sizeof(dest) - 1);
    CHECK_EQUAL(7, ret);
    CHECK_EQUAL('\0', dest[6]);
    CHECK_EQUAL("765432", dest);
  }

  //----------------------------------------------------------------------------
  TEST(strcat) {
    char dest[6];
    dest[0] = 0;
    size_t ret;

    // empty string
    ret = bl_strcat(dest, "", sizeof(dest));
    CHECK_EQUAL(0, ret);
    CHECK_EQUAL('\0', dest[0]);
    CHECK_EQUAL("", dest);

    // within bounds
    ret = bl_strcat(dest, "hi", sizeof(dest));
    CHECK_EQUAL(2, ret);
    CHECK_EQUAL('\0', dest[2]);
    CHECK_EQUAL("hi", dest);

    // filled buffer
    ret = bl_strcat(dest, "mom", sizeof(dest));
    CHECK_EQUAL(5, ret);
    CHECK_EQUAL('\0', dest[5]);
    CHECK_EQUAL("himom", dest);

    // buffer overflow
    ret = bl_strcat(dest, "ma", sizeof(dest) - 1);
    CHECK_EQUAL(7, ret);
    CHECK_EQUAL('\0', dest[5]);
    CHECK_EQUAL("himom", dest);
  }

  //----------------------------------------------------------------------------
  // tests both bl_sprintf and bl_vsprintf because the former uses the latter.
  TEST(sprintf) {
    char dest[6];
    int ret;

    // empty string
    ret = bl_sprintf(dest, sizeof(dest), "");
    CHECK_EQUAL(0, ret);
    CHECK_EQUAL(0, dest[0]);

    // within bounds
    ret = bl_sprintf(dest, sizeof(dest), "%d", 12);
    CHECK_EQUAL(2, ret);
    CHECK_EQUAL(0, dest[2]);
    CHECK_EQUAL("12", dest);

    // filled buffer
    ret = bl_sprintf(dest, sizeof(dest), "%d", 12345);
    CHECK_EQUAL(5, ret);
    CHECK_EQUAL(0, dest[5]);
    CHECK_EQUAL("12345", dest);

    // buffer overflow
    ret = bl_sprintf(dest, sizeof(dest), "%d", 123456);
    CHECK_EQUAL(6, ret);
    CHECK_EQUAL(0, dest[5]);
    CHECK_EQUAL("12345", dest);
  }
}
