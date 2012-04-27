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

#include "../base.h"
#include "base_int.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysctl.h>

//
// local vars
//

static BLAssertHandler s_assert_handler;


//
// local functions
//

//------------------------------------------------------------------------------
static void signal_handler(int sig) {
  // re-hook the handler in case it gets called again (i.e. multiple threads)
  signal(sig, &signal_handler);

  switch (sig) {
    case SIGABRT:
      BL_FATAL("caught SIGABRT: program aborted");
      break;

    case SIGBUS:
      BL_FATAL("caught SIGBUS: bus error");
      break;

    case SIGFPE:
      BL_FATAL("caught SIGFPE: floating-point exception");
      break;

    case SIGILL:
      BL_FATAL("caught SIGILL: illegal instruction");
      break;

    case SIGPIPE:
      BL_FATAL("caught SIGPIPE: write on a pipe with no reader");
      break;

    case SIGSEGV:
      BL_FATAL("caught SIGSEGV: segmentation violation");
      break;

    case SIGSYS:
      BL_FATAL("caught SIGSYS: non-existent system call invoked");
      break;

    default:
      BL_FATAL("caught unknown signal: generic failure");
      break;
  }
}

//------------------------------------------------------------------------------
static void default_assert_handler(const char* cond, const char* msg, const char* func, const char* file, unsigned int line) {
  BL_DEBUG_MSG(
      "ASSERT: %s%s%s%s%s%s(%d)",
      cond ? cond : "",
      cond ? ". " : "",
      msg ? msg : "",
      msg ? ". " : "",
      func,
      file,
      line
      );

  abort();
}


//
// exported functions
//

//------------------------------------------------------------------------------
void crash_handler_initialize() {
  // hook various signals related to error conditions
  signal(SIGABRT, &signal_handler);
  signal(SIGBUS,  &signal_handler);
  signal(SIGFPE,  &signal_handler);
  signal(SIGILL,  &signal_handler);
  signal(SIGPIPE, &signal_handler);
  signal(SIGSEGV, &signal_handler);
  signal(SIGSYS,  &signal_handler);

  // TODO: Figure out how to hook into more application crashes on OS X
  // - Unhandled exceptions
  //  - Need to first detect that exceptions are compiled in
  // - Pure virtual function calls
  // - stack overflow
  // - std::terminate()
}

//------------------------------------------------------------------------------
void crash_handler_finalize() {
  // remove signals hooks
  signal(SIGABRT, SIG_DFL);
  signal(SIGBUS,  SIG_DFL);
  signal(SIGFPE,  SIG_DFL);
  signal(SIGILL,  SIG_DFL);
  signal(SIGPIPE, SIG_DFL);
  signal(SIGSEGV, SIG_DFL);
  signal(SIGSYS,  SIG_DFL);
}

//------------------------------------------------------------------------------
void bl_set_assert_handler(BLAssertHandler handler) {
  s_assert_handler = handler;
}

//------------------------------------------------------------------------------
void bl_assert_failed(const char* cond, const char* func, const char* file, unsigned int line, const char* format, ...) {
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

  handler(cond, msg, func, file, line);
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

//------------------------------------------------------------------------------
void bl_debug_msg_raw(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

//------------------------------------------------------------------------------
bool bl_debugger_is_attached() {
#ifdef BL_PLATFORM_OSX
  int               junk;
  int               mib[4];
  struct kinfo_proc info;
  size_t            size;

  // initialize the flags so that, if sysctl fails for some bizarre
  // reason, we get a predictable result
  info.kp_proc.p_flag = 0;

  // initialize mib, which tells sysctl the info we want, in this case
  // we're looking for information about a specific process id
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PID;
  mib[3] = getpid();

  // call sysctl
  size = sizeof(info);
  junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

  // we're being debugged if the P_TRACED flag is set
  return ((info.kp_proc.p_flag & P_TRACED) != 0);
#endif
}

