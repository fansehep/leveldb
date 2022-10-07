// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
#define STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

#include <cstddef>

#include "leveldb/export.h"

namespace leveldb {

class Cache;
class Comparator;
class Env;
class FilterPolicy;
class Logger;
class Snapshot;

// DB contents are stored in a set of blocks, each of which holds a
// sequence of key,value pairs.  Each block may be compressed before
// being stored in a file.  The following enum describes which
// compression method (if any) is used to compress a block.

// DB的内容被存储在一组块中，每个块中都有一个
// 键值对的序列。 每个块在存储到文件中之前都可以被压缩。
// 被存储在一个文件中。 下面的枚举描述了哪种
// 压缩方法（如果有的话）来压缩一个块。

//* enum CompressionType {}
//* LevelDB 是以块为单元进行文件读取的,
//* 1. 如果调用者需要针对大块的数据进行遍历访问或操作时, 可以适当增加
//*    增大块的大小. 从而减少磁盘 IO 的次数.
//* 2. 如果调用者的应用场景是频繁地一次读取少数数据点, 那么可以适当减少
//*    块的大小, 进而提高块的读取速度.

enum CompressionType {
  // NOTE: do not change the values of existing entries, as these are
  // part of the persistent format on disk.
  // 注意：不要改变现有条目的值，因为这些是磁盘上的持久格式。
  // 是磁盘上持久化格式的一部分。
  kNoCompression = 0x0,
  kSnappyCompression = 0x1
};

// Options to control the behavior of a database (passed to DB::Open)
struct LEVELDB_EXPORT Options {
  // Create an Options object with default values for all fields.
  Options();

  // -------------------
  // Parameters that affect behavior

  // Comparator used to define the order of keys in the table.
  // Default: a comparator that uses lexicographic byte-wise ordering
  //
  // REQUIRES: The client must ensure that the comparator supplied
  // here has the same name and orders keys *exactly* the same as the
  // comparator provided to previous open calls on the same DB.
  const Comparator* comparator;

  // If true, the database will be created if it is missing.
  bool create_if_missing = false;

  // If true, an error is raised if the database already exists.
  bool error_if_exists = false;

  // If true, the implementation will do aggressive checking of the
  // data it is processing and will stop early if it detects any
  // errors.  This may have unforeseen ramifications: for example, a
  // corruption of one DB entry may cause a large number of entries to
  // become unreadable or for the entire DB to become unopenable.
  bool paranoid_checks = false;

  // Use the specified object to interact with the environment,
  // e.g. to read/write files, schedule background work, etc.
  // Default: Env::Default()
  Env* env;

  // Any internal progress/error information generated by the db will
  // be written to info_log if it is non-null, or to a file stored
  // in the same directory as the DB contents if info_log is null.
  Logger* info_log = nullptr;

  // -------------------
  // Parameters that affect performance

  // Amount of data to build up in memory (backed by an unsorted log
  // on disk) before converting to a sorted on-disk file.
  //
  // Larger values increase performance, especially during bulk loads.
  // Up to two write buffers may be held in memory at the same time,
  // so you may wish to adjust this parameter to control memory usage.
  // Also, a larger write buffer will result in a longer recovery time
  // the next time the database is opened.
  /*
   * 内存中将要写入到硬盘文件的数据量大小(sorted on -disk file)
   * 默认是 4 MB 
   */
  size_t write_buffer_size = 4 * 1024 * 1024;

  // Number of open files that can be used by the DB.  You may need to
  // increase this if your database has a large working set (budget
  // one open file per 2MB of working set).
  //* leveldb 默认最大能打开的文件数量
  int max_open_files = 1000;

  // Control over blocks (user data is stored in a set of blocks, and
  // a block is the unit of reading from disk).

  // If non-null, use the specified cache for blocks.
  // If null, leveldb will automatically create and use an 8MB internal cache.
  // 
  //
  Cache* block_cache = nullptr;

  // Approximate size of user data packed per block.  Note that the
  // block size specified here corresponds to uncompressed data.  The
  // actual size of the unit read from disk may be smaller if
  // compression is enabled.  This parameter can be changed dynamically.
  size_t block_size = 4 * 1024;

  // Number of keys between restart points for delta encoding of keys.
  // This parameter can be changed dynamically.  Most clients should
  // leave this parameter alone.
  int block_restart_interval = 16;

