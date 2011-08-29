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

#include "../job.h"
#include "..//queue.h"

//
// macros
//

#define CHECK_PTR(ptr)                                    \
  if (BL_UNLIKELY(!(ptr))) {                              \
    return BL_JOB_STATUS_ERR_BAD_PARAM;                   \
  }                                                       \
  ((void)0)

#define CHECK_ALIGNMENT(ptr, alignment)                   \
  if (BL_UNLIKELY(!BL_IS_ALIGNED_PTR(ptr, alignment))) {  \
    return BL_JOB_STATUS_ERR_BAD_ALIGNMENT;               \
  }                                                       \
  ((void)0)

#define CHECK_PTR_AND_ALIGNMENT(ptr, alignment)           \
  CHECK_PTR(ptr);                                         \
  CHECK_ALIGNMENT(ptr, alignment)


//
// local types
//

struct BLJobQueue {
  volatile int32_t  wait_count;   //
  BLSemaphore       wait_sem;     // used
};


//
// local vars
//

static volatile bool  s_shutdown;
static unsigned int   s_worker_count;
static BLThread*      s_worker_threads;

static BLQueueSWSR    s_queue;
static BLMutex        s_queue_write_lock;
static BLMutex        s_queue_read_lock;
static BLCond         s_queue_not_empty_cond;


//
// local functions
//

//------------------------------------------------------------------------------
static void job_worker_proc(void* param) {
  unsigned int worker_id = (unsigned int)((uintptr_t)param);
  char thread_name[64];
  bl_sprintf(thread_name, sizeof(thread_name), "job worker %u", worker_id);
  bl_thread_set_name(thread_name);

  for (;;) {
    // pull a job off the queue
    BLJob* __restrict job = NULL;
    bl_mutex_lock(&s_queue_read_lock);
    {
      BLJob** __restrict job_ptr = (BLJob**)bl_queue_read_fetch(&s_queue);
      // if the queue is empty, wait until it's full again
      while (BL_UNLIKELY(job_ptr == NULL && !s_shutdown)) {
        bl_cond_wait(&s_queue_not_empty_cond, &s_queue_read_lock);
        job_ptr = (BLJob**)bl_queue_read_fetch(&s_queue);
      }

      // check for out of work and shutdown request
      if (BL_UNLIKELY(!job_ptr && s_shutdown)) {
        // try one more time to get a job
        job_ptr = (BLJob**)bl_queue_read_fetch(&s_queue);
        if (!job_ptr) {
          bl_mutex_unlock(&s_queue_read_lock);
          break;
        }
      }

      job = *job_ptr;
      bl_queue_read_consume(&s_queue);
    }
    bl_mutex_unlock(&s_queue_read_lock);

    // run the job
    BLJobQueue* __restrict queue = job->queue;
    job->func(job);

    // notify completion of the job
    int32_t new_wait_count = bl_atomic_decrement(&queue->wait_count);
    if (BL_UNLIKELY(new_wait_count == 0)) {
      // queue is waiting on the job and this is the last job, wake it up
      bl_semaphore_post(&queue->wait_sem);
    }
  }
}


//
// exported functions
//

//------------------------------------------------------------------------------
BLJobStatus bl_job_lib_initialize(BLJobLibInitParams* __restrict params) {
  CHECK_PTR(params);
  if (params->worker_thread_count == 0) {
    return BL_JOB_STATUS_ERR_BAD_PARAM;
  }

  // create the qlobal job queue
  const size_t buf_count = 512;
  void* queue_buf = bl_alloc(buf_count * sizeof(BLJob*), 128);
  if (BL_UNLIKELY(!queue_buf)) {
    return BL_JOB_STATUS_ERR_OUT_OF_MEMORY;
  }
  bl_queue_init(&s_queue, queue_buf, buf_count, sizeof(BLJob*));
  bl_mutex_create(&s_queue_write_lock);
  bl_mutex_create(&s_queue_read_lock);
  bl_cond_create(&s_queue_not_empty_cond);

  // create the worker threads
  s_shutdown = false;
  s_worker_count = params->worker_thread_count;
  s_worker_threads = (BLThread*)bl_alloc(s_worker_count * sizeof(BLThread), 16);
  for (unsigned int index = 0; index < s_worker_count; ++index) {
    bl_thread_create(s_worker_threads + index, &job_worker_proc, (void*)index);
  }

  return BL_JOB_STATUS_OK;
}

