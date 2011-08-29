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
#ifndef BL_JOB_H
#define BL_JOB_H

#include <blink/base.h>


//
// constants
//

enum BLJobStatus {
  BL_JOB_STATUS_OK,
  BL_JOB_STATUS_ERR_BAD_PARAM,
  BL_JOB_STATUS_ERR_BAD_ALIGNMENT,
  BL_JOB_STATUS_ERR_FULL,
  BL_JOB_STATUS_ERR_OUT_OF_MEMORY,
};


//
// types
//

struct BLJob;
struct BLJobQueue;

typedef void (*BLJobFunc)(const BLJob* __restrict job);

struct BLJobLibInitParams {
  unsigned int  worker_thread_count;    // number of worker threads to create
};

struct BLJob {
  BLJobFunc               func;   // job work entry point
  const void* __restrict  input;  // input buffer
  void* __restrict        output; // output buffer
  BLJobQueue* __restrict  queue;  // (internal) queue that owns this job
  uint8_t                 user_data[128 - (4 * BL_POINTER_SIZE)];
};
BL_STATIC_ASSERT(sizeof(BLJob) == 128);


//
// library management
//

BLJobStatus bl_job_lib_initialize(BLJobLibInitParams* __restrict params);
void bl_job_lib_finalize();


//
// job queue management
//

// Creates a new queue to hold related jobs. The queue can only be manipulated
// by a single thread.
BLJobQueue* bl_job_queue_create();

// Destroys a job queue. If the queue has pending jobs, this call will block
// until those jobs have completed.
BLJobStatus bl_job_queue_destroy(BLJobQueue* __restrict queue);

// Pushes a job onto the queue.
BLJobStatus bl_job_queue_push_job(BLJobQueue* __restrict queue, BLJob* __restrict job);

// Waits for all jobs in the group to finish.
BLJobStatus bl_job_queue_wait(BLJobQueue* __restrict queue);

#endif
