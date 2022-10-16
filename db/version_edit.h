// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_DB_VERSION_EDIT_H_
#define STORAGE_LEVELDB_DB_VERSION_EDIT_H_

#include "db/dbformat.h"
#include <set>
#include <utility>
#include <vector>

namespace leveldb {

class VersionSet;

struct FileMetaData {
  FileMetaData() : refs(0), allowed_seeks(1 << 30), file_size(0) {}

  // 引用次数
  int refs;
  // 允许最大的无效查询次数
  int allowed_seeks;  // Seeks allowed until compaction
  // 文件序列号
  uint64_t number;
  // 文件大小
  uint64_t file_size;    // File size in bytes
  // 该文件中的最小键
  InternalKey smallest;  // Smallest internal key served by table
  // 该文件的最大键
  InternalKey largest;   // Largest internal key served by table
};

class VersionEdit {
 public:
  VersionEdit() { Clear(); }
  ~VersionEdit() = default;

  void Clear();

  void SetComparatorName(const Slice& name) {
    has_comparator_ = true;
    comparator_ = name.ToString();
  }

  void SetLogNumber(uint64_t num) {
    has_log_number_ = true;
    log_number_ = num;
  }

  void SetPrevLogNumber(uint64_t num) {
    has_prev_log_number_ = true;
    prev_log_number_ = num;
  }

  void SetNextFile(uint64_t num) {
    has_next_file_number_ = true;
    next_file_number_ = num;
  }

  void SetLastSequence(SequenceNumber seq) {
    has_last_sequence_ = true;
    last_sequence_ = seq;
  }

  void SetCompactPointer(int level, const InternalKey& key) {
    compact_pointers_.push_back(std::make_pair(level, key));
  }

  // Add the specified file at the specified number.
  // REQUIRES: This version has not been saved (see VersionSet::SaveTo)
  // REQUIRES: "smallest" and "largest" are smallest and largest keys in file
  void AddFile(int level, uint64_t file, uint64_t file_size,
               const InternalKey& smallest, const InternalKey& largest) {
    FileMetaData f;
    f.number = file;
    f.file_size = file_size;
    f.smallest = smallest;
    f.largest = largest;
    new_files_.push_back(std::make_pair(level, f));
  }

  // Delete the specified "file" from the specified "level".
  void RemoveFile(int level, uint64_t file) {
    deleted_files_.insert(std::make_pair(level, file));
  }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(const Slice& src);

  std::string DebugString() const;

 private:
  friend class VersionSet;

  typedef std::set<std::pair<int, uint64_t>> DeletedFileSet;
  //* 比较器名称, 因为 MemTable 以及 SSTable 均是有序排列,
  //* 因此需要一个比较器
  std::string comparator_;

  // 日志文件序号
  // 日志文件序号, 日志文件与 MemTable 一一对应,
  // 当一个MemTable 生成为SSTable后会将旧的日志文件删除
  // 并且生成一个新的日志文件, 日志文件的名称由 6 位 (不足 6 位 则前边加 0 补充)
  // 日志文件序列号 + log 后缀组成, 例如 000001.log
  uint64_t log_number_;
  uint64_t prev_log_number_;
  // 下一个文件序列号, LevelDB 中的文件包括日志文件
  // 以及SSTable文件, Manifest 文件, SSTable 文件由 6 位
  // 文件序列号加 sst 后缀组成
  uint64_t next_file_number_;
  
  //
  // 下一个写入序列号,
  // LevelDB 的每次写入都会递增序列号
  //
  SequenceNumber last_sequence_;
  // 5 个 成员变量, 代表对应的成员是否成立
  bool has_comparator_;
  bool has_log_number_;
  bool has_prev_log_number_;
  bool has_next_file_number_;
  bool has_last_sequence_;

  // 该变量用来指示 LevelDB 中每个层级下一次进行 Compaction
  // 从哪个 key 开始, 对每个
  std::vector<std::pair<int, InternalKey>> compact_pointers_;

  // deleted_files_: 记录每个层级执行 Compaction 操作
  // 之后删除掉的文件, 注意此处只需要删除文件的文件序列号
  DeletedFileSet deleted_files_;
  //
  // 记录每个层级执行 Compaction 操作之后新增加的文件,
  // 
  std::vector<std::pair<int, FileMetaData>> new_files_;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_VERSION_EDIT_H_