  // Leveldb will write up to this amount of bytes to a file before
  // switching to a new one.
  // Most clients should leave this parameter alone.  However if your
  // filesystem is more efficient with larger files, you could
  // consider increasing the value.  The downside will be longer
  // compactions and hence longer latency/performance hiccups.
  // Another reason to increase this parameter might be when you are
  // initially populating a large database.
  //* 向一个文件写入的最大值, 默认是 2MB,
  //Leveldb在切换到一个新的文件之前，将向该文件写入最多这个字节数。
  // 切换到一个新的文件。
  // 大多数客户应该不使用这个参数。 但是如果你的
  // 文件系统对较大的文件更有效，你可以
  // 考虑增加这个值。 这样做的坏处是会延长
  // 缩减时间，从而导致更长的延迟/性能障碍。
  // 增加这个参数的另一个原因可能是当你正在
  // 最初填充一个大型数据库。
  size_t max_file_size = 2 * 1024 * 1024;

  // Compress blocks using the specified compression algorithm.  This
  // parameter can be changed dynamically.
  //
  // Default: kSnappyCompression, which gives lightweight but fast
  // compression.
  //
  // Typical speeds of kSnappyCompression on an Intel(R) Core(TM)2 2.4GHz:
  //    ~200-500MB/s compression
  //    ~400-800MB/s decompression
  // Note that these speeds are significantly faster than most
  // persistent storage speeds, and therefore it is typically never
  // worth switching to kNoCompression.  Even if the input data is
  // incompressible, the kSnappyCompression implementation will
  // efficiently detect that and will switch to uncompressed mode.
  // 使用指定的压缩算法对块进行压缩。 这个
  // 参数可以动态地改变。
  //
  // 默认：kSnappyCompression，它提供轻量级但快速的
  // 压缩。
  //
  // kSnappyCompression在Intel(R) Core(TM) 2 2.4GHz上的典型速度。
  // ~200-500MB/s的压缩速度
  // ~400-800MB/s解压缩
  // 请注意，这些速度明显快于大多数
  // 的持久性存储速度，因此，通常情况下永远不
  // 值得切换到kNoCompression。 即使输入的数据是
  // 不可压缩的，kSnappyCompression的实现也会
  // 有效地检测到这一点，并将切换到非压缩模式。
  CompressionType compression = kSnappyCompression;

  // EXPERIMENTAL: If true, append to existing MANIFEST and log files
  // when a database is opened.  This can significantly speed up open.
  //
  // Default: currently false, but may become true later.
  // 实验性的：如果是真的，在数据库打开时追加到现有的MANIFEST和日志文件中。
  // 当一个数据库被打开时。 这可以大大加快打开速度。
  //
  // 默认值：目前为false，但以后可能变成true。
  //
  bool reuse_logs = false;

  // If non-null, use the specified filter policy to reduce disk reads.
  // Many applications will benefit from passing the result of
  // NewBloomFilterPolicy() here.
  // 默认是 nullptr, 如果不为 nullptr, 用户可以指定相应的过滤策略,
  // 以减少磁盘读取次数
  const FilterPolicy* filter_policy = nullptr;
};

// Options that control read operations
struct LEVELDB_EXPORT ReadOptions {
  // If true, all data read from underlying storage will be
  // verified against corresponding checksums.
  // 是否启用校验和, 当读数据时, 会对数据的校验和进行验证, 从而保证数据的一致性
  bool verify_checksums = false;

  // Should the data read for this iteration be cached in memory?
  // Callers may wish to set this field to false for bulk scans.
  bool fill_cache = true;

  // If "snapshot" is non-null, read as of the supplied snapshot
  // (which must belong to the DB that is being read and which must
  // not have been released).  If "snapshot" is null, use an implicit
  // snapshot of the state at the beginning of this read operation.
  const Snapshot* snapshot = nullptr;
};

// Options that control write operations
struct LEVELDB_EXPORT WriteOptions {
  WriteOptions() = default;

  // If true, the write will be flushed from the operating system
  // buffer cache (by calling WritableFile::Sync()) before the write
  // is considered complete.  If this flag is true, writes will be
  // slower.
  //
  // If this flag is false, and the machine crashes, some recent
  // writes may be lost.  Note that if it is just the process that
  // crashes (i.e., the machine does not reboot), no writes will be
  // lost even if sync==false.
  //
  // In other words, a DB write with sync==false has similar
  // crash semantics as the "write()" system call.  A DB write
  // with sync==true has similar crash semantics to a "write()"
  // system call followed by "fsync()".
  bool sync = false;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
