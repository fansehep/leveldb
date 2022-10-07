// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A database can be configured with a custom FilterPolicy object.
// This object is responsible for creating a small filter from a set
// of keys.  These filters are stored in leveldb and are consulted
// automatically by leveldb to decide whether or not to read some
// information from disk. In many cases, a filter can cut down the
// number of disk seeks form a handful to a single disk seek per
// DB::Get() call.
//
// Most people will want to use the builtin bloom filter support (see
// NewBloomFilterPolicy() below).

#ifndef STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_
#define STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_

#include <string>

#include "leveldb/export.h"

namespace leveldb {

class Slice;

class LEVELDB_EXPORT FilterPolicy {
 public:
  virtual ~FilterPolicy();

  // Return the name of this policy.  Note that if the filter encoding
  // changes in an incompatible way, the name returned by this method
  // must be changed.  Otherwise, old incompatible filters may be
  // passed to methods of this type.
  virtual const char* Name() const = 0;

  // keys[0,n-1] contains a list of keys (potentially with duplicates)
  // that are ordered according to the user supplied comparator.
  // Append a filter that summarizes keys[0,n-1] to *dst.
  //
  // Warning: do not change the initial contents of *dst.  Instead,
  // append the newly constructed filter to *dst.
  virtual void CreateFilter(const Slice* keys, int n,
                            std::string* dst) const = 0;

  // "filter" contains the data appended by a preceding call to
  // CreateFilter() on this class.  This method must return true if
  // the key was in the list of keys passed to CreateFilter().
  // This method may return true or false if the key was not on the
  // list, but it should aim to return false with a high probability.
  virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0;
};

// Return a new filter policy that uses a bloom filter with approximately
// the specified number of bits per key.  A good value for bits_per_key
// is 10, which yields a filter with ~ 1% false positive rate.
//
// Callers must delete the result after any database that is using the
// result has been closed.
//
// Note: if you are using a custom comparator that ignores some parts
// of the keys being compared, you must not use NewBloomFilterPolicy()
// and must provide your own FilterPolicy that also ignores the
// corresponding parts of the keys.  For example, if the comparator
// ignores trailing spaces, it would be incorrect to use a
// FilterPolicy (like NewBloomFilterPolicy) that does not ignore
// trailing spaces in keys.
// 返回一个新的过滤策略，该策略使用了一个Bloom过滤器，大约有
// 指定的每个密钥的比特数。 每键比特数的一个好值是10。
// 是10，这将产生一个假阳性率为1%的过滤器。
//
// 调用者必须在任何使用该结果的数据库关闭后删除该结果。
// 结果的数据库被关闭后，调用者必须删除该结果。
//
// 注意：如果你使用的是一个自定义的比较器，该比较器忽略了被比较的键的某些部分。
// 的某些部分，你就不能使用NewBloomFilterPolicy()
// 而必须提供你自己的FilterPolicy，该Policy也忽略了
// 键的相应部分。 例如，如果比较器
// 忽略了尾部的空格，那么使用一个
// 忽略键值的FilterPolicy（如NewBloomFilterPolicy）是不对的。
// 键中的尾部空格。

//* leveldb::DB* db
//* leveldb::Options op;
//* op.filter_policy = leveldb::NewBloomFilterPolicy(10);
//* leveldb::Status status = leveldb::DB::Open(op, "/tmp/test_db", &db);
LEVELDB_EXPORT const FilterPolicy* NewBloomFilterPolicy(int bits_per_key);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_
