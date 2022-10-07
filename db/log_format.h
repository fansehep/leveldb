// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Log format information shared by reader and writer.
// See ../doc/log_format.md for more detail.

#ifndef STORAGE_LEVELDB_DB_LOG_FORMAT_H_
#define STORAGE_LEVELDB_DB_LOG_FORMAT_H_

namespace leveldb {
namespace log {



/*
 * |  crc(4) | length(2) | type(1) | content |
 *
 *
 *
 *
 */

enum RecordType {
  // Zero is reserved for preallocated files
  kZeroType = 0,
  //* 表示一条记录完整地写到了一个块上
  kFullType = 1,

  // For fragments
  //* 当一条记录跨几个块时, 
  //* kFirstType 表示该条记录的第一部分
  //* kMiddleType 表示该条记录的中间部分
  //* kLastType 表示该条记录的最后一部分
  kFirstType = 2,
  kMiddleType = 3,
  kLastType = 4
};
static const int kMaxRecordType = kLastType;

static const int kBlockSize = 32768;

// Header is checksum (4 bytes), length (2 bytes), type (1 byte).
static const int kHeaderSize = 4 + 2 + 1;

}  // namespace log
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_LOG_FORMAT_H_
