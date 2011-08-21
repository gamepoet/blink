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

#pragma once
#ifndef BL_QUEUE_H
#define BL_QUEUE_H

#include <base/base.h>

// This SWSR implementation of a bounded queue is simple, but bad on cache-line
// sharing. The basic issue is that both threads want to access the get and put
// members causing the cache line to go back and forth between threads. :(
// When/if this becomes a performance issue, I'll improve the implementation.
struct BLQueueSWSR {
  void* __restrict  buf;
  size_t            capacity;     // total number of elements that could be stored in the queue
  size_t            element_size; // size of each element in the queue
  char              pad1[128];    // next cache line
  volatile int32_t  get;          // index of the read head
  char              pad2[128];    // next cache line
  volatile int32_t  put;          // index of the write head
};

// Initializes a queue with a given buffer.
void bl_queue_init(BLQueueSWSR* __restrict queue, void* __restrict buf, size_t capacity, size_t element_size);

// Tries to fetch an element from the queue. Returns a pointer to the element if
// there was data to read; NULL if the queue is empty. This call only returns a
// pointer to the element in the queue, bl_queue_read_consume() must be called
// to actually advance tho read head.
void* bl_queue_read_fetch(BLQueueSWSR* __restrict queue);

// Consumes an item on the queue. This should only be called after a successful
// call to bl_queue_read_fetch().
void bl_queue_read_consume(BLQueueSWSR* __restrict queue);

// Tries to prepare to write an element to the queue. Returns a pointer to the
// element that can be added; NULL if the queue is full. This call only returns
// a pointer to the new element in the queue, bl_queue_write_commit() must be
// called to actually advance the write head.
void* bl_queue_write_prepare(BLQueueSWSR* __restrict queue, bool* __restrict empty);

// Commits a new element to the queue. This should only be called after a
// successful call to bl_queue_write_prepare().
void bl_queue_write_commit(BLQueueSWSR* __restrict queue);

#endif