//------------------------------------------------------------------------------
void bl_job_lib_finalize() {
  // kill the workers
  s_shutdown = true;
  bl_mutex_lock(&s_queue_read_lock);
  {
    bl_cond_notify_all(&s_queue_not_empty_cond);
  }
  bl_mutex_unlock(&s_queue_read_lock);
  for (unsigned int index = 0; index < s_worker_count; ++index) {
    bl_thread_join(s_worker_threads + index);
  }
  bl_free(s_worker_threads);
  s_worker_threads = NULL;
  s_worker_count = 0;
  s_shutdown = false;

  // free the global job queue
  bl_free(s_queue.buf);
  bl_mutex_destroy(&s_queue_write_lock);
  bl_mutex_destroy(&s_queue_read_lock);
  bl_cond_destroy(&s_queue_not_empty_cond);
}

//------------------------------------------------------------------------------
BLJobQueue* bl_job_queue_create() {
  BLJobQueue* __restrict queue = (BLJobQueue*)bl_alloc(sizeof(BLJobQueue), 128);
  if (BL_UNLIKELY(!queue)) {
    return NULL;
  }
  queue->wait_count = 1;
  bl_semaphore_create(&queue->wait_sem, 0);
  return queue;
}

//------------------------------------------------------------------------------
BLJobStatus bl_job_queue_destroy(BLJobQueue* __restrict queue) {
  CHECK_PTR_AND_ALIGNMENT(queue, 128);
  BLJobStatus status = bl_job_queue_wait(queue);

  bl_semaphore_destroy(&queue->wait_sem);
  bl_free(queue);
  return status;
}

//------------------------------------------------------------------------------
BLJobStatus bl_job_queue_push_job(BLJobQueue* __restrict queue, BLJob* __restrict job) {
  CHECK_PTR_AND_ALIGNMENT(queue, 128);
  CHECK_PTR_AND_ALIGNMENT(job, 128);

  bl_mutex_lock(&s_queue_write_lock);
  {
    // try to reserve space in the queue
    bool empty;
    BLJob** dest = (BLJob**)bl_queue_write_prepare(&s_queue, &empty);
    if (BL_UNLIKELY(!dest)) {
      bl_mutex_unlock(&s_queue_write_lock);
      return BL_JOB_STATUS_ERR_FULL;
    }

    // add the job to the queue
    bl_atomic_increment(&queue->wait_count);
    job->queue = queue;
    *dest = job;
    bl_queue_write_commit(&s_queue);

    // if the queue was empty, wake the worker threads that there is now work
    // available
    if (BL_UNLIKELY(empty)) {
      bl_cond_notify_all(&s_queue_not_empty_cond);
    }
  }
  bl_mutex_unlock(&s_queue_write_lock);

  return BL_JOB_STATUS_OK;
}

//------------------------------------------------------------------------------
BLJobStatus bl_job_queue_wait(BLJobQueue* __restrict queue) {
  CHECK_PTR_AND_ALIGNMENT(queue, 128);

  int32_t new_wait_count = bl_atomic_decrement(&queue->wait_count);
  if (BL_UNLIKELY(new_wait_count > 0)) {
    // there are still active jobs; wait for them to complete
    bl_semaphore_wait(&queue->wait_sem);
  }

  // reset the wait count to 1
  bl_atomic_increment(&queue->wait_count);

  return BL_JOB_STATUS_OK;
}
