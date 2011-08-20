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

//------------------------------------------------------------------------------
static void atomic_increment_decrement_test_func(void* param) {
  volatile int32_t* val = (int32_t*)param;
  for (int index = 0; index < 50; ++index) {
    bl_atomic_increment(val);
    bl_atomic_decrement(val);
    bl_atomic_increment(val);
  }
}

//------------------------------------------------------------------------------
static void atomic_add_sub_test_func(void* param) {
  volatile int32_t* val = (int32_t*)param;
  for (int index = 0; index < 50; ++index) {
    bl_atomic_add(val, 11);
    bl_atomic_sub(val, 9);
    bl_atomic_add(val, 3);
  }
}


SUITE(base) {
  //-----------------------------------------------------------------------------
  TEST(atomic_increment_decrement) {
    volatile int32_t val = 0;

    const int thread_count = 5;
    BLThread threads[thread_count];
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_create(threads + index, &atomic_increment_decrement_test_func, (void*)&val);
    }
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_join(threads + index);
    }

    CHECK_EQUAL(50 * thread_count, val);
  }

  //-----------------------------------------------------------------------------
  TEST(atomic_add_sub) {
    volatile int32_t val = 0;
    
    const int thread_count = 5;
    BLThread threads[thread_count];
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_create(threads + index, &atomic_add_sub_test_func, (void*)&val);
    }
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_join(threads + index);
    }
    
    CHECK_EQUAL(250 * thread_count, val);
  }
}
