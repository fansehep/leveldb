// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "leveldb/table_builder.h"

#include <cassert>

#include "leveldb/comparator.h"
#include "leveldb/env.h"
#include "leveldb/filter_policy.h"
#include "leveldb/options.h"
#include "table/block_builder.h"
#include "table/filter_block.h"
#include "table/format.h"
#include "util/coding.h"
#include "util/crc32c.h"

namespace leveldb {

struct TableBuilder::Rep {
  Rep(const Options& opt, WritableFile* f)
      : options(opt),
        index_block_options(opt),
        file(f),
        offset(0),
        data_block(&options),
        index_block(&index_block_options),
        num_entries(0),
        closed(false),
        filter_block(opt.filter_policy == nullptr
                         ? nullptr
                         : new FilterBlockBuilder(opt.filter_policy)),
        pending_index_entry(false) {
    index_block_options.block_restart_interval = 1;
  }

  Options options;
  Options index_block_options;
  //* SSTable 生成之后的文件
  WritableFile* file;
  //* 
  uint64_t offset;
  Status status;
  //* data_block: 用来生成 SSTable 中的数据区域
  BlockBuilder data_block;
  //* index_block: 数据索引区域
  BlockBuilder index_block;
  std::string last_key;
  int64_t num_entries;
  bool closed;  // Either Finish() or Abandon() has been called.
  //* 用来生成 SSTable 中的元数据区域
  //* 元数据的保存格式与数据块不同, 因此需要一个专门的生成类 
  FilterBlockBuilder* filter_block;

  // We do not emit the index entry for a block until we have seen the
  // first key for the next data block.  This allows us to use shorter
  // keys in the index block.  For example, consider a block boundary
  // between the keys "the quick brown fox" and "the who".  We can use
  // "the r" as the key for the index block entry since it is >= all
  // entries in the first block and < all entries in subsequent
  // blocks.
  //
  // Invariant: r->pending_index_entry is true only if data_block is empty.
  //
  //* 我们在看到下一个数据块的第一个键之前，不会发出一个数据块的索引条目。
  //* 下一个数据块的第一个键。 这使得我们可以在索引块中使用较短的
  //* 的索引块中使用较短的键。 例如，考虑一个块的边界
  //* 在 "快速的棕色狐狸 "和 "谁 "这两个键之间。 我们可以使用
  //* "the r "作为索引块条目的键，因为它>=第一块中的所有
  //* 第一个块中的所有条目，并且<后续块中的所有条目。
  //* 块中的所有条目。
  //
  //* 不变量：只有当data_block为空时，r->pending_index_entry为真。
  //* pending_index_entry: 这两个变量用来决定是否需要写数据索引
  //* SSTable 中每次完整写入最后一个块后需要生成该块的索引, 索
  bool pending_index_entry;
  BlockHandle pending_handle;  // Handle to add to index block

