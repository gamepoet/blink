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

#include "queue.h"

//
// exported functions
//

//------------------------------------------------------------------------------
void bl_queue_init(BLQueueSWSR* __restrict queue, void* __restrict buf, size_t capacity, size_t element_size) {
  queue->buf          = buf;
  queue->capacity     = capacity;
  queue->element_size = element_size;
  queue->get          = 0;
  queue->put          = 0;
}

//------------------------------------------------------------------------------
void* bl_queue_read_fetch(BLQueueSWSR* __restrict queue) {
  int32_t put = queue->put;
  bl_atomic_barrier();
  int32_t get = queue->get;

  // check empty queue
  if (get == put) {
    return NULL;
  }

  return (uint8_t*)queue->buf + (queue->element_size * get);
}

//------------------------------------------------------------------------------
void bl_queue_read_consume(BLQueueSWSR* __restrict queue) {
  // advance the get index
  int32_t get = queue->get + 1;
  if (BL_UNLIKELY(get >= queue->capacity)) {
    get = 0;
  }
  queue->get = get;
}

//------------------------------------------------------------------------------
void* bl_queue_write_prepare(BLQueueSWSR* __restrict queue, bool* __restrict empty) {
  int32_t get          = queue->get;
  bl_atomic_barrier();
  int32_t put          = queue->put;
  int32_t put_plus_one = put + 1;
  size_t capacity      = queue->capacity;

  // check full queue
  if (BL_UNLIKELY((put_plus_one == get) || ((get == 0) && (put_plus_one == capacity)))) {
    *empty = false;
    return NULL;
  }

  *empty = (get == put);
  return (uint8_t*)queue->buf + (queue->element_size * put);
}

//------------------------------------------------------------------------------
void bl_queue_write_commit(BLQueueSWSR* __restrict queue) {
  // advance the put index
  int32_t put = queue->put + 1;
  if (BL_UNLIKELY(put >= queue->capacity)) {
    put = 0;
  }
  queue->put = put;
}

