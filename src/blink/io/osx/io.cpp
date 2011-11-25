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

#include "../../io.h"
#include <cstdio>
#include <cerrno>
#include <queue>

//
// constants
//

enum IoOpType {
  IO_OP_TYPE_CLOSE,
  IO_OP_TYPE_OPEN,
  IO_OP_TYPE_READ,
};


//
// local types
//

struct IoOpImpl : public BLIoOp {
  BLMutex   complete_mutex;
  BLCond    complete_cond;
  IoOpType  op_type;
};

struct BLIoFile {
  char      file_name[BL_IO_MAX_FILE_NAME_LENGTH];
  uint64_t  offset;
  FILE*     handle;
};


//
// local vars
//

static std::queue<IoOpImpl*>  s_work_queue;
static BLMutex                s_work_queue_mutex;
static BLCond                 s_work_queue_not_empty_cond;

static volatile bool          s_thread_quit;
static BLThread               s_thread;

// setup a default op attribute
static BLIoOpAttr             s_default_op_attr = {
  NULL,
  NULL
};


//
// local functions
//

//----------------------------------------------------------------------------
static void queue_op(IoOpImpl* op) {
  // queue the op
  bl_mutex_lock(&s_work_queue_mutex);
  {
    s_work_queue.push(op);
  }
  bl_mutex_unlock(&s_work_queue_mutex);

  // wake the worker thread if needed
  bl_cond_notify_one(&s_work_queue_not_empty_cond);
}

//----------------------------------------------------------------------------
// called whenever an op completes whether successfully or with error.
// this will signal completion of the op and issue any associated
// callbacks.
static void mark_op_complete(IoOpImpl* op, BLIoStatus status) {
  // update the op's status
  op->status = status;

  // signal completion of the op
  bl_cond_notify_all(&op->complete_cond);

  // issue the callback if requested
  if (op->attr.callback) {
    op->attr.callback(op, op->attr.context);
  }
}

//----------------------------------------------------------------------------
static void process_op_open(IoOpImpl* op) {
  // verify file is not already open
  BLIoFile* file = op->file;
  if (file->handle != NULL) {
    mark_op_complete(op, BL_IO_STATUS_ERROR_BAD_FILE_HANDLE);
    return;
  }

  // open the file
  file->handle = fopen(op->file->file_name, "rb");
  if (!file->handle) {
    // determine the error
    BLIoStatus status;
    switch (errno) {
      case EACCES:        // access denied
      case EPERM:         // access denied (when trying to set atime?)
        status = BL_IO_STATUS_ERROR_ACCESS_DENIED;
        break;
      case EMFILE:        // too many open files for the process
      case ENFILE:        // too many open files for the OS
        status = BL_IO_STATUS_ERROR_TOO_MANY_OPEN_FILES;
        break;
      case ENOENT:        // file not found
        status = BL_IO_STATUS_ERROR_NOT_FOUND;
        break;
      case EEXIST:        // file already exists and mode wasn't setup to overwrite the file
      case EFAULT:        // bad filename pointer
      case EINVAL:        // bad mode argument
      case EISDIR:        // filename is a directory
      case ELOOP:         // symlink loop
      case ENAMETOOLONG:  // filename is too long
      case ENODEV:        // filename refers to a non-existent device
      case ENOMEM:        // out of memory
      case ENOSPC:        // no space left on device (for creating new files)
      case ENOTDIR:       // tried to open as a directory but not filename is not a directory
      case ENXIO:         // tried to open a special device file with wrong mode
      case EOVERFLOW:     // file is too large
      case EROFS:         // tried to open read-only filesystem with write mode
      case ETXTBSY:       // filename is executable and already running when opened with write mode
      case EWOULDBLOCK:   // O_NONBLOCK specified but is incompatible with file
      default:
        status = BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC;
        break;
    }

    mark_op_complete(op, status);
    return;
  }
  else {
    int ret;

    // get the file size by seeking to the end of the file and back
    // to the original position.
    long curr_pos = ftell(file->handle);
    if (curr_pos == -1L) {
      mark_op_complete(op, BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC);
      return;
    }
    ret = fseek(file->handle, 0, SEEK_END);
    if (ret == -1) {
      mark_op_complete(op, BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC);
      return;
    }
    long end_pos = ftell(file->handle);
    if (end_pos == -1L) {
      mark_op_complete(op, BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC);
      return;
    }
    ret = fseek(file->handle, curr_pos, SEEK_SET);
    if (ret == -1) {
      mark_op_complete(op, BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC);
      return;
    }

    // op is complete
    op->fulfilled_size = (uint64_t)end_pos;
    mark_op_complete(op, BL_IO_STATUS_OK);
  }
}

//----------------------------------------------------------------------------
static void process_op_close(IoOpImpl* op) {
  // verify the file is open
  BLIoFile* file = op->file;
  if (!file->handle) {
    mark_op_complete(op, BL_IO_STATUS_ERROR_BAD_FILE_HANDLE);
    return;
  }

  int ret;
  ret = fclose(file->handle);
  if (ret == -1) {
    mark_op_complete(op, BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC);
    return;
  }

  // success
  mark_op_complete(op, BL_IO_STATUS_OK);
  bl_free(file);
}

