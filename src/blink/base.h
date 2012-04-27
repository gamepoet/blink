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
#ifndef BL_BASE_H
#define BL_BASE_H


//
// architecture and platform
//

enum BL_ARCH_T {
  BL_ARCH_TYPE_PPC64,
  BL_ARCH_TYPE_IA32,
  BL_ARCH_TYPE_X64,
};
enum BL_PLATFORM_T {
  BL_PLATFORM_TYPE_OSX,
  BL_PLATFORM_TYPE_WINDOWS,
};

// determine architecture
#if defined(__ppc64__)
# define BL_ARCH_PPC64
# define BL_ARCH BL_ARCH_TYPE_PPC64
# define BL_BIG_ENDIAN
# define BL_POINTER_SIZE 8

#elif defined(__i386__) || defined(_M_IX86)
# define BL_ARCH_IA32
# define BL_ARCH BL_ARCH_TYPE_IA32
# define BL_LITTLE_ENDIAN
# define BL_POINTER_SIZE 4

#elif defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
# define BL_ARCH_X64
# define BL_ARCH BL_ARCH_TYPE_X64
# define BL_LITTLE_ENDIAN
# define BL_POINTER_SIZE 8

#else
# error unsupported architecture

#endif

// determine platform
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
# define BL_PLATFORM_WINDOWS
# define BL_PLATFORM BL_PLATFORM_TYPE_WINDOWS

#elif defined(__APPLE_CC__)
# define BL_PLATFORM_OSX
# define BL_PLATFORM BL_PLATFORM_TYPE_OSX

#else
# error unsupported platform

#endif


//
// types
//

#include <stddef.h>
#include <stdarg.h>

// standard compilers defines integer types in stdint.h
#ifndef _MSC_VER
# include <stdint.h>
#endif


//
// library management
//

struct BLBaseInitAttr {
  const char* log_filename;
};

void bl_base_lib_initialize(BLBaseInitAttr* attr);
void bl_base_lib_finalize();


//
// memory
//

void* bl_alloc(size_t size, size_t alignment);
void bl_free(void* ptr);


//
// string manipulation
//

size_t bl_strlen(const char* __restrict src, size_t size);
int bl_strcmp(const char* __restrict a, const char* __restrict b, size_t size);
int bl_stricmp(const char* __restrict a, const char* __restrict b, size_t size);
size_t bl_strcpy(char* __restrict dest, const char* __restrict src, size_t size);
size_t bl_strcat(char* __restrict dest, const char* __restrict src, size_t size);

int bl_sprintf(char* __restrict str, size_t size, const char* __restrict format, ...);
int bl_vsprintf(char* __restrict str, size_t size, const char* __restrict format, va_list args);


//
// endian utilities
//

uint16_t bl_endian_swap(uint16_t value);
uint32_t bl_endian_swap(uint32_t value);
uint64_t bl_endian_swap(uint64_t value);
float bl_endian_swap(float value);
void bl_endian_swap(uint16_t* __restrict value);
void bl_endian_swap(uint32_t* __restrict value);
void bl_endian_swap(uint64_t* __restrict value);
void bl_endian_swap(float* __restrict value);
void bl_endian_swap(uint16_t* __restrict values, size_t count);
void bl_endian_swap(uint32_t* __restrict values, size_t count);
void bl_endian_swap(uint64_t* __restrict values, size_t count);
void bl_endian_swap(float* __restrict values, size_t count);

#ifdef BL_BIG_ENDIAN
# define BL_FROM_BIG_ENDIAN(value)        (value)
# define BL_TO_BIG_ENDIAN(value)          (value)
# define BL_FROM_LITTLE_ENDIAN(value)     bl_endian_swap(value)
# define BL_TO_LITTLE_ENDIAN(value)       bl_endian_swap(value)
#else
# define BL_FROM_BIG_ENDIAN(value)        bl_endian_swap(value)
# define BL_TO_BIG_ENDIAN(value)          bl_endian_swap(value)
# define BL_FROM_LITTLE_ENDIAN(value)     (value)
# define BL_TO_LITTLE_ENDIAN(value)       (value)
#endif


//
// generally useful macros
//

// join two tokens together
#define BL_JOIN(a, b)         BL_JOIN_IMPL_1(a, b)
#define BL_JOIN_IMPL_1(a, b)  BL_JOIN_IMPL_2(a, b)
#define BL_JOIN_IMPL_2(a, b)  a ## b

