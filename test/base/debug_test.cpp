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

static bool s_assert_fired;

//------------------------------------------------------------------------------
static BLAssertResponse assert_handler(const char* cond, const char* msg, const char* file, unsigned int line) {
  s_assert_fired = true;
  return BL_ASSERT_RESPONSE_CONTINUE;
}

SUITE(base) {
  //----------------------------------------------------------------------------
  TEST(assert) {
    // hook the assert handler
    bl_set_assert_handler(&assert_handler);

    // success
    s_assert_fired = false;
    BL_ASSERT(true);
    CHECK(!s_assert_fired);

    s_assert_fired = false;
    BL_ASSERT_MSG(true, "oh yeah!");
    CHECK(!s_assert_fired);

    // fail
    s_assert_fired = false;
    BL_ASSERT(false);
    CHECK(s_assert_fired);

    s_assert_fired = false;
    BL_ASSERT_MSG(false, "oh crap");
    CHECK(s_assert_fired);

    // fatal
    s_assert_fired = false;
    BL_FATAL("oh crap");
    CHECK(s_assert_fired);

    // unhook the assert handler
    bl_set_assert_handler(NULL);
  }
}
