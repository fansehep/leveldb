// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_DB_MEMTABLE_H_
#define STORAGE_LEVELDB_DB_MEMTABLE_H_

#include <string>

#include "db/dbformat.h"
#include "db/skiplist.h"
#include "leveldb/db.h"
#include "util/arena.h"

namespace leveldb {

//* key 比较器
class InternalKeyComparator;
class MemTableIterator;

class MemTable {
 public:
  // MemTables are reference counted.  The initial reference count
  // is zero and the caller must call Ref() at least once.
  //* MemTables是引用计数的。 最初的引用计数
  //* 是零，调用者必须至少调用一次Ref()。
  explicit MemTable(const InternalKeyComparator& comparator);

  MemTable(const MemTable&) = delete;
  MemTable& operator=(const MemTable&) = delete;

  // Increase reference count.
  void Ref() { ++refs_; }

  // Drop reference count.  Delete if no more references exist.
  void Unref() {
    --refs_;
    assert(refs_ >= 0);
    if (refs_ <= 0) {
      delete this;
    }
  }

  // Returns an estimate of the number of bytes of data in use by this
  // data structure. It is safe to call when MemTable is being modified.
  //* 返回这个数据结构所使用的数据字节数的估计值。
  //* 数据结构使用的数据字节数。当MemTable被修改时，调用它是安全的。
  size_t ApproximateMemoryUsage();

  // Return an iterator that yields the contents of the memtable.
  //
  // The caller must ensure that the underlying MemTable remains live
  // while the returned iterator is live.  The keys returned by this
  // iterator are internal keys encoded by AppendInternalKey in the
  // db/format.{h,cc} module.
  //*
  //* 返回一个迭代器，产生memtable的内容。
  //*
  //* 调用者必须确保底层的MemTable是活的。
  //* 当返回的迭代器是活的。 这个迭代器所返回的键
  //* 迭代器返回的键是由AppendInternalKey编码的内部键，在
  //* db/format.{h,cc}模块中的AppendInternalKey编码。
  Iterator* NewIterator();

  /**
   *
   *
   */

  // Add an entry into memtable that maps key to value at the
  // specified sequence number and with the specified type.
  // Typically value will be empty if type==kTypeDeletion.
  //*
  //*
  //* 在memtable中添加一个条目，在指定的序列号和指定的类型下将键映射到值。
  //* 指定的序列号和指定的类型。
  //* 如果类型==kTypeDeletion，典型的值将是空的。
  void Add(SequenceNumber seq, ValueType type, const Slice& key,
           const Slice& value);

  // If memtable contains a value for key, store it in *value and return true.
  // If memtable contains a deletion for key, store a NotFound() error
  // in *status and return true.
  // Else, return false.
  //*
  //* 如果memtable包含一个key的值，将其存储在*value中并返回true。
  //* 如果memtable中包含一个删除的key，将NotFound()错误存储在*status中并返回true。
  //* 并返回true。
  //* 否则，返回false。
  bool Get(const LookupKey& key, std::string* value, Status* s);

 private:
  friend class MemTableIterator;
  friend class MemTableBackwardIterator;

  struct KeyComparator {
    const InternalKeyComparator comparator;
    explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) {}
    int operator()(const char* a, const char* b) const;
  };

  //* Memtable 的核心数据结构就是跳表
  typedef SkipList<const char*, KeyComparator> Table;

  ~MemTable();  // Private since only Unref() should be used to delete it

  KeyComparator comparator_;
  int refs_;
  Arena arena_;
  Table table_;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_MEMTABLE_H_
