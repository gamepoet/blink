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

#include "base.h"
#include "base_int.h"
#include <stdio.h>


//
// local vars
//

static BLLogLevel s_level;
static FILE*      s_log_fh;


//
// local functions
//

//------------------------------------------------------------------------------
static void log(const char* __restrict format, va_list args) {
  // append a newline to the msg
  char fmt[256];
  bl_sprintf(fmt, sizeof(fmt), "%s\n", format);

  // build the string
  char buf[256];
  bl_vsprintf(buf, sizeof(buf), fmt, args);

  // send to the debugger
  BL_DEBUG_MSG(buf);

  // write to the log
  if (s_log_fh) {
    fputs(buf, s_log_fh);
  }
}


//
// exported functions
//

//------------------------------------------------------------------------------
void log_initialize(const char* log_filename) {
#ifdef NDEBUG
  s_level = BL_LOG_LEVEL_WARN;
#else
  s_level = BL_LOG_LEVEL_DEBUG;
#endif

  // open the log file
  if (log_filename) {
    s_log_fh = fopen(log_filename, "wb");
  }
  else {
    s_log_fh = NULL;
  }
}

//------------------------------------------------------------------------------
void log_finalize() {
  if (s_log_fh) {
    fclose(s_log_fh);
    s_log_fh = NULL;
  }
}

//------------------------------------------------------------------------------
void bl_log_flush() {
  if (s_log_fh) {
    fflush(s_log_fh);
  }
}

//------------------------------------------------------------------------------
void bl_log_set_level(BLLogLevel level) {
  s_level = level;
}

//------------------------------------------------------------------------------
void bl_log_debug(const char* __restrict format, ...) {
  if (s_level >= BL_LOG_LEVEL_DEBUG) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
  }
}

//------------------------------------------------------------------------------
void bl_log_info(const char* __restrict format, ...) {
  if (s_level >= BL_LOG_LEVEL_INFO) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
  }
}

//------------------------------------------------------------------------------
void bl_log_warn(const char* __restrict format, ...) {
  if (s_level >= BL_LOG_LEVEL_WARN) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
  }
}

//------------------------------------------------------------------------------
void bl_log_error(const char* __restrict format, ...) {
  if (s_level >= BL_LOG_LEVEL_ERROR) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
  }
}

//------------------------------------------------------------------------------
void bl_log_fatal(const char* __restrict format, ...) {
  if (s_level >= BL_LOG_LEVEL_FATAL) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
  }
}

//------------------------------------------------------------------------------
void bl_log_debug_v(const char* __restrict format, va_list args) {
  if (s_level >= BL_LOG_LEVEL_DEBUG) {
    log(format, args);
  }
}

//------------------------------------------------------------------------------
void bl_log_info_v(const char* __restrict format, va_list args) {
  if (s_level >= BL_LOG_LEVEL_INFO) {
    log(format, args);
  }
}

//------------------------------------------------------------------------------
void bl_log_warn_v(const char* __restrict format, va_list args) {
  if (s_level >= BL_LOG_LEVEL_WARN) {
    log(format, args);
  }
}

//------------------------------------------------------------------------------
void bl_log_error_v(const char* __restrict format, va_list args) {
  if (s_level >= BL_LOG_LEVEL_ERROR) {
    log(format, args);
  }
}

//------------------------------------------------------------------------------
void bl_log_fatal_v(const char* __restrict format, va_list args) {
  if (s_level >= BL_LOG_LEVEL_FATAL) {
    log(format, args);
  }
}
