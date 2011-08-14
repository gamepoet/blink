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

#include "../base.h"
//#include <sys/sysctl.h>
//#include <unistd.h>
//#include <err.h>
//#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <mach/semaphore.h>
#include <mach/task.h>
#include <mach/mach_init.h>

//
// local types
//

struct Thread {
  BLThreadEntryFunc entry_func;
  void*             param;
  pthread_t         handle;
};

// verify struct sizes match
BL_STATIC_ASSERT(sizeof(BLThread) == sizeof(Thread));
BL_STATIC_ASSERT(sizeof(BLMutex) == sizeof(pthread_mutex_t));
BL_STATIC_ASSERT(sizeof(BLCond) == sizeof(pthread_cond_t));
BL_STATIC_ASSERT(sizeof(BLSemaphore) == sizeof(semaphore_t));
BL_STATIC_ASSERT(sizeof(BLThreadSpecificPtr) == sizeof(pthread_key_t));


//
// local functions
//

//------------------------------------------------------------------------------
static void* thread_trampoline(void* param) {
  Thread* thread = (Thread*)param;
  thread->entry_func(thread->param);
  return NULL;
}


//
// exported functions
//

//------------------------------------------------------------------------------
void bl_thread_create(BLThread* __restrict thread, BLThreadEntryFunc func, void* param) {
  BL_ASSERT(thread);
  BL_ASSERT(func);
  Thread* __restrict impl = (Thread*)thread;

  // setup the thread
  impl->entry_func  = func;
  impl->param       = param;

  // setup the pthread attributes
  int ret;
  pthread_attr_t attr;
  ret = pthread_attr_init(&attr);
  BL_ASSERT(ret == 0);

  // create the thread
  ret = pthread_create(&impl->handle, &attr, &thread_trampoline, impl);
  BL_ASSERT(ret == 0);

  // destroy the pthread attributes
  ret = pthread_attr_destroy(&attr);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_thread_join(BLThread* __restrict thread) {
  BL_ASSERT(thread);
  Thread* __restrict impl = (Thread*)thread;

  // wait for the thread to terminate
  int ret;
  ret = pthread_join(impl->handle, NULL);
  BL_ASSERT(ret == 0);

  // cleanup the thread
  impl->entry_func  = NULL;
  impl->param       = NULL;
  impl->handle      = NULL;
}

//------------------------------------------------------------------------------
void bl_thread_set_name(const char* __restrict name) {
  BL_ASSERT(name);

  int ret;
  ret = pthread_setname_np(name);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_mutex_create(BLMutex* __restrict mutex) {
  BL_ASSERT(mutex);
  pthread_mutex_t* __restrict handle = (pthread_mutex_t*)mutex;

  // setup the mutex attributes
  int ret;
  pthread_mutexattr_t attr;
  ret = pthread_mutexattr_init(&attr);
  BL_ASSERT(ret == 0);

  // create the mutex
  ret = pthread_mutex_init(handle, &attr);
  BL_ASSERT(ret == 0);

  // destroy the mutex attributes
  ret = pthread_mutexattr_destroy(&attr);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_mutex_destroy(BLMutex* __restrict mutex) {
  BL_ASSERT(mutex);
  pthread_mutex_t* __restrict handle = (pthread_mutex_t*)mutex;

  int ret;
  ret = pthread_mutex_destroy(handle);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_mutex_lock(BLMutex* __restrict mutex) {
  BL_ASSERT(mutex);
  pthread_mutex_t* __restrict handle = (pthread_mutex_t*)mutex;

  int ret;
  ret = pthread_mutex_lock(handle);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_mutex_unlock(BLMutex* __restrict mutex) {
  BL_ASSERT(mutex);
  pthread_mutex_t* __restrict handle = (pthread_mutex_t*)mutex;

  int ret;
  ret = pthread_mutex_unlock(handle);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_cond_create(BLCond* __restrict cond) {
  BL_ASSERT(cond);
  pthread_cond_t* __restrict handle = (pthread_cond_t*)cond;

  // setup the condition variable attributes
  int ret;
  pthread_condattr_t attr;
  ret = pthread_condattr_init(&attr);
  BL_ASSERT(ret == 0);

  // create the condition variable
  ret = pthread_cond_init(handle, &attr);
  BL_ASSERT(ret == 0);

  // destroy the condition variable attributes
  ret = pthread_condattr_destroy(&attr);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_cond_destroy(BLCond* __restrict cond) {
  BL_ASSERT(cond);
  pthread_cond_t* __restrict handle = (pthread_cond_t*)cond;

  int ret;
  ret = pthread_cond_destroy(handle);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_cond_notify_one(BLCond* __restrict cond) {
  BL_ASSERT(cond);
  pthread_cond_t* __restrict handle = (pthread_cond_t*)cond;

  int ret;
  ret = pthread_cond_signal(handle);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_cond_notify_all(BLCond* __restrict cond) {
  BL_ASSERT(cond);
  pthread_cond_t* __restrict handle = (pthread_cond_t*)cond;

  int ret;
  ret = pthread_cond_broadcast(handle);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_cond_wait(BLCond* __restrict cond, BLMutex* __restrict mutex) {
  BL_ASSERT(cond);
  BL_ASSERT(mutex);
  pthread_cond_t* __restrict cond_handle = (pthread_cond_t*)cond;
  pthread_mutex_t* __restrict mutex_handle = (pthread_mutex_t*)mutex;

  int ret;
  ret = pthread_cond_wait(cond_handle, mutex_handle);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_cond_wait_timeout(BLCond* __restrict cond, BLMutex* __restrict mutex, uint64_t timeout_ms) {
  BL_ASSERT(cond);
  BL_ASSERT(mutex);
  pthread_cond_t* __restrict cond_handle = (pthread_cond_t*)cond;
  pthread_mutex_t* __restrict mutex_handle = (pthread_mutex_t*)mutex;

  int ret;

  // extract the secs and ns separately from the timeout arg
  uint64_t secs = timeout_ms / 1000ULL;
  uint64_t ns   = (timeout_ms - (secs * 1000ULL) * 1000ULL);

  // get the current time
  timeval now;
  ret = gettimeofday(&now, NULL);
  BL_ASSERT(ret == 0);

  // flesh out the timespec required by the pthread api
  timespec wait_time;
  wait_time.tv_sec  = secs;
  wait_time.tv_nsec = ns;

  ret = pthread_cond_timedwait(cond_handle, mutex_handle, &wait_time);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_semaphore_create(BLSemaphore* __restrict semaphore, int initial_value) {
  BL_ASSERT(semaphore);
  semaphore_t* __restrict handle = (semaphore_t*)semaphore;

  kern_return_t ret;
  ret = semaphore_create(mach_task_self(), handle, SYNC_POLICY_FIFO, initial_value);
  BL_ASSERT(ret == KERN_SUCCESS);
}

//------------------------------------------------------------------------------
void bl_semaphore_destroy(BLSemaphore* __restrict semaphore) {
  BL_ASSERT(semaphore);
  semaphore_t* __restrict handle = (semaphore_t*)semaphore;

  kern_return_t ret;
  ret = semaphore_destroy(mach_task_self(), *handle);
  BL_ASSERT(ret == KERN_SUCCESS);
}

//------------------------------------------------------------------------------
void bl_semaphore_post(BLSemaphore* __restrict semaphore) {
  BL_ASSERT(semaphore);
  semaphore_t* __restrict handle = (semaphore_t*)semaphore;

  kern_return_t ret;
  ret = semaphore_signal(*handle);
  BL_ASSERT(ret == KERN_SUCCESS);
}

//------------------------------------------------------------------------------
void bl_semaphore_wait(BLSemaphore* __restrict semaphore) {
  BL_ASSERT(semaphore);
  semaphore_t* __restrict handle = (semaphore_t*)semaphore;

  kern_return_t ret;
  ret = semaphore_wait(*handle);
  BL_ASSERT(ret == KERN_SUCCESS);
}

//------------------------------------------------------------------------------
void bl_thread_specific_ptr_create(BLThreadSpecificPtr* __restrict tsp) {
  BL_ASSERT(tsp);
  pthread_key_t* __restrict handle = (pthread_key_t*)tsp;

  int ret;
  ret = pthread_key_create(handle, NULL);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_thread_specific_ptr_destroy(BLThreadSpecificPtr* __restrict tsp) {
  BL_ASSERT(tsp);
  pthread_key_t* __restrict handle = (pthread_key_t*)tsp;

  int ret;
  ret = pthread_key_delete(*handle);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void bl_thread_specific_ptr_set(BLThreadSpecificPtr* __restrict tsp, void* value) {
  BL_ASSERT(tsp);
  pthread_key_t* __restrict handle = (pthread_key_t*)tsp;

  int ret;
  ret = pthread_setspecific(*handle, value);
  BL_ASSERT(ret == 0);
}

//------------------------------------------------------------------------------
void* bl_thread_specific_ptr_get(BLThreadSpecificPtr* __restrict tsp) {
  BL_ASSERT(tsp);
  pthread_key_t* __restrict handle = (pthread_key_t*)tsp;

  return pthread_getspecific(*handle);
}
