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
  BL_ARCH_PPC64,
  BL_ARCH_IA32,
  BL_ARCH_X64,
};
enum BL_PLATFORM_T {
  BL_PLATFORM_TYPE_OSX,
  BL_PLATFORM_TYPE_WINDOWS,
};

// determine architecture
#if defined(__ppc64__)
# define BL_ARCH BL_ARCH_PPC64
# define BL_BIG_ENDIAN
# define BL_POINTER_SIZE 8

#elif defined(__i386__) || defined(_M_IX86)
# define BL_ARCH BL_ARCH_IA32
# define BL_LITTLE_ENDIAN
# define BL_POINTER_SIZE 4

#elif defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
# define BL_ARCH BL_ARCH_X64
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
# define BL_UNUSED(x)         do { (void)sizeof(x); } while (0)


//
// debug support
//

// Response codes from an assert handler.
enum BLAssertResponse {
  BL_ASSERT_RESPONSE_HALT,
  BL_ASSERT_RESPONSE_CONTINUE,
};

typedef BLAssertResponse (*BLAssertHandler)(const char* cond, const char* msg, const char* file, unsigned int line);

// Sets the current assert handler. Set to NULL to reset to the default handler.
void bl_set_assert_handler(BLAssertHandler handler);

// Called by a failing assertion. This function will invoke the current assert handler.
BLAssertResponse bl_assert_failed(const char* cond, const char* file, unsigned int line, const char* format, ...);

// Prints a message to the debug tty.
void bl_debug_msg(const char* format, ...);

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
#     define BL_DEBUG_BREAK() asm("int $3")
#   else
#     error unsupported architecture
#   endif
# else
#   error unsupported architecture
# endif
#endif

// print debug message in debugger console
#ifdef NDEBUG
# define BL_DEBUG_MSG(fmt, ...)   ((void)0)
#else
# define BL_DEBUG_MSG(fmt, ...)   bl_debug_msg(fmt, __VA_ARGS__)
#endif

// static assert
#define BL_STATIC_ASSERT(b)   enum { BL_JOIN(bl_static_assert_, __LINE__) = sizeof(bl::STATIC_ASSERTION_FAILURE<(bool)(b)>) }
template< bool b >
struct BL_STATIC_ASSERTION_FAILURE;
template<>
struct BL_STATIC_ASSERTION_FAILURE<true> {};

// assertion
#ifdef NDEBUG
# define BL_ASSERT(cond)                do { BL_UNUSED(cond); } while (0)
# define BL_ASSERT_MSG(cond, ...)       do { BL_UNUSED(cond); } while (0)
# define BL_FATAL(...)                  ((void)0)
#else
# define BL_ASSERT(cond)                                                                          \
  do {                                                                                            \
    if (BL_UNLIKELY(!(cond))) {                                                                   \
      if (bl_assert_failed(#cond, __FILE__, __LINE__, 0) == BL_ASSERT_RESPONSE_HALT) {            \
        BL_DEBUG_BREAK();                                                                         \
      }                                                                                           \
    }                                                                                             \
  }                                                                                               \
  while (0)
# define BL_ASSERT_MSG(cond, ...)                                                                 \
  do {                                                                                            \
    if (BL_UNLIKELY(!(cond))) {                                                                   \
      if (bl_assert_failed(#cond, __FILE__, __LINE__, __VA_ARGS__) == BL_ASSERT_RESPONSE_HALT) {  \
        BL_DEBUG_BREAK();                                                                         \
      }                                                                                           \
    }                                                                                             \
  }                                                                                               \
  while (0)
# define BL_FATAL(...)                                                                            \
  do {                                                                                            \
    if (bl_assert_failed(0, __FILE__, __LINE__, __VA_ARGS__) == BL_ASSERT_RESPONSE_HALT) {        \
      BL_DEBUG_BREAK();                                                                           \
    }                                                                                             \
  }                                                                                               \
  while (0)
#endif


//
// implementations
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

#endif