// help the compiler decide which side of a branch is more likely
#ifdef __GNUC__
# define BL_LIKELY(cond)      __builtin_expect(!!(cond), 1)
# define BL_UNLIKELY(cond)    __builtin_expect(!!(cond), 0)
#else
# define BL_LIKELY(cond)      cond
# define BL_UNLIKELY(cond)    cond
#endif

// ensure an intentionally unused expression does not generate a warning
#define BL_UNUSED(x)          do { (void)sizeof(x); } while (0)

// align a value up
#define BL_ALIGN(x, align)              (((x) + (align) - 1) & (~((align) - 1)))
#define BL_ALIGN_PTR(T, p, align)       ((T*)BL_ALIGN((uintptr_t)(p), align))

// align a value down
#define BL_ALIGN_DOWN(x, align)         ((x) & (~((align) - 1)))
#define BL_ALIGN_DOWN_PTR(T, p, align)  ((T*)BL_ALIGN_DOWN((uintptr_t)(p), align))

// check if a value is aligned
#define BL_IS_ALIGNED(x, align)         (((x) & ((align) - 1)) == 0)
#define BL_IS_ALIGNED_PTR(p, align)     BL_IS_ALIGNED((uintptr_t)(p), align)


//
// debug support
//

typedef void (*BLAssertHandler)(const char* cond, const char* msg, const char* func, const char* file, unsigned int line);

// Sets the current assert handler. Set to NULL to reset to the default handler.
void bl_set_assert_handler(BLAssertHandler handler);

// Called by a failing assertion. This function will invoke the current assert handler.
void bl_assert_failed(const char* cond, const char* func, const char* file, unsigned int line, const char* format, ...);

// Prints a message to the debug tty. A newline will be appended to the string.
void bl_debug_msg(const char* format, ...);

// Prints a message to the debug tty as is.
void bl_debug_msg_raw(const char* format, ...);

// Reports whether this program is running under a debugger.
bool bl_debugger_is_attached();

// break into the debugger
#ifdef NDEBUG
# define BL_DEBUG_BREAK()     ((void)0)
#else
# if defined(BL_PLATFORM_WINDOWS)
# define BL_DEBUG_BREAK()     __debug_break()
# elif defined(BL_PLATFORM_OSX)
#   if defined(__ppc__) || defined(__ppc64__)
#     define BL_DEBUG_BREAK() asm { trap }
#   elif defined(__i386) || defined(__x86_64__)
      inline void bl_debug_break() { asm("int3"); }
#     define BL_DEBUG_BREAK() bl_debug_break()
#   else
#     error unsupported architecture
#   endif
# else
#   error unsupported architecture
# endif
#endif

// print debug message in debugger console
#ifdef NDEBUG
# define BL_DEBUG_MSG(...)        ((void)0)
# define BL_DEBUG_MSG_RAW(...)    ((void)0)
#else
# define BL_DEBUG_MSG(...)        bl_debug_msg(__VA_ARGS__)
# define BL_DEBUG_MSG_RAW(...)    bl_debug_msg_raw(__VA_ARGS__)
#endif

// static assert
#define BL_STATIC_ASSERT(b)   enum { BL_JOIN(bl_static_assert_, __LINE__) = sizeof(BL_STATIC_ASSERTION_FAILURE<(bool)(b)>) }
template< bool b >
struct BL_STATIC_ASSERTION_FAILURE;
template<>
struct BL_STATIC_ASSERTION_FAILURE<true> {};

