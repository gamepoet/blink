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
#ifndef BL_IO_H
#define BL_IO_H

#include <blink/base.h>

//
// constants
//

enum BLIoStatus {
  BL_IO_STATUS_OK,
  BL_IO_STATUS_PENDING,
  BL_IO_STATUS_ERROR_ACCESS_DENIED,
  BL_IO_STATUS_ERROR_BAD_FILE_HANDLE,
  BL_IO_STATUS_ERROR_EOF,
  BL_IO_STATUS_ERROR_NOT_FOUND,
  BL_IO_STATUS_ERROR_PLATFORM_SPECIFIC,
  BL_IO_STATUS_ERROR_TOO_MANY_OPEN_FILES,
};

// maximum length of a file name supported by this module
static const unsigned int BL_IO_MAX_FILE_NAME_LENGTH = 512;


//
// types
//

struct BLIoFile;
struct BLIoOp;

typedef void (*BLIoOpCallback)(BLIoOp* op, void* context);

struct BLIoOpAttr {
  BLIoOpCallback  callback;
  void*           context;
};

struct BLIoOp {
  BLIoOpAttr      attr;
  uint64_t        fulfilled_size;
  uint64_t        offset;
  uint64_t        requested_size;
  void*           buffer;
  BLIoFile*       file;
  BLIoStatus      status;
};


//
// library management
//

void bl_io_lib_initialize();
void bl_io_lib_finalize();


//
// file ops
//

BLIoOp* bl_io_file_open(const char* file_name, const BLIoOpAttr* attr, BLIoFile** file);
BLIoStatus bl_io_file_open_sync(const char* file_name, const BLIoOpAttr* attr, BLIoFile** file, uint64_t* file_size = NULL);

BLIoOp* bl_io_file_close(BLIoFile* file, const BLIoOpAttr* attr);
BLIoStatus bl_io_file_close_sync(BLIoFile* file, const BLIoOpAttr* attr);

BLIoOp* bl_io_file_read(BLIoFile* file, const BLIoOpAttr* attr, void* buffer, uint64_t size);
BLIoStatus bl_io_file_read_sync(BLIoFile* file, const BLIoOpAttr* attr, void* buffer, uint64_t size);

void bl_io_file_seek_sync(BLIoFile* file, uint64_t offset);
uint64_t bl_io_file_tell_sync(BLIoFile* file);


//
// ops
//

BLIoStatus bl_io_op_wait(BLIoOp* op);
BLIoStatus bl_io_op_wait_timeout(BLIoOp* op, uint32_t timeout_ms);

void bl_io_op_delete(BLIoOp* op);

#endif
