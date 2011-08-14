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

struct MutexTestParam {
  intptr_t    count;
  intptr_t    val;
  BLMutex     mutex;
};

struct CondNotifyOneTestParam {
  BLMutex*        mutex;
  BLCond*         ready_cond;
  BLCond*         done_cond;
  volatile bool*  ready;
  bool            done;
};

struct CondNotifyAllTestParam {
  BLMutex*        mutex;
  BLCond*         cond;
  volatile bool*  ready;
  bool            done;
};

struct SemaphoreTestParam {
  BLSemaphore*  sem0;
  BLSemaphore*  sem1;
};

struct ThreadSpecificPtrTestParam {
  BLThreadSpecificPtr*  tsp;
  bool                  success;
};

//------------------------------------------------------------------------------
static void thread_test_func(void* param) {
  *((uintptr_t*)param) = 5;
}

//------------------------------------------------------------------------------
static void mutex_test_func(void* param) {
  MutexTestParam* p = (MutexTestParam*)param;
  const intptr_t count = p->count;
  for (intptr_t index = 0; index < count; ++index) {
    bl_mutex_lock(&p->mutex);
    {
      p->val = p->val + 1;
    }
    bl_mutex_unlock(&p->mutex);
  }
}

//------------------------------------------------------------------------------
static void cond_notify_one_test_func(void* param) {
  CondNotifyOneTestParam* p = (CondNotifyOneTestParam*)param;

  bl_mutex_lock(p->mutex);
  {
    while (!(*p->ready)) {
      bl_cond_wait(p->ready_cond, p->mutex);
    }
    *p->ready = false;
    p->done = true;
  }
  bl_mutex_unlock(p->mutex);

  // wake the main thread
  bl_cond_notify_one(p->done_cond);
}

//------------------------------------------------------------------------------
static void cond_notify_all_test_func(void* param) {
  CondNotifyAllTestParam* p = (CondNotifyAllTestParam*)param;

  bl_mutex_lock(p->mutex);
  {
    while (!(*p->ready)) {
      bl_cond_wait(p->cond, p->mutex);
    }
    p->done = true;
  }
  bl_mutex_unlock(p->mutex);
}

//------------------------------------------------------------------------------
static void semaphore_test_func(void* param) {
  SemaphoreTestParam* p = (SemaphoreTestParam*)param;
  bl_semaphore_wait(p->sem0);
  bl_semaphore_post(p->sem1);
}

//------------------------------------------------------------------------------
static void thread_specific_ptr_test_func(void* param) {
  ThreadSpecificPtrTestParam* p = (ThreadSpecificPtrTestParam*)param;
  bl_thread_specific_ptr_set(p->tsp, (void*)1);
  p->success = (1 == (uintptr_t)bl_thread_specific_ptr_get(p->tsp));
}

