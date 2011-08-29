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
#include <blink/queue.h>

SUITE(queue) {
  //----------------------------------------------------------------------------
  TEST(queue_swsr_can_put_and_get) {
    const int queue_capacity = 4;
    int buf[queue_capacity];
    BLQueueSWSR q;
    bl_queue_init(&q, buf, queue_capacity, sizeof(int));

    int* p;

    // write an item to the queue
    bool empty;
    p = (int*)bl_queue_write_prepare(&q, &empty);
    CHECK(empty);
    CHECK(p);
    *p = 5;
    bl_queue_write_commit(&q);

    // read the item off the queue
    p = (int*)bl_queue_read_fetch(&q);
    CHECK(p);
    CHECK_EQUAL(5, *p);
    bl_queue_read_consume(&q);
  }

  //----------------------------------------------------------------------------
  TEST(queue_swsr_read_on_empty_queue_should_fail) {
    const int queue_capacity = 4;
    int buf[queue_capacity];
    BLQueueSWSR q;
    bl_queue_init(&q, buf, queue_capacity, sizeof(int));

    int* p = (int*)bl_queue_read_fetch(&q);
    CHECK(!p);
  }

  //----------------------------------------------------------------------------
  TEST(queue_swsr_write_on_full_queue_should_fail) {
    const int queue_capacity = 4;
    int buf[queue_capacity];
    BLQueueSWSR q;
    bl_queue_init(&q, buf, queue_capacity, sizeof(int));

    bool empty;
    int* p;

    // success
    p = (int*)bl_queue_write_prepare(&q, &empty);
    CHECK(p);
    CHECK(empty);
    *p = 1;
    bl_queue_write_commit(&q);

    // success
    p = (int*)bl_queue_write_prepare(&q, &empty);
    CHECK(p);
    CHECK(!empty);
    *p = 2;
    bl_queue_write_commit(&q);

    // success
    p = (int*)bl_queue_write_prepare(&q, &empty);
    CHECK(p);
    CHECK(!empty);
    *p = 3;
    bl_queue_write_commit(&q);

    // fail: full
    p = (int*)bl_queue_write_prepare(&q, &empty);
    CHECK(!p);
    CHECK(!empty);
  }
}
