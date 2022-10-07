// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Simple hash function used for internal data structures

#ifndef STORAGE_LEVELDB_UTIL_HASH_H_
#define STORAGE_LEVELDB_UTIL_HASH_H_

#include <cstddef>
#include <cstdint>

namespace leveldb {


/**
 * @brief : 对 data[0:n] 返回一个 uint32_t 的值
 * 
 * @param data : 数据的起始地址
 * @param n : 数据的偏移量
 * @param seed : 
 * @return uint32_t 
 */
uint32_t Hash(const char* data, size_t n, uint32_t seed);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_UTIL_HASH_H_
