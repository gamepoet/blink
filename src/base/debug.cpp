/// Copyright (c) 2011, Ben Scott.
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

#include "base.h"
#include <stdio.h>

//
// local vars
//

static BLAssertHandler s_assert_handler;


//
// local functions
//

//------------------------------------------------------------------------------
static BLAssertResponse default_assert_handler(const char* cond, const char* msg, const char* file, unsigned int line) {
  BL_DEBUG_MSG(
      "ASSERT: %s. %s%s%s(%d)",
      cond,
      msg ? msg : "",
      msg ? ". " : "",
      file,
      line
      );
  return BL_ASSERT_RESPONSE_HALT;
}


//
// exported functions
//

//------------------------------------------------------------------------------
void bl_set_assert_handler(BLAssertHandler handler) {
  s_assert_handler = handler;
}

//------------------------------------------------------------------------------
BLAssertResponse bl_assert_failed(const char* cond, const char* file, unsigned int line, const char* format, ...) {
  BLAssertHandler handler = s_assert_handler;
  if (!handler) {
    handler = &default_assert_handler;
  }

  char msg[256];
  if (format) {
    va_list args;
    va_start(args, format);
    bl_vsprintf(msg, sizeof(msg), format, args);
    va_end(args);
  }
  else {
    msg[0] = 0;
  }

  return handler(cond, msg, file, line);
}

//------------------------------------------------------------------------------
void bl_debug_msg(const char* format, ...) {
  // tack on a trailing newline
  char new_format[256];
  bl_sprintf(new_format, sizeof(new_format), "%s\n", format);

  va_list args;
  va_start(args, format);
  vprintf(new_format, args);
  va_end(args);
}