//----------------------------------------------------------------------------
static void process_op_read(IoOpImpl* op) {
  // verify the file is open
  BLIoFile* file = op->file;
  if (!file->handle) {
    mark_op_complete(op, BL_IO_STATUS_ERROR_BAD_FILE_HANDLE);
    return;
  }

  // seek to the appropriate offset
  int ret;
  ret = fseek(file->handle, op->offset, SEEK_SET);
  if (ret == -1) {
    BLIoStatus status;
    switch (errno) {
      // TODO: check for specific errors
      default:
        status = BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC;
        break;
    }

    mark_op_complete(op, status);
    return;
  }

  // do the read
  size_t bytes_read;
  size_t bytes_requested = (size_t)op->requested_size;
  bytes_read = fread(op->buffer, 1, bytes_requested, file->handle);
  op->fulfilled_size = (uint64_t)bytes_read;

  // check for eof or error
  BLIoStatus status;
  if (0 != feof(file->handle)) {
    status = BL_IO_STATUS_ERROR_EOF;
  }
  else if (0 != ferror(file->handle)) {
    status = BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC;
  }
  else {
    status = BL_IO_STATUS_OK;
  }
  op->status = status;
  mark_op_complete(op, status);
}

//----------------------------------------------------------------------------
static void io_thread_proc(void*) {
  bl_thread_set_name("io");
  while (!s_thread_quit) {
    // grab the next work item off the queue
    IoOpImpl* op;

    bl_mutex_lock(&s_work_queue_mutex);
    {
      // wait while the queue is empty
      while (s_work_queue.empty() && !s_thread_quit) {
        bl_cond_wait(&s_work_queue_not_empty_cond, &s_work_queue_mutex);
      }

      // kill the thread if the queue is empty and we need to quit
      if (s_thread_quit && s_work_queue.empty()) {
        bl_mutex_unlock(&s_work_queue_mutex);
        break;
      }

      // pull an item off the queue
      BL_ASSERT(!s_work_queue.empty());
      op = s_work_queue.front();
      s_work_queue.pop();
    }
    bl_mutex_unlock(&s_work_queue_mutex);

    // do the work
    switch (op->op_type) {
      case IO_OP_TYPE_CLOSE:
        process_op_close(op);
        break;

      case IO_OP_TYPE_OPEN:
        process_op_open(op);
        break;

      case IO_OP_TYPE_READ:
        process_op_read(op);
        break;

      default:
        BL_FATAL("unhandled op type: %u", op->op_type);
    }
  }
}


//
// exported functions
//

//------------------------------------------------------------------------------
void bl_io_lib_initialize() {
  // init the sync primitives
  bl_mutex_create(&s_work_queue_mutex);
  bl_cond_create(&s_work_queue_not_empty_cond);

  // startup the worker thread
  s_thread_quit = false;
  bl_thread_create(&s_thread, &io_thread_proc, NULL);
}

//------------------------------------------------------------------------------
void bl_io_lib_finalize() {
  // signal the worker thread to exit and wait
  s_thread_quit = true;
  bl_cond_notify_all(&s_work_queue_not_empty_cond);
  bl_thread_join(&s_thread);

  // cleanup sync primitives
  bl_cond_destroy(&s_work_queue_not_empty_cond);
  bl_mutex_destroy(&s_work_queue_mutex);
}

//------------------------------------------------------------------------------
BLIoOp* bl_io_file_open(const char* file_name, const BLIoOpAttr* attr, BLIoFile** file) {
  BL_ASSERT(file_name);
  BL_ASSERT(file);

  // protect against being given a NULL attribute
  if (!attr) {
    attr = &s_default_op_attr;
  }

  // create the file handle
  BLIoFile* __restrict new_file = (BLIoFile*)bl_alloc(sizeof(BLIoFile), 4);
  new_file->offset = 0;
  new_file->handle = NULL;
  bl_strcpy(new_file->file_name, file_name, BL_IO_MAX_FILE_NAME_LENGTH);
  *file = new_file;

  // define the async op
  IoOpImpl* __restrict op = (IoOpImpl*)bl_alloc(sizeof(IoOpImpl), 4);
  op->attr            = *attr;
  op->fulfilled_size  = 0;
  op->offset          = 0;
  op->requested_size  = 0;
  op->buffer          = NULL;
  op->file            = new_file;
  op->status          = BL_IO_STATUS_PENDING;
  op->op_type         = IO_OP_TYPE_OPEN;
  bl_mutex_create(&op->complete_mutex);
  bl_cond_create(&op->complete_cond);

  queue_op(op);
  return op;
}

