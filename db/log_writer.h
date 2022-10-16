// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_DB_LOG_WRITER_H_
#define STORAGE_LEVELDB_DB_LOG_WRITER_H_

#include <cstdint>

#include "db/log_format.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

class WritableFile;

namespace log {

class Writer {
 public:
  // Create a writer that will append data to "*dest".
  // "*dest" must be initially empty.
  // "*dest" must remain live while this Writer is in use.
  //*
  //* 创建一个写入器，将数据追加到 "*dest"。
  //* "*dest "最初必须是空的。
  //* "*dest "在使用该写入器时必须保持有效。
  explicit Writer(WritableFile* dest);

  // Create a writer that will append data to "*dest".
  // "*dest" must have initial length "dest_length".
  // "*dest" must remain live while this Writer is in use.
  //*
  //* 创建一个写入器，将数据追加到 "*dest"。
  //* "*dest "必须有初始长度 "dest_length"。
  //* "*dest "在使用此写入器时必须保持有效。
  Writer(WritableFile* dest, uint64_t dest_length);

  Writer(const Writer&) = delete;
  Writer& operator=(const Writer&) = delete;

  ~Writer();

  Status AddRecord(const Slice& slice);

 private:
  Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);
  //* 写入文件的抽象
  WritableFile* dest_;
  //* 当前块的偏移量
  int block_offset_;  // Current offset in block

  // crc32c values for all supported record types.  These are
  // pre-computed to reduce the overhead of computing the crc of the
  // record type stored in the header.
  //*
  //* 所有支持的记录类型的crc32c值。 这些值是
  //* 预先计算，以减少计算头中存储的记录类型的crc值的开销。
  //* 记录类型存储在头文件中。
  uint32_t type_crc_[kMaxRecordType + 1];
};

}  // namespace log
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_LOG_WRITER_H_
