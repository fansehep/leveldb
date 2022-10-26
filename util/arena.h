// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_UTIL_ARENA_H_
#define STORAGE_LEVELDB_UTIL_ARENA_H_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace leveldb {

//* leveldb 实现的内存池
// >>__<<???
// 看起来该内存池实现是与某个类绑定一起的.
// 当这个类发生析构的时候, 其实内存也就释放了c
class Arena {
 public:
  Arena();

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  ~Arena();

  // Return a pointer to a newly allocated memory block of "bytes" bytes.
  //* 分配 bytes 大小的内存
  //* @return: 返回分配后的内存段的起始指针地址
  char* Allocate(size_t bytes);

  // Allocate memory with the normal alignment guarantees provided by malloc.
  //* 
  // 以malloc提供的正常对齐保证来分配内存。
  //* 
  char* AllocateAligned(size_t bytes);

  // Returns an estimate of the total memory usage of data allocated by the arena.
  // 返回竞技场分配的数据的总内存使用量的估计值
  size_t MemoryUsage() const {
    return memory_usage_.load(std::memory_order_relaxed);
  }

 private:
  char* AllocateFallback(size_t bytes);
  char* AllocateNewBlock(size_t block_bytes);

  // Allocation state
  char* alloc_ptr_;
  //* 剩余的 alloc_bytes_remaining_.
  size_t alloc_bytes_remaining_;

  // Array of new[] allocated memory blocks
  std::vector<char*> blocks_;

  // Total memory usage of the arena.
  //
  // TODO(costan): This member is accessed via atomics, but the others are
  //               accessed without any locking. Is this OK?
  // 总内存使用量。
  //
  // TODO（costan）。这个成员是通过atomics访问的，但其他成员是
  // 但是其他的成员是在没有任何锁定的情况下访问的。这样做可以吗？
  std::atomic<size_t> memory_usage_;
};

inline char* Arena::Allocate(size_t bytes) {
  // The semantics of what to return are a bit messy if we allow
  // 0-byte allocations, so we disallow them here (we don't need
  // them for our internal use).
  //
  // 如果我们允许返回什么，语义就会有点混乱
  // 0字节的分配，所以我们在这里不允许它们（我们不需要
  // 我们的内部使用不需要它们）。
  //
  assert(bytes > 0);
  if (bytes <= alloc_bytes_remaining_) {
    char* result = alloc_ptr_;
    //* 移动指针, alloc_bytes_remaining_ -= bytes.
    //* 剩余的内存池需要相减
    alloc_ptr_ += bytes;
    alloc_bytes_remaining_ -= bytes;
    return result;
  }
  //* 若当前block不够, 则需要AllocateFallback
  return AllocateFallback(bytes);
}

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_UTIL_ARENA_H_