// assertion
#ifdef NDEBUG
# define BL_ASSERT(cond)                (cond)
# define BL_ASSERT_MSG(cond, ...)       (cond)
# define BL_ASSERT_DBG(cond)            do { BL_UNUSED(cond); } while (0)
# define BL_ASSERT_DBG_MSG(cond, ...)   do { BL_UNUSED(cond); } while (0)
# define BL_FATAL(...)                  ((void)0)
#else
# define BL_ASSERT(cond)                                                    \
  (                                                                         \
    BL_LIKELY(cond) ?                                                       \
      true : (                                                              \
        (bl_debugger_is_attached() ? (void)BL_DEBUG_BREAK() : (void)0),     \
        bl_assert_failed(#cond, __func__, __FILE__, __LINE__, 0),           \
        false                                                               \
      )                                                                     \
  )

# define BL_ASSERT_MSG(cond, ...)                                           \
  (                                                                         \
    BL_LIKELY(cond) ?                                                       \
      true : (                                                              \
        (bl_debugger_is_attached() ? (void)BL_DEBUG_BREAK() : (void)0),     \
        bl_assert_failed(#cond, __func__, __FILE__, __LINE__, __VA_ARGS__), \
        false                                                               \
      )                                                                     \
  )

# define BL_ASSERT_DBG(cond)            BL_ASSERT(cond)
# define BL_ASSERT_MSG_DBG(cond, ...)   BL_ASSERT_MSG(cond, __VA_ARGS__)

# define BL_FATAL(...)                                                      \
  BL_DEBUG_BREAK();                                                         \
  bl_assert_failed(0, __func__, __FILE__, __LINE__, __VA_ARGS__)
#endif


//
// log
//

// By default, compile out the log in release
#ifndef BL_LOG_ENABLE
# ifdef NDEBUG
#   ifndef BL_LOG_DISABLE
#     define BL_LOG_DISABLE
#   endif
# endif
#endif

enum BLLogLevel {
  BL_LOG_LEVEL_FATAL,
  BL_LOG_LEVEL_ERROR,
  BL_LOG_LEVEL_WARN,
  BL_LOG_LEVEL_INFO,
  BL_LOG_LEVEL_DEBUG
};

// Flush the log file to disk
void bl_log_flush();

// Sets the log level.
void bl_log_set_level(BLLogLevel level);

// Log functions
#ifndef BL_LOG_DISABLE
# ifdef NDEBUG
#   define bl_log_debug(...)                                ((void)0)
# else
    void bl_log_debug(const char* __restrict format, ...);
# endif
    void bl_log_info(const char* __restrict format, ...);
    void bl_log_warn(const char* __restrict format, ...);
    void bl_log_error(const char* __restrict format, ...);
    void bl_log_fatal(const char* __restrict format, ...);
# ifdef NDEBUG
#   define bl_log_debug_v(...)                              ((void)0)
# else
    void bl_log_debug_v(const char* __restrict format, va_list args);
# endif
    void bl_log_info_v(const char* __restrict format, va_list args);
    void bl_log_warn_v(const char* __restrict format, va_list args);
    void bl_log_error_v(const char* __restrict format, va_list args);
    void bl_log_fatal_v(const char* __restrict format, va_list args);
#else
# define bl_log_debug(...)                                  ((void)0)
# define bl_log_info(...)                                   ((void)0)
# define bl_log_warn(...)                                   ((void)0)
# define bl_log_error(...)                                  ((void)0)
# define bl_log_fatal(...)                                  ((void)0)

# define bl_log_debug_v(...)                                ((void)0)
# define bl_log_info_v(...)                                 ((void)0)
# define bl_log_warn_v(...)                                 ((void)0)
# define bl_log_error_v(...)                                ((void)0)
# define bl_log_fatal_v(...)                                ((void)0)
#endif


//
// threading
//

typedef void (*BLThreadEntryFunc)(void* param);

struct BLThread {
#ifdef BL_PLATFORM_OSX
  char pad[24];
#else
# error unsupported platform
#endif
};
struct BLMutex {
#ifdef BL_PLATFORM_OSX
  char pad[64];
#else
# error unsupported platform
#endif
};
struct BLCond {
#ifdef BL_PLATFORM_OSX
  char pad[48];
#else
# error unsupported platform
#endif
};
struct BLSemaphore {
#ifdef BL_PLATFORM_OSX
  char pad[4];
#else
# error unsupported platform
#endif
};
struct BLThreadSpecificPtr {
#ifdef BL_PLATFORM_OSX
  char pad[8];
#else
# error unsupported platform
#endif
};

void bl_thread_create(BLThread* __restrict thread, BLThreadEntryFunc func, void* param);
void bl_thread_join(BLThread* __restrict thread);
void bl_thread_set_name(const char* __restrict name);

void bl_mutex_create(BLMutex* __restrict mutex);
void bl_mutex_destroy(BLMutex* __restrict mutex);
void bl_mutex_lock(BLMutex* __restrict mutex);
void bl_mutex_unlock(BLMutex* __restrict mutex);

void bl_cond_create(BLCond* __restrict cond);
void bl_cond_destroy(BLCond* __restrict cond);
void bl_cond_notify_one(BLCond* __restrict cond);
void bl_cond_notify_all(BLCond* __restrict cond);
void bl_cond_wait(BLCond* __restrict cond, BLMutex* __restrict mutex);
void bl_cond_wait_timeout(BLCond* __restrict cond, BLMutex* __restrict mutex, uint64_t timeout_ms);

void bl_semaphore_create(BLSemaphore* __restrict semaphore, int initial_value);
void bl_semaphore_destroy(BLSemaphore* __restrict semaphore);
void bl_semaphore_post(BLSemaphore* __restrict semaphore);
void bl_semaphore_wait(BLSemaphore* __restrict semaphore);

void bl_thread_specific_ptr_create(BLThreadSpecificPtr* __restrict tsp);
void bl_thread_specific_ptr_destroy(BLThreadSpecificPtr* __restrict tsp);
void bl_thread_specific_ptr_set(BLThreadSpecificPtr* __restrict tsp, void* value);
void* bl_thread_specific_ptr_get(BLThreadSpecificPtr* __restrict tsp);


//
// atomic ops
//

// Stricly orders memory access such that all loads and stores before the
// barrier will complete before any load or store after the barrier.
void bl_atomic_barrier();

// Compares old_value to *val and sets *val to new_value if the comparison is
// equal. Returns true if *val was set to the new value.
bool bl_atomic_cas(volatile int32_t* val, int32_t old_value, int32_t new_value);
bool bl_atomic_cas(volatile int64_t* val, int64_t old_value, int64_t new_value);
bool bl_atomic_cas(void* volatile* val, void* old_value, void* new_value);

// Increments *val. Returns the result of the operation.
int32_t bl_atomic_increment(volatile int32_t* val);
int64_t bl_atomic_increment(volatile int64_t* val);

// Decrements *val. Returns the result of the operation.
int32_t bl_atomic_decrement(volatile int32_t* val);
int64_t bl_atomic_decrement(volatile int64_t* val);

// Adds the given amount to *val. Returns the result of the operation.
int32_t bl_atomic_add(volatile int32_t* val, int32_t amount);
int64_t bl_atomic_add(volatile int64_t* val, int64_t amount);

// Subtracts the given amount from *val. Returns the result of the operation.
int32_t bl_atomic_sub(volatile int32_t* val, int32_t amount);
int64_t bl_atomic_sub(volatile int64_t* val, int64_t amount);

// Bitwise ANDs the mask to *val. Returns the result of the operation.
uint32_t bl_atomic_and(volatile uint32_t* val, uint32_t mask);
uint64_t bl_atomic_and(volatile uint64_t* val, uint64_t mask);

// Bitwise ORs the mask to *val. Returns the result of the operation.
uint32_t bl_atomic_or(volatile uint32_t* val, uint32_t mask);
uint64_t bl_atomic_or(volatile uint64_t* val, uint64_t mask);

// Bitwise XORs the mask to *val. Returns the result of the operation.
uint32_t bl_atomic_xor(volatile uint32_t* val, uint32_t mask);
uint64_t bl_atomic_xor(volatile uint64_t* val, uint64_t mask);


//
// endian swap implementation
//

//------------------------------------------------------------------------------
inline uint16_t bl_endian_swap(uint16_t value) {
  return  ((value & 0xff00) >> 8) |
          ((value & 0x00ff) << 8);
}

//------------------------------------------------------------------------------
inline uint32_t bl_endian_swap(uint32_t value) {
  return  ((value & 0xff000000) >> 24) |
          ((value & 0x00ff0000) >> 8) |
          ((value & 0x0000ff00) << 8) |
          ((value & 0x000000ff) << 24);
}

//------------------------------------------------------------------------------
inline uint64_t bl_endian_swap(uint64_t value) {
  return  ((value & 0xff00000000000000) >> 56) |
          ((value & 0x00ff000000000000) >> 40) |
          ((value & 0x0000ff0000000000) >> 24) |
          ((value & 0x000000ff00000000) >> 8) |
          ((value & 0x00000000ff000000) << 8) |
          ((value & 0x0000000000ff0000) << 24) |
          ((value & 0x000000000000ff00) << 40) |
          ((value & 0x00000000000000ff) << 56);
}

//------------------------------------------------------------------------------
inline float bl_endian_swap(float value) {
  union F2I {
    float     f;
    uint32_t  i;
  };
  F2I conv;
  conv.f = value;
  conv.i = bl_endian_swap(conv.i);
  return conv.f;
}

//------------------------------------------------------------------------------
inline void bl_endian_swap(uint16_t* __restrict value) {
  *value = bl_endian_swap(*value);
}

//------------------------------------------------------------------------------
inline void bl_endian_swap(uint32_t* __restrict value) {
  *value = bl_endian_swap(*value);
}

//------------------------------------------------------------------------------
inline void bl_endian_swap(uint64_t* __restrict value) {
  *value = bl_endian_swap(*value);
}

//------------------------------------------------------------------------------
inline void bl_endian_swap(float* __restrict value) {
  *value = bl_endian_swap(*value);
}

//------------------------------------------------------------------------------
inline void bl_endian_swap(uint16_t* __restrict values, size_t count) {
  uint16_t* __restrict end = values + count;
  for (; values != end; ++values) {
    bl_endian_swap(values);
  }
}

//------------------------------------------------------------------------------
inline void bl_endian_swap(uint32_t* __restrict values, size_t count) {
  uint32_t* __restrict end = values + count;
  for (; values != end; ++values) {
    bl_endian_swap(values);
  }
}

//------------------------------------------------------------------------------
inline void bl_endian_swap(uint64_t* __restrict values, size_t count) {
  uint64_t* __restrict end = values + count;
  for (; values != end; ++values) {
    bl_endian_swap(values);
  }
}

//------------------------------------------------------------------------------
inline void bl_endian_swap(float* __restrict values, size_t count) {
  float* __restrict end = values + count;
  for (; values != end; ++values) {
    bl_endian_swap(values);
  }
}


//
// atomic ops implementation
//

#if defined(BL_PLATFORM_OSX)
#include <libkern/OSAtomic.h>

//------------------------------------------------------------------------------
inline void bl_atomic_barrier() {
  OSMemoryBarrier();
}

//------------------------------------------------------------------------------
inline bool bl_atomic_cas(volatile int32_t* val, int32_t old_value, int32_t new_value) {
  return OSAtomicCompareAndSwap32(old_value, new_value, val);
}

//------------------------------------------------------------------------------
inline bool bl_atomic_cas(volatile int64_t* val, int64_t old_value, int64_t new_value) {
  return OSAtomicCompareAndSwap64(old_value, new_value, val);
}

//------------------------------------------------------------------------------
inline bool bl_atomic_cas(void* volatile* val, void* old_value, void* new_value) {
  return OSAtomicCompareAndSwapPtr(old_value, new_value, val);
}

//------------------------------------------------------------------------------
inline int32_t bl_atomic_increment(volatile int32_t* val) {
  return OSAtomicIncrement32(val);
}

//------------------------------------------------------------------------------
inline int64_t bl_atomic_increment(volatile int64_t* val) {
  return OSAtomicIncrement64(val);
}

//------------------------------------------------------------------------------
inline int32_t bl_atomic_decrement(volatile int32_t* val) {
  return OSAtomicDecrement32(val);
}

//------------------------------------------------------------------------------
inline int64_t bl_atomic_decrement(volatile int64_t* val) {
  return OSAtomicDecrement64(val);
}

//------------------------------------------------------------------------------
inline int32_t bl_atomic_add(volatile int32_t* val, int32_t amount) {
  return OSAtomicAdd32(amount, val);
}

//------------------------------------------------------------------------------
inline int64_t bl_atomic_add(volatile int64_t* val, int64_t amount) {
  return OSAtomicAdd64(amount, val);
}

//------------------------------------------------------------------------------
inline int32_t bl_atomic_sub(volatile int32_t* val, int32_t amount) {
  return OSAtomicAdd32(-amount, val);
}

//------------------------------------------------------------------------------
inline int64_t bl_atomic_sub(volatile int64_t* val, int64_t amount) {
  return OSAtomicAdd64(-amount, val);
}

//------------------------------------------------------------------------------
inline uint32_t bl_atomic_and(volatile uint32_t* val, uint32_t mask) {
  return OSAtomicAnd32(mask, val);
}

//------------------------------------------------------------------------------
inline uint64_t bl_atomic_and(volatile uint64_t* val, uint64_t mask) {
  for (;;) {
    uint64_t old_val = *val;
    uint64_t new_val = old_val & mask;
    if (bl_atomic_cas((volatile int64_t*)val, (int64_t)old_val, (int64_t)new_val)) {
      return new_val;
    }
  }
}

//------------------------------------------------------------------------------
inline uint32_t bl_atomic_or(volatile uint32_t* val, uint32_t mask) {
  return OSAtomicOr32(mask, val);
}

//------------------------------------------------------------------------------
inline uint64_t bl_atomic_or(volatile uint64_t* val, uint64_t mask) {
  for (;;) {
    uint64_t old_val = *val;
    uint64_t new_val = old_val | mask;
    if (bl_atomic_cas((volatile int64_t*)val, (int64_t)old_val, (int64_t)new_val)) {
      return new_val;
    }
  }
}

//------------------------------------------------------------------------------
inline uint32_t bl_atomic_xor(volatile uint32_t* val, uint32_t mask) {
  return OSAtomicXor32(mask, val);
}

//------------------------------------------------------------------------------
inline uint64_t bl_atomic_xor(volatile uint64_t* val, uint64_t mask) {
  for (;;) {
    uint64_t old_val = *val;
    uint64_t new_val = old_val ^ mask;
    if (bl_atomic_cas((volatile int64_t*)val, (int64_t)old_val, (int64_t)new_val)) {
      return new_val;
    }
  }
}

#elif defined(BL_PLATFORM_WINDOWS)

//------------------------------------------------------------------------------
inline void atomic_barrier() {
  MemoryBarrier();
}

//------------------------------------------------------------------------------
inline bool atomic_cas(volatile int32_t* val, int32_t old_value, int32_t new_value) {
  int32_t orig = InterlockedCompareExchange(val, new_value, old_value);
  return (orig == old_value);
}

//------------------------------------------------------------------------------
inline bool atomic_cas(volatile int64_t* val, int64_t old_value, int64_t new_value) {
  int64_t orig = InterlockedCompareExchange64(val, new_value, old_value);
  return (orig == old_value);
}

//------------------------------------------------------------------------------
inline bool atomic_cas(void* volatile* val, void* old_value, void* new_value) {
  void* orig = InterlockedCompareExchange64(val, new_value, old_value);
  return (orig == old_value);
}

//------------------------------------------------------------------------------
inline int32_t atomic_increment(volatile int32_t* val) {
  return InterlockedIncrement(val);
}

//------------------------------------------------------------------------------
inline int64_t atomic_increment(volatile int64_t* val) {
  return InterlockedIncrement64(val);
}

//------------------------------------------------------------------------------
inline int32_t atomic_decrement(volatile int32_t* val) {
  return InterlockedDecrement(val);
}

//------------------------------------------------------------------------------
inline int64_t atomic_decrement(volatile int64_t* val) {
  return InterlockedDecrement64(val);
}

//------------------------------------------------------------------------------
inline int32_t atomic_add(volatile int32_t* val, int32_t amount) {
  return InterlockedAdd(val, amount);
}

//------------------------------------------------------------------------------
inline int64_t atomic_add(volatile int64_t* val, int64_t amount) {
  return InterlockedAdd64(val, amount);
}

//------------------------------------------------------------------------------
inline int32_t atomic_sub(volatile int32_t* val, int32_t amount) {
  return InterlockedAdd(val, -amount);
}

//------------------------------------------------------------------------------
inline int64_t atomic_sub(volatile int64_t* val, int64_t amount) {
  return InterlockedAdd64(val, -amount);
}

//------------------------------------------------------------------------------
inline uint32_t atomic_and(volatile uint32_t* val, uint32_t mask) {
  uint32_t orig = InterlockedAnd(val, mask);
  return orig & mask;
}

//------------------------------------------------------------------------------
inline uint64_t atomic_and(volatile uint64_t* val, uint64_t mask) {
  uint64_t orig = InterlockedAnd(val, mask);
  return orig & mask;
}

//------------------------------------------------------------------------------
inline uint32_t atomic_or(volatile uint32_t* val, uint32_t mask) {
  uint32_t orig = InterlockedOr(val, mask);
  return orig | mask;
}

//------------------------------------------------------------------------------
inline uint64_t atomic_or(volatile uint64_t* val, uint64_t mask) {
  uint64_t orig = InterlockedOr64(val, mask);
  return orig | mask;
}

//------------------------------------------------------------------------------
inline uint32_t atomic_xor(volatile uint32_t* val, uint32_t mask) {
  uint32_t orig = InterlockedXor(val, mask);
  return orig ^ mask;
}

//------------------------------------------------------------------------------
inline uint64_t atomic_xor(volatile uint64_t* val, uint64_t mask) {
  uint64_t orig = InterlockedXor64(val, mask);
  return orig ^ mask;
}

#else
# error unsupported platform
#endif


#endif
