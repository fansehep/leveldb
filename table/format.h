// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_TABLE_FORMAT_H_
#define STORAGE_LEVELDB_TABLE_FORMAT_H_

#include <cstdint>
#include <string>

#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "leveldb/table_builder.h"

namespace leveldb {

class Block;
class RandomAccessFile;
struct ReadOptions;

// BlockHandle is a pointer to the extent of a file that stores a data
// block or a meta block.
//
// BlockHandle是一个指向存储数据块或元块的文件范围的指针。
// 块或元块。
class BlockHandle {
 public:
  // Maximum encoding length of a BlockHandle
  //
  // 一个BlockHandle的最大编码长度
  enum { kMaxEncodedLength = 10 + 10 };

  BlockHandle();

  // The offset of the block in the file.
  //
  // 该块在文件中的偏移量
  uint64_t offset() const { return offset_; }
  void set_offset(uint64_t offset) { offset_ = offset; }

  // The size of the stored block
  //
  // 存储区块的大小
  //
  uint64_t size() const { return size_; }
  void set_size(uint64_t size) { size_ = size; }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* input);

 private:
  //
  // 偏移量, 编码为可变长度的 64位 整型, 最多占用 10 个字节.
  uint64_t offset_;
  // 大小, 编码为可变长度的 64 位整型, 最多占用 10 个字节.
  uint64_t size_;
};

// Footer encapsulates the fixed information stored at the tail
// end of every table file.
//
// 页脚封装了存储在每个表文件尾部的固定信息。
// 每个表文件的尾部。
//

class Footer {
 public:
  //
  // Encoded length of a Footer.  Note that the serialization of a
  // Footer will always occupy exactly this many bytes.  It consists
  // of two block handles and a magic number.
  //
  // 编码后的页脚长度。 请注意，一个脚注的序列化将永远占据这个字节。
  // 的序列化将始终占据这个字节数。 它包括
  // 由两个块柄和一个神奇的数字组成。
  //
  enum { kEncodedLength = 2 * BlockHandle::kMaxEncodedLength + 8 };

  Footer() = default;

  // The block handle for the metaindex block of the table
  const BlockHandle& metaindex_handle() const { return metaindex_handle_; }
  void set_metaindex_handle(const BlockHandle& h) { metaindex_handle_ = h; }

  // The block handle for the index block of the table
  const BlockHandle& index_handle() const { return index_handle_; }
  void set_index_handle(const BlockHandle& h) { index_handle_ = h; }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* input);

 private:
  BlockHandle metaindex_handle_;
  BlockHandle index_handle_;
};

// kTableMagicNumber was picked by running
//    echo http://code.google.com/p/leveldb/ | sha1sum
// and taking the leading 64 bits.
// kTableMagicNumber是通过运行来挑选的。
// echo http://code.google.com/p/leveldb/ | sha1sum
// 并取前64位。
static const uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;

// 1-byte type + 32-bit crc
static const size_t kBlockTrailerSize = 5;

struct BlockContents {
  Slice data;           // Actual contents of data
  bool cachable;        // True iff data can be cached
  bool heap_allocated;  // True iff caller should delete[] data.data()
};

// Read the block identified by "handle" from "file".  On failure
// return non-OK.  On success fill *result and return OK.
//
// 从 "文件 "中读取由 "handle "标识的块。 失败时
// 返回非OK。 成功时填充*result并返回OK。
//
Status ReadBlock(RandomAccessFile* file, const ReadOptions& options,
                 const BlockHandle& handle, BlockContents* result);

//
// Implementation details follow.  Clients should ignore,
// 实施细节如下。 客户应该忽略。
//
inline BlockHandle::BlockHandle()
    : offset_(~static_cast<uint64_t>(0)), size_(~static_cast<uint64_t>(0)) {}

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_FORMAT_H_
