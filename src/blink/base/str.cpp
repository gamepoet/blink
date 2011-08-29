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
#include <string.h>
#include <stdio.h>

//------------------------------------------------------------------------------
size_t bl_strlen(const char* __restrict src, size_t size) {
  return strnlen(src, size);
}

//------------------------------------------------------------------------------
int bl_strcmp(const char* __restrict a, const char* __restrict b, size_t size) {
  return strncmp(a, b, size);
}

//------------------------------------------------------------------------------
int bl_stricmp(const char* __restrict a, const char* __restrict b, size_t size) {
  return strncasecmp(a, b, size);
}

//------------------------------------------------------------------------------
size_t bl_strcpy(char* __restrict dest, const char* __restrict src, size_t size) {
  return strlcpy(dest, src, size);
}

//------------------------------------------------------------------------------
size_t bl_strcat(char* __restrict dest, const char* __restrict src, size_t size) {
  return strlcat(dest, src, size);
}

//------------------------------------------------------------------------------
int bl_sprintf(char* __restrict dest, size_t size, const char* __restrict format, ...) {
  va_list args;
  va_start(args, format);
  int ret = bl_vsprintf(dest, size, format, args);
  va_end(args);
  return ret;
}

//------------------------------------------------------------------------------
int bl_vsprintf(char* __restrict dest, size_t size, const char* __restrict format, va_list args) {
  return vsnprintf(dest, size, format, args);
}