SUITE(base) {
  //----------------------------------------------------------------------------
  TEST(thread) {
    uintptr_t val = 1;
    BLThread thread;
    bl_thread_create(&thread, &thread_test_func, &val);
    bl_thread_join(&thread);
    CHECK_EQUAL(5, val);
  }

  //----------------------------------------------------------------------------
  TEST(mutex) {
    MutexTestParam p;
    p.count = 200;
    p.val = 0;
    bl_mutex_create(&p.mutex);

    const int thread_count = 5;
    BLThread threads[thread_count];
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_create(threads + index, &mutex_test_func, &p);
    }
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_join(threads + index);
    }

    CHECK_EQUAL(1000, p.val);

    bl_mutex_destroy(&p.mutex);
  }

  //----------------------------------------------------------------------------
  TEST(cond_notify_one) {
    BLMutex ready_mutex, done_mutex;
    BLCond ready_cond, done_cond;
    bl_mutex_create(&ready_mutex);
    bl_mutex_create(&done_mutex);
    bl_cond_create(&ready_cond);
    bl_cond_create(&done_cond);

    volatile bool ready = false;

    CondNotifyOneTestParam p0;
    CondNotifyOneTestParam p1;
    p0.ready      = &ready;
    p1.ready      = &ready;
    p0.done       = false;
    p1.done       = false;
    p0.mutex      = &ready_mutex;
    p1.mutex      = &ready_mutex;
    p0.ready_cond = &ready_cond;
    p1.ready_cond = &ready_cond;
    p0.done_cond  = &done_cond;
    p1.done_cond  = &done_cond;

    BLThread thread0, thread1;
    bl_thread_create(&thread0, &cond_notify_one_test_func, &p0);
    bl_thread_create(&thread1, &cond_notify_one_test_func, &p1);

    // wake one thread and wait for it to do some work
    ready = true;
    bl_cond_notify_one(&ready_cond);
    bl_mutex_lock(&done_mutex);
    {
      while (!p0.done && !p1.done) {
        bl_cond_wait(&done_cond, &done_mutex);
      }
    }
    bl_mutex_unlock(&done_mutex);
    CHECK(p0.done || p1.done);
    CHECK(p0.done != p1.done);

    // wake the other thread
    ready = true;
    bl_cond_notify_one(&ready_cond);
    bl_mutex_lock(&done_mutex);
    {
      bl_cond_wait(&done_cond, &done_mutex);
    }
    bl_mutex_unlock(&done_mutex);
    CHECK(p0.done && p1.done);

    // cleanup
    bl_thread_join(&thread0);
    bl_thread_join(&thread1);

    bl_cond_destroy(&ready_cond);
    bl_cond_destroy(&done_cond);
    bl_mutex_destroy(&ready_mutex);
    bl_mutex_destroy(&done_mutex);
  }

  //----------------------------------------------------------------------------
  TEST(cond_notify_all) {
    BLMutex mutex;
    BLCond cond;

    bl_mutex_create(&mutex);
    bl_cond_create(&cond);

    bool ready = false;

    const int thread_count = 5;
    CondNotifyAllTestParam params[thread_count];
    for (int index = 0; index < thread_count; ++index) {
      CondNotifyAllTestParam* p = params + index;
      p->mutex = &mutex;
      p->cond = &cond;
      p->done = false;
      p->ready = &ready;
    }

    // spin up the threads
    BLThread threads[thread_count];
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_create(threads + index, &cond_notify_all_test_func, params + index);
    }

    // wake all threads
    ready = true;
    bl_cond_notify_all(&cond);

    // join all the threads
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_join(threads + index);
    }

    for (int index = 0; index < thread_count; ++index) {
      CHECK(params[index].done);
    }

    bl_mutex_destroy(&mutex);
    bl_cond_destroy(&cond);
  }

  //----------------------------------------------------------------------------
  TEST(semaphore) {
    BLSemaphore sem0, sem1;

    SemaphoreTestParam p;
    p.sem0 = &sem0;
    p.sem1 = &sem1;

    bl_semaphore_create(&sem0, 0);
    bl_semaphore_create(&sem1, 0);

    // spin up another thread
    BLThread thread;
    bl_thread_create(&thread, &semaphore_test_func, &p);

    bl_semaphore_post(&sem0);
    bl_semaphore_wait(&sem1);

    bl_thread_join(&thread);

    bl_semaphore_destroy(&sem0);
    bl_semaphore_destroy(&sem1);
  }

  //----------------------------------------------------------------------------
  TEST(thread_specific_ptr) {
    BLThreadSpecificPtr tsp;
    bl_thread_specific_ptr_create(&tsp);

    const int thread_count = 5;
    ThreadSpecificPtrTestParam params[thread_count];
    for (int index = 0; index < thread_count; ++index) {
      ThreadSpecificPtrTestParam* p = params + index;
      p->tsp = &tsp;
      p->success = false;
    }

    BLThread threads[thread_count];
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_create(threads + index, &thread_specific_ptr_test_func, params + index);
    }

    // join all the threads
    for (int index = 0; index < thread_count; ++index) {
      bl_thread_join(threads + index);
    }

    for (int index = 0; index < thread_count; ++index) {
      CHECK(params[index].success);
    }

    bl_thread_specific_ptr_destroy(&tsp);
  }
}