  std::string compressed_output;
};

TableBuilder::TableBuilder(const Options& options, WritableFile* file)
    : rep_(new Rep(options, file)) {
  if (rep_->filter_block != nullptr) {
    rep_->filter_block->StartBlock(0);
  }
}

TableBuilder::~TableBuilder() {
  assert(rep_->closed);  // Catch errors where caller forgot to call Finish()
  delete rep_->filter_block;
  delete rep_;
}

Status TableBuilder::ChangeOptions(const Options& options) {
  // Note: if more fields are added to Options, update
  // this function to catch changes that should not be allowed to
  // change in the middle of building a Table.
  if (options.comparator != rep_->options.comparator) {
    return Status::InvalidArgument("changing comparator while building table");
  }

  // Note that any live BlockBuilders point to rep_->options and therefore
  // will automatically pick up the updated options.
  rep_->options = options;
  rep_->index_block_options = options;
  rep_->index_block_options.block_restart_interval = 1;
  return Status::OK();
}

void TableBuilder::Add(const Slice& key, const Slice& value) {
  //* r 赋值给 TableBuilder
  Rep* r = rep_;
  assert(!r->closed);
  if (!ok()) return;

  if (r->num_entries > 0) {
    assert(r->options.comparator->Compare(key, Slice(r->last_key)) > 0);
  }

  /*
   * 判断是否需要增加数据索引, 当完整生成一个块之后,
   * 需要写入数据索引.
   */
  if (r->pending_index_entry) {
    assert(r->data_block.empty());
    //* 查找该块的最大key 与 即将插入 key (即下一个块的最小 key) 之间的最短分隔符
    r->options.comparator->FindShortestSeparator(&r->last_key, key);
    std::string handle_encoding;
    //* 将该块的BlockHandle 编码, 即将偏移量和大小分别编码为可变长度的 64位整型
    r->pending_handle.EncodeTo(&handle_encoding);
    //* 在数据索引中写入 key 和该块的 BlockHandle, BlockHandle 结构包括块的偏移量
    //* 以及块大小.
    r->index_block.Add(r->last_key, Slice(handle_encoding));
    //* 将 pending_index_entry = false, 等待下一次生成一个完整的 Block并
    //* 将该值再次置为 true.
    r->pending_index_entry = false;
  }

  //* 在元数据块中增加该 key
  if (r->filter_block != nullptr) {
    r->filter_block->AddKey(key);
  }

  //* 将 las_key 赋值给即将插入的 value
  r->last_key.assign(key.data(), key.size());
  r->num_entries++;
  //* 在数据块中增加 key-value.
  r->data_block.Add(key, value);

  const size_t estimated_block_size = r->data_block.CurrentSizeEstimate();
  //* 判断如果当前块的大小 > 配置块的大小, 则调用 Flush 函数.
  if (estimated_block_size >= r->options.block_size) {
    Flush();
  }
}

void TableBuilder::Flush() {
  Rep* r = rep_;
  assert(!r->closed);
  if (!ok()) return;
  if (r->data_block.empty()) return;
  assert(!r->pending_index_entry);
  WriteBlock(&r->data_block, &r->pending_handle);
  if (ok()) {
    r->pending_index_entry = true;
    r->status = r->file->Flush();
  }
  if (r->filter_block != nullptr) {
    r->filter_block->StartBlock(r->offset);
  }
}

void TableBuilder::WriteBlock(BlockBuilder* block, BlockHandle* handle) {
  // File format contains a sequence of blocks where each block has:
  //    block_data: uint8[n]
  //    type: uint8
  //    crc: uint32
  assert(ok());
  Rep* r = rep_;
  Slice raw = block->Finish();

  Slice block_contents;
  CompressionType type = r->options.compression;
  // TODO(postrelease): Support more compression options: zlib?
  switch (type) {
    case kNoCompression:
      block_contents = raw;
      break;

    case kSnappyCompression: {
      std::string* compressed = &r->compressed_output;
      if (port::Snappy_Compress(raw.data(), raw.size(), compressed) &&
          compressed->size() < raw.size() - (raw.size() / 8u)) {
        block_contents = *compressed;
      } else {
        // Snappy not supported, or compressed less than 12.5%, so just
        // store uncompressed form
        block_contents = raw;
        type = kNoCompression;
      }
      break;
    }
  }
  WriteRawBlock(block_contents, type, handle);
  r->compressed_output.clear();
  block->Reset();
}

void TableBuilder::WriteRawBlock(const Slice& block_contents,
                                 CompressionType type, BlockHandle* handle) {
  Rep* r = rep_;
  handle->set_offset(r->offset);
  handle->set_size(block_contents.size());
  r->status = r->file->Append(block_contents);
  if (r->status.ok()) {
    char trailer[kBlockTrailerSize];
    trailer[0] = type;
    uint32_t crc = crc32c::Value(block_contents.data(), block_contents.size());
    crc = crc32c::Extend(crc, trailer, 1);  // Extend crc to cover block type
    EncodeFixed32(trailer + 1, crc32c::Mask(crc));
    r->status = r->file->Append(Slice(trailer, kBlockTrailerSize));
    if (r->status.ok()) {
      r->offset += block_contents.size() + kBlockTrailerSize;
    }
  }
}

Status TableBuilder::status() const { return rep_->status; }

Status TableBuilder::Finish() {
  Rep* r = rep_;
  //* Flush 函数会将数据块写入 SSTable 文件并且刷新到磁盘
  Flush();
  assert(!r->closed);
  r->closed = true;

  //* filter_block_handle 为元数据的BlockHandle,
  //* metaindex_block_handle 为元数据索引的 BlockHandle
  //* index_block_handle 为数据索引的 BlockHandle
  BlockHandle filter_block_handle, metaindex_block_handle, index_block_handle;

  // Write filter block
  //* 写入元数据块
  if (ok() && r->filter_block != nullptr) {
    WriteRawBlock(r->filter_block->Finish(), kNoCompression,
                  &filter_block_handle);
  }

  // Write metaindex block
  //* 写入元数据索引
  if (ok()) {
    BlockBuilder meta_index_block(&r->options);
    if (r->filter_block != nullptr) {
      // Add mapping from "filter.Name" to location of filter data
      //* 元数据索引块' key = "filter." 加上配置的过滤器名称,
      //* default = filter.leveldb.BuiltinBloomFilter2
      std::string key = "filter.";
      key.append(r->options.filter_policy->Name());
      std::string handle_encoding;
      filter_block_handle.EncodeTo(&handle_encoding);
      //* 元数据索引块的 value : 也是一个 BlockHandle
      //* BlockHandle 也是一个指向元数据块的偏移量 以及 元数据块的大小
      meta_index_block.Add(key, handle_encoding);
    }

    // TODO(postrelease): Add stats and other meta blocks
    WriteBlock(&meta_index_block, &metaindex_block_handle);
  }

  // Write index block
  //* 写入数据块索引
  if (ok()) {
    if (r->pending_index_entry) {
      r->options.comparator->FindShortSuccessor(&r->last_key);
      std::string handle_encoding;
      r->pending_handle.EncodeTo(&handle_encoding);
      r->index_block.Add(r->last_key, Slice(handle_encoding));
      r->pending_index_entry = false;
    }
    WriteBlock(&r->index_block, &index_block_handle);
  }

  // Write footer
  //* 写入尾部
  if (ok()) {
    Footer footer;
    //* 将元数据索引区域的 BlockHandle value 设置到尾部
    footer.set_metaindex_handle(metaindex_block_handle);
    //* 将数据索引区域的 BlockHandle value 设置到尾部
    footer.set_index_handle(index_block_handle);
    std::string footer_encoding;
    footer.EncodeTo(&footer_encoding);
    r->status = r->file->Append(footer_encoding);
    if (r->status.ok()) {
      r->offset += footer_encoding.size();
    }
  }
  return r->status;
}

void TableBuilder::Abandon() {
  Rep* r = rep_;
  assert(!r->closed);
  r->closed = true;
}

uint64_t TableBuilder::NumEntries() const { return rep_->num_entries; }

uint64_t TableBuilder::FileSize() const { return rep_->offset; }

}  // namespace leveldb
