// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_INCLUDE_DB_H_
#define STORAGE_LEVELDB_INCLUDE_DB_H_

#include <cstdint>
#include <cstdio>

#include "leveldb/export.h"
#include "leveldb/iterator.h"
#include "leveldb/options.h"

namespace leveldb {

// Update CMakeLists.txt if you change these
static const int kMajorVersion = 1;
static const int kMinorVersion = 23;

struct Options;
struct ReadOptions;
struct WriteOptions;
class WriteBatch;

// Abstract handle to particular state of a DB.
// A Snapshot is an immutable object and can therefore be safely
// accessed from multiple threads without any external synchronization.
class LEVELDB_EXPORT Snapshot {
 protected:
  virtual ~Snapshot();
};

// A range of keys
struct LEVELDB_EXPORT Range {
  Range() = default;
  Range(const Slice& s, const Slice& l) : start(s), limit(l) {}

  Slice start;  // Included in the range
  Slice limit;  // Not included in the range
};

/*
 * key的命名设计
 * leveldb 中磁盘数据读取与缓存均以块为单位, 并且实际存储中
 * 所有的数据记录均以 key 进行顺序存储.
 * 根据排序结果, 相邻的key所对应的数据记录一般会存储在同一个块中,
 * 由于该特性: 用户针对自身的应用场景需要充分考虑如何优化key
 * 的命名设计, 从而最大限度地提升整体的读取性能.
 * 在 key 命名时, 通过设置相同的前缀保证这些数据的key是
 */

// A DB is a persistent ordered map from keys to values.
// A DB is safe for concurrent access from multiple threads without
// any external synchronization.
//* DB 是 key => value 持久性有序映射
//* DB 对于多个线程的并发访问是线程安全的, 不需要任何同步
class LEVELDB_EXPORT DB {
 public:
  // Open the database with the specified "name".
  // Stores a pointer to a heap-allocated database in *dbptr and returns
  // OK on success.
  // Stores nullptr in *dbptr and returns a non-OK status on error.
  // Caller should delete *dbptr when it is no longer needed.

  /*
   * leveldb::DB* db = nullptr;
   * leveldb::Options op;
   * 如果没有该数据库就创建
   * op.create_if_missing = true;
   * leveldb::Status status = leveldb::DB::Open(op, "/tmp/test_db", &db);
   * assert(status.ok() == true);
   * delete db;
   */
  //* google code style means that every function
  //* output args must after input args
  static Status Open(const Options& options, const std::string& name,
                     DB** dbptr);

  DB() = default;

  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;

  virtual ~DB();

  // Set the database entry for "key" to "value".  Returns OK on success,
  // and a non-OK status on error.
  // Note: consider setting options.sync = true.
  virtual Status Put(const WriteOptions& options, const Slice& key,
                     const Slice& value) = 0;

  // Remove the database entry (if any) for "key".  Returns OK on
  // success, and a non-OK status on error.  It is not an error if "key"
  // did not exist in the database.
  // Note: consider setting options.sync = true.
  //* 删除 key, 我们也认为 remove key 也是一种写操作
  //* 但并不是真正的将数据从磁盘中删除, 而是在对应位置插入一个key的标志物
  //* 在后续的 compaction 中才会去除这一条 key-value 记录
  virtual Status Delete(const WriteOptions& options, const Slice& key) = 0;

  // Apply the specified updates to the database.
  // Returns OK on success, non-OK on failure.
  // Note: consider setting options.sync = true.
  //* 批量写入的一种方式
  //* 更多详见 leveldb/include/leveldb/write_batch.h
  //* 可以定义一个 WriteBatch对象, 以达到批量写入的目的
  virtual Status Write(const WriteOptions& options, WriteBatch* updates) = 0;

  // If the database contains an entry for "key" store the
  // corresponding value in *value and return OK.
  //
  // If there is no entry for "key" leave *value unchanged and return
  // a status for which Status::IsNotFound() returns true.
  //
  // May return some other Status on an error.
  //
  // 如果数据库中包含一个 "key "的条目，则在*value中存储相应的值并返回OK。
  // 相应的值在*value中并返回OK。
  //
  // 如果没有 "key "的条目，则保持*value不变并返回
  // 一个Status::IsNotFound()返回true的状态。
  //
  // 可以在出错时返回其他状态。
  //
  virtual Status Get(const ReadOptions& options, const Slice& key,
                     std::string* value) = 0;

  // Return a heap-allocated iterator over the contents of the database.
  // The result of NewIterator() is initially invalid (caller must
  // call one of the Seek methods on the iterator before using it).
  //
  // Caller should delete the iterator when it is no longer needed.
  // The returned iterator should be deleted before this db is deleted.
  //* NewIterator, 一个新的迭代器
  // auto iter = db->NewIterator(leveldb::ReadOptions());
  //
  // 前向遍历: for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {}
  // 后向遍历: for (iter->SeekToLast(); iter->Valid(); iter->Prev()) {}
  // 当不再需要该迭代器时, 可以直接使用 delete 对该指针进行删除
  //* simple example:
  //* for (iter->Seek("k2"); iter->Valid() &&
  //* iter->Key().ToString() < "k4") {
  //* std::cout << iter->key().ToString() << ": "
  //* << iter->value().ToString() << std::endl; }
  //* 通过 for 循环定位到 key = "k2" 的数据, 然后通过Next向后迭代
  //* , 并调用
  virtual Iterator* NewIterator(const ReadOptions& options) = 0;

