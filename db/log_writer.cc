// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/log_writer.h"

#include <cstdint>

#include "leveldb/env.h"
#include "util/coding.h"
#include "util/crc32c.h"

namespace leveldb {
namespace log {

static void InitTypeCrc(uint32_t* type_crc) {
  for (int i = 0; i <= kMaxRecordType; i++) {
    char t = static_cast<char>(i);
    type_crc[i] = crc32c::Value(&t, 1);
  }
}

Writer::Writer(WritableFile* dest) : dest_(dest), block_offset_(0) {
  InitTypeCrc(type_crc_);
}

Writer::Writer(WritableFile* dest, uint64_t dest_length)
    : dest_(dest), block_offset_(dest_length % kBlockSize) {
  InitTypeCrc(type_crc_);
}

Writer::~Writer() = default;

//* | SequenceNumber(8) | Count(4) | type(1) 0x0 delete / 0x1 key/value | key_len | key | value_len | value |
//* SequenceNumber: 占 8 个字节, 表示该批次的序列号
//* Count: 占 4 个字节, 表示该批次有多少键值对
//* Type: 0x1: 表示删除该批次 key/value, 0x1: 表示新增加 key / value
//* 
//*

Status Writer::AddRecord(const Slice& slice) {
  //* ptr 指向需要写入的记录内容
  const char* ptr = slice.data();
  //* left 表示需要写入的记录内容长度
  size_t left = slice.size();

  // Fragment the record if necessary and emit it.  Note that if slice
  // is empty, we still want to iterate once to emit a single
  // zero-length record
  Status s;
  //* 1. begin 为 true 表明该条记录是第一次写入, 即如果一个记录
  //*    跨越多个块时, 只有写入第一个块时:
  //* 2. begin == true: 其他时候会设置为 false, 通过该值可以确定
  //*    头部类型字段是否为 kFirstType.
  bool begin = true;
  do {
    //* kBlockSize 为一个块的大小 (32768字节)
    //* block_offset_ 代表当前块的写入偏移量
    const int leftover = kBlockSize - block_offset_;
    assert(leftover >= 0);
    //* 如果当前块剩下的 size < kHeaderSize(7)
    //* 那么就结束当前块的写入, 并且追加上 "\x00\x00\x00\x00\x00\x00".
    if (leftover < kHeaderSize) {
      // Switch to a new block
      if (leftover > 0) {
        // Fill the trailer (literal below relies on kHeaderSize being 7)
        static_assert(kHeaderSize == 7, "");
        //* 将剩余空间填满, 填补上 "\x00\x00\x00\x00\x00\x00".
        dest_->Append(Slice("\x00\x00\x00\x00\x00\x00", leftover));
      }
      //* 块的偏移量置为 0
      block_offset_ = 0;
    }

    // Invariant: we never leave < kHeaderSize bytes in a block.
    assert(kBlockSize - block_offset_ - kHeaderSize >= 0);
    //* 计算块剩余空间
    const size_t avail = kBlockSize - block_offset_ - kHeaderSize;
    //* 当前块能够写入的数据大小 取决于 记录剩余内容和块剩余空间之中比较小的值
    const size_t fragment_length = (left < avail) ? left : avail;

    RecordType type;
    //* end 字段表示该条记录是否已经完整地写入
    const bool end = (left == fragment_length);
    if (begin && end) {
      //* 开始写入就完整地写入了一条记录, 因此为 kFullType
      type = kFullType;
    } else if (begin) {
      //* 开始写入但不是完整地写入, 因此是 kFirstType
      type = kFirstType;
    } else if (end) {
      type = kLastType;
    } else {
      type = kMiddleType;
    }
    //* 按图, 然后更新 block_offset_字段长度
    s = EmitPhysicalRecord(type, ptr, fragment_length);
    //* 更新需要写入的记录指针
    ptr += fragment_length;
    //* 更新需要写入的记录内容大小
    left -= fragment_length;
    //* 第一次写入之后将 begin = false
    begin = false;
    //* 一直到 left <= 0 或者某次写入失败就停止
  } while (s.ok() && left > 0);
  return s;
}

Status Writer::EmitPhysicalRecord(RecordType t, const char* ptr,
                                  size_t length) {
  assert(length <= 0xffff);  // Must fit in two bytes
  assert(block_offset_ + kHeaderSize + length <= kBlockSize);

  // Format the header
  char buf[kHeaderSize];
  buf[4] = static_cast<char>(length & 0xff);
  buf[5] = static_cast<char>(length >> 8);
  buf[6] = static_cast<char>(t);

  // Compute the crc of the record type and the payload.
  uint32_t crc = crc32c::Extend(type_crc_[t], ptr, length);
  crc = crc32c::Mask(crc);  // Adjust for storage
  EncodeFixed32(buf, crc);

  // Write the header and the payload
  Status s = dest_->Append(Slice(buf, kHeaderSize));
  if (s.ok()) {
    s = dest_->Append(Slice(ptr, length));
    if (s.ok()) {
      s = dest_->Flush();
    }
  }
  block_offset_ += kHeaderSize + length;
  return s;
}

}  // namespace log
}  // namespace leveldb
