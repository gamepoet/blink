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
#include <job/job.h>

struct UserData {
  volatile int32_t * completed_count;
};

//------------------------------------------------------------------------------
static void job_func(const BLJob* __restrict job) {
  UserData* __restrict ud = (UserData*)job->user_data;
  bl_atomic_increment(ud->completed_count);
}

SUITE(job) {
  //----------------------------------------------------------------------------
  TEST(sync_should_wait_for_all_jobs) {
    BLJobStatus status;

    BLJobLibInitParams param;
    param.worker_thread_count = 2;
    status = bl_job_lib_initialize(&param);
    CHECK_EQUAL(BL_JOB_STATUS_OK, status);

    BLJobQueue* queue = bl_job_queue_create();
    CHECK(queue);

    volatile int32_t completed_count = 0;
    UserData ud;
    ud.completed_count = &completed_count;

    const int job_count = 5;
    BLJob* jobs = (BLJob*)bl_alloc(sizeof(BLJob) * job_count, 128);

    // setup the jobs
    for (int index = 0; index < job_count; ++index) {
      BLJob* job = jobs + index;
      job->func       = &job_func;
      job->input      = NULL;
      job->output     = NULL;
      UserData * ud = (UserData*)job->user_data;
      ud->completed_count = &completed_count;
    }

    // run each job and wait for each in sequence
    for (int32_t index = 0; index < job_count; ++index) {
      BLJob* job = jobs + index;
      status = bl_job_queue_push_job(queue, job);
      CHECK_EQUAL(BL_JOB_STATUS_OK, status);
      status = bl_job_queue_wait(queue);
      CHECK_EQUAL(BL_JOB_STATUS_OK, status);
      CHECK_EQUAL(index + 1, completed_count);
    }

    // run all jobs then wait once
    completed_count = 0;
    for (int32_t index = 0; index < job_count; ++index) {
      BLJob* job = jobs + index;
      status = bl_job_queue_push_job(queue, job);
      CHECK_EQUAL(BL_JOB_STATUS_OK, status);
    }
    status = bl_job_queue_wait(queue);
    CHECK_EQUAL(BL_JOB_STATUS_OK, status);
    CHECK_EQUAL(job_count, completed_count);

    // cleanup
    status = bl_job_queue_destroy(queue);
    CHECK_EQUAL(BL_JOB_STATUS_OK, status);

    bl_job_lib_finalize();
  }
}
