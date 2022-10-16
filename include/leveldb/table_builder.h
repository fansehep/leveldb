// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// TableBuilder provides the interface used to build a Table
// (an immutable and sorted map from keys to values).
//
// Multiple threads can invoke const methods on a TableBuilder without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same TableBuilder must use
// external synchronization.

#ifndef STORAGE_LEVELDB_INCLUDE_TABLE_BUILDER_H_
#define STORAGE_LEVELDB_INCLUDE_TABLE_BUILDER_H_

#include <cstdint>

#include "leveldb/export.h"
#include "leveldb/options.h"
#include "leveldb/status.h"

namespace leveldb {

class BlockBuilder;
class BlockHandle;
class WritableFile;

class LEVELDB_EXPORT TableBuilder {
 public:
  // Create a builder that will store the contents of the table it is
  // building in *file.  Does not close the file.  It is up to the
  // caller to close the file after calling Finish().
  //
  // 创建一个构建器，将它正在构建的表的内容存储在*file中。
  // 不关闭该文件。 在调用Finish()后，
  // 由调用者自己决定是否关闭文件。
  TableBuilder(const Options& options, WritableFile* file);

  TableBuilder(const TableBuilder&) = delete;
  TableBuilder& operator=(const TableBuilder&) = delete;

  // REQUIRES: Either Finish() or Abandon() has been called.
  ~TableBuilder();

  // Change the options used by this builder.  Note: only some of the
  // option fields can be changed after construction.  If a field is
  // not allowed to change dynamically and its value in the structure
  // passed to the constructor is different from its value in the
  // structure passed to this method, this method will return an error
  // without changing any fields.
  //
  // 改变这个构建器所使用的选项。 注意：只有部分 选项字段可以在构建后改变。
  // 如果一个字段是 不允许动态变化，并且其在结构中的值 中的值与传递给本方法的结构中的值不同。
  // 结构中的值不同，该方法将返回一个错误 而不改变任何字段。
  Status ChangeOptions(const Options& options);

  // Add key,value to the table being constructed.
  // REQUIRES: key is after any previously added key according to comparator.
  // REQUIRES: Finish(), Abandon() have not been called
  //
  // 将key,value添加到正在构建的表中。
  // 要求：根据比较器，key在任何先前添加的key之后。
  // 需要。Finish(), Abandon()还没有被调用。
  void Add(const Slice& key, const Slice& value);

  // Advanced operation: flush any buffered key/value pairs to file.
  // Can be used to ensure that two adjacent entries never live in
  // the same data block.  Most clients should not need to use this method.
  // REQUIRES: Finish(), Abandon() have not been called
  //
  // 高级操作：将任何缓冲的键/值对刷新到文件。
  // 可以用来确保两个相邻的条目永远不会出现在
  // 同一个数据块中。 大多数客户应该不需要使用这个方法。
  // 需要。Finish(), Abandon()还没有被调用。
  void Flush();

  // Return non-ok iff some error has been detected.
  //
  // 如果检测到一些错误，则返回非ok。
  Status status() const;

  // Finish building the table.  Stops using the file passed to the
  // constructor after this function returns.
  // REQUIRES: Finish(), Abandon() have not been called
  //
  // 完成建表。 在此函数返回后，停止使用传递给的文件，在此函数返回后停止使用传递给构造函数的文件。
  // 需要。Finish(), Abandon()还没有被调用。
  Status Finish();

  // Indicate that the contents of this builder should be abandoned.  Stops
  // using the file passed to the constructor after this function returns.
  // If the caller is not going to call Finish(), it must call Abandon()
  // before destroying this builder.
  // REQUIRES: Finish(), Abandon() have not been called
  //
  // 表明该构建器的内容应该被放弃。 在此函数返回后停止使用传递给构造函数的文件。
  // 如果调用者不打算调用Finish()，必须在销毁此构建器之前调用Abandon()
  // 之前调用Abandon()。需要。Finish()、Abandon()未被调用。
  void Abandon();

  // Number of calls to Add() so far.
  uint64_t NumEntries() const;

  // Size of the file generated so far.  If invoked after a successful
  // Finish() call, returns the size of the final generated file.
  // 到目前为止生成的文件的大小。 如果在一个成功的
  // Finish()调用后，返回最终生成的文件的大小。
  uint64_t FileSize() const;

 private:
  bool ok() const { return status().ok(); }

  void WriteBlock(BlockBuilder* block, BlockHandle* handle);

  void WriteRawBlock(const Slice& data, CompressionType, BlockHandle* handle);

  struct Rep;
  Rep* rep_;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_TABLE_BUILDER_H_