  // Return a handle to the current DB state.  Iterators created with
  // this handle will all observe a stable snapshot of the current DB
  // state.  The caller must call ReleaseSnapshot(result) when the
  // snapshot is no longer needed.
  //
  // 返回一个当前DB状态的句柄。 用这个句柄创建的迭代器
  // 创建的迭代器都将观察到当前数据库的稳定快照。
  // 状态。 当不再需要快照时，调用者必须调用 ReleaseSnapshot(result)。
  // 不再需要快照时，调用者必须调用ReleaseSnapshot(result)。
  virtual const Snapshot* GetSnapshot() = 0;

  // Release a previously acquired snapshot.  The caller must not
  // use "snapshot" after this call.
  //
  // 释放一个先前获得的快照。 调用者不得
  // 在此调用之后使用 "snapshot"。
  virtual void ReleaseSnapshot(const Snapshot* snapshot) = 0;

  // DB implementations can export properties about their state
  // via this method.  If "property" is a valid property understood by this
  // DB implementation, fills "*value" with its current value and returns
  // true.  Otherwise returns false.
  //
  //
  // Valid property names include:
  //
  //  "leveldb.num-files-at-level<N>" - return the number of files at level <N>,
  //     where <N> is an ASCII representation of a level number (e.g. "0").
  //  "leveldb.stats" - returns a multi-line string that describes statistics
  //     about the internal operation of the DB.
  //  "leveldb.sstables" - returns a multi-line string that describes all
  //     of the sstables that make up the db contents.
  //  "leveldb.approximate-memory-usage" - returns the approximate number of
  //     bytes of memory in use by the DB.
  //
  // DB的实现可以通过这个方法输出关于其状态的属性。
  // 通过这个方法。 如果 "property "是一个有效的属性，被这个DB实现所理解
  // DB实现，则用其当前值填充 "*value "并返回
  // true。 否则返回false。
  //
  //
  // 有效的属性名称包括。
  //
  // "leveldb.num-files-at-level<N>" - 返回<N>级的文件数。
  // 其中<N>是一个ASCII表示的级别号（例如 "0"）。
  // "leveldb.stats" - 返回一个多行字符串，描述了关于数据库内部操作的统计数据
  // 关于数据库的内部操作。
  // "leveldb.sstables" - 返回一个多行字符串，描述所有的
  // 组成数据库内容的所有sstables。
  // "leveldb.approximate-memory-usage" - 返回数据库使用的内存的大约数量。
  // DB所使用的内存的大约数量。
  //
  virtual bool GetProperty(const Slice& property, std::string* value) = 0;

  // For each i in [0,n-1], store in "sizes[i]", the approximate
  // file system space used by keys in "[range[i].start .. range[i].limit)".
  //
  // Note that the returned sizes measure file system space usage, so
  // if the user data compresses by a factor of ten, the returned
  // sizes will be one-tenth the size of the corresponding user data size.
  //
  // The results may not include the sizes of recently written data.
  //
  // 对于[0,n-1]中的每一个i，在 "size[i]"中存储"[range[i].start . range[i].limit]"中的键所使用的大约
  // "[range[i].start ... range[i].limit) "中的键所使用的文件系统空间。
  //
  // 注意，返回的大小是衡量文件系统空间使用情况的，所以
  // 如果用户数据压缩了10倍，那么返回的
  // 的大小将是相应用户数据大小的十分之一。
  //
  // 结果可能不包括最近写入的数据的大小。
  virtual void GetApproximateSizes(const Range* range, int n,
                                   uint64_t* sizes) = 0;

  // Compact the underlying storage for the key range [*begin,*end].
  // In particular, deleted and overwritten versions are discarded,
  // and the data is rearranged to reduce the cost of operations
  // needed to access the data.  This operation should typically only
  // be invoked by users who understand the underlying implementation.
  //
  // begin==nullptr is treated as a key before all keys in the database.
  // end==nullptr is treated as a key after all keys in the database.
  // Therefore the following call will compact the entire database:
  //    db->CompactRange(nullptr, nullptr);
  //
  // 压缩键值范围[*begin,*end]的基础存储。
  // 特别是，删除和覆盖的版本被丢弃了。
  // 并且数据被重新排列，以减少访问数据所需的操作成本。
  // 访问数据所需的操作成本。 这个操作通常只应该
  // 由了解底层实现的用户来调用。
  //
  // begin==nullptr被视为数据库中所有键之前的一个键。
  // end==nullptr被视为数据库中所有键之后的一个键。
  // 因此下面的调用将压缩整个数据库。
  // db->CompactRange(nullptr, nullptr)。
  virtual void CompactRange(const Slice* begin, const Slice* end) = 0;
};

// Destroy the contents of the specified database.
// Be very careful using this method.
//
// Note: For backwards compatibility, if DestroyDB is unable to list the
// database files, Status::OK() will still be returned masking this failure.
//
// 销毁指定数据库的内容。
// 使用这个方法要非常小心。
//
// 注意: 为了向后兼容，如果DestroyDB无法列出
// 数据库文件，Status::OK()仍将被返回，以掩盖这一失败。
LEVELDB_EXPORT Status DestroyDB(const std::string& name,
                                const Options& options);

// If a DB cannot be opened, you may attempt to call this method to
// resurrect as much of the contents of the database as possible.
// Some data may be lost, so be careful when calling this function
// on a database that contains important information.
//
// 如果一个数据库不能被打开，你可以尝试调用这个方法来
// 尽可能多地复活数据库的内容。
// 有些数据可能会丢失，所以在调用这个函数时要小心。
// 时要小心，因为它包含了重要的信息。
LEVELDB_EXPORT Status RepairDB(const std::string& dbname,
                               const Options& options);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_DB_H_