//------------------------------------------------------------------------------
BLIoStatus bl_io_file_open_sync(const char* file_name, const BLIoOpAttr* attr, BLIoFile** file, uint64_t* file_size) {
  BL_ASSERT(file_name);
  BL_ASSERT(file);

  // handle file_size optional argument as NULL
  uint64_t local_file_size;
  if (!file_size) {
    file_size = &local_file_size;
  }

  BLIoOp* op = bl_io_file_open(file_name, attr, file);
  BLIoStatus status = bl_io_op_wait(op);
  *file_size = op->fulfilled_size;
  bl_io_op_delete(op);
  return status;
}

//------------------------------------------------------------------------------
BLIoOp* bl_io_file_close(BLIoFile* file, const BLIoOpAttr* attr) {
  BL_ASSERT(file);

  // protect against being given a NULL attribute
  if (!attr) {
    attr = &s_default_op_attr;
  }

  // define the async op
  IoOpImpl* __restrict op = (IoOpImpl*)bl_alloc(sizeof(IoOpImpl), 4);
  op->attr            = *attr;
  op->fulfilled_size  = 0;
  op->offset          = 0;
  op->requested_size  = 0;
  op->buffer          = NULL;
  op->file            = file;
  op->status          = BL_IO_STATUS_PENDING;
  op->op_type         = IO_OP_TYPE_CLOSE;
  bl_mutex_create(&op->complete_mutex);
  bl_cond_create(&op->complete_cond);

  queue_op(op);
  return op;
}

//------------------------------------------------------------------------------
BLIoStatus bl_io_file_close_sync(BLIoFile* file, const BLIoOpAttr* attr) {
  BL_ASSERT(file);

  BLIoOp* op = bl_io_file_close(file, attr);
  BLIoStatus status = bl_io_op_wait(op);
  bl_io_op_delete(op);
  return status;
}

//------------------------------------------------------------------------------
BLIoOp* bl_io_file_read(BLIoFile* file, const BLIoOpAttr* attr, void* dest, uint64_t size) {
  BL_ASSERT(file);
  BL_ASSERT(dest || size == 0);

  // protect against being given a NULL attribute
  if (!attr) {
    attr = &s_default_op_attr;
  }

  // define the async op
  IoOpImpl* __restrict op = (IoOpImpl*)bl_alloc(sizeof(IoOpImpl), 4);
  op->attr            = *attr;
  op->fulfilled_size  = 0;
  op->offset          = file->offset;
  op->requested_size  = size;
  op->buffer          = dest;
  op->file            = file;
  op->status          = BL_IO_STATUS_PENDING;
  op->op_type         = IO_OP_TYPE_READ;
  bl_mutex_create(&op->complete_mutex);
  bl_cond_create(&op->complete_cond);

  // move the file offset along for the next op
  file->offset += size;

  queue_op(op);
  return op;
}

//------------------------------------------------------------------------------
BLIoStatus bl_io_file_read_sync(BLIoFile* file, const BLIoOpAttr* attr, void* dest, uint64_t size) {
  BL_ASSERT(file);
  BL_ASSERT(dest || size == 0);

  BLIoOp* op = bl_io_file_read(file, attr, dest, size);
  BLIoStatus status = bl_io_op_wait(op);
  bl_io_op_delete(op);
  return status;
}

//------------------------------------------------------------------------------
void bl_io_file_seek_sync(BLIoFile* file, uint64_t offset) {
  BL_ASSERT(file);

  file->offset = offset;
}

//------------------------------------------------------------------------------
uint64_t bl_io_file_tell_sync(BLIoFile* file) {
  BL_ASSERT(file);

  return file->offset;
}

//------------------------------------------------------------------------------
BLIoStatus bl_io_op_wait(BLIoOp* op) {
  BL_ASSERT(op);
  IoOpImpl* op_impl = (IoOpImpl*)op;

  // wait for the op to complete with the timeout
  bl_mutex_lock(&op_impl->complete_mutex);
  while (op_impl->status == BL_IO_STATUS_PENDING) {
    bl_cond_wait(&op_impl->complete_cond, &op_impl->complete_mutex);
  }
  bl_mutex_unlock(&op_impl->complete_mutex);

  return op_impl->status;
}

//------------------------------------------------------------------------------
BLIoStatus bl_io_op_wait_timeout(BLIoOp* op, uint32_t timeout_ms) {
  BL_ASSERT(op);
  IoOpImpl* op_impl = (IoOpImpl*)op;

  // wait for the op to complete with the timeout
  bl_mutex_lock(&op_impl->complete_mutex);
  if (op_impl->status == BL_IO_STATUS_PENDING) {
    bl_cond_wait_timeout(&op_impl->complete_cond, &op_impl->complete_mutex, timeout_ms);
  }
  bl_mutex_unlock(&op_impl->complete_mutex);

  return op_impl->status;
}

//------------------------------------------------------------------------------
void bl_io_op_delete(BLIoOp* op) {
  BL_ASSERT(op);
  IoOpImpl* op_impl = (IoOpImpl*)op;

  bl_io_op_wait(op);
  bl_cond_destroy(&op_impl->complete_cond);
  bl_mutex_destroy(&op_impl->complete_mutex);
  bl_free(op_impl);
}
