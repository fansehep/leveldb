// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// This file contains the specification, but not the implementations,
// of the types/operations/etc. that should be defined by a platform
// specific port_<platform>.h file.  Use this file as a reference for
// how to port this package to a new platform.

#ifndef STORAGE_LEVELDB_PORT_PORT_EXAMPLE_H_
#define STORAGE_LEVELDB_PORT_PORT_EXAMPLE_H_

#include "port/thread_annotations.h"

namespace leveldb {
namespace port {

// TODO(jorlow): Many of these belong more in the environment class rather than
//               here. We should try moving them and see if it affects perf.

// ------------------ Threading -------------------

// A Mutex represents an exclusive lock.

// TODO(jorlow)。其中很多都属于环境类，而不是
// 这里。我们应该试着移动它们，看看是否会影响性能。

// ------------------ 线程 -------------------

// 一个Mutex代表一个独占锁。

class LOCKABLE Mutex {
 public:
  Mutex();
  ~Mutex();

  // Lock the mutex.  Waits until other lockers have exited.
  // Will deadlock if the mutex is already locked by this thread.
  void Lock() EXCLUSIVE_LOCK_FUNCTION();

  // Unlock the mutex.
  // REQUIRES: This mutex was locked by this thread.
  void Unlock() UNLOCK_FUNCTION();

  // Optionally crash if this thread does not hold this mutex.
  // The implementation must be fast, especially if NDEBUG is
  // defined.  The implementation is allowed to skip all checks.
  void AssertHeld() ASSERT_EXCLUSIVE_LOCK();
};

class CondVar {
 public:
  explicit CondVar(Mutex* mu);
  ~CondVar();

  // Atomically release *mu and block on this condition variable until
  // either a call to SignalAll(), or a call to Signal() that picks
  // this thread to wakeup.
  // REQUIRES: this thread holds *mu
  void Wait();

  // If there are some threads waiting, wake up at least one of them.
  void Signal();

  // Wake up all waiting threads.
  void SignalAll();
};

// ------------------ Compression -------------------

// Store the snappy compression of "input[0,input_length-1]" in *output.
// Returns false if snappy is not supported by this port.
// 在 *output 中存储 "input[0,input_length-1]" 的 snappy 压缩。
//
// 如果这个端口不支持snappy，则返回false。
// 将 input[intput_length] 的内容进行压缩, 并且将压缩内容放入到
// output 中去, 如果不支持 snappy 压缩则返回false
bool Snappy_Compress(const char* input, size_t input_length,
                     std::string* output);

// If input[0,input_length-1] looks like a valid snappy compressed
// buffer, store the size of the uncompressed data in *result and
// return true.  Else return false.
//
// input: input 指针指向的数据
// length: input_length 表明待解压缩的数据的长度
// 如果无法按照Snappy解压缩, 则返回false
// function: 计算解压缩后的数据长度, 结果存储在 result 中.
bool Snappy_GetUncompressedLength(const char* input, size_t length,
                                  size_t* result);

// Attempt to snappy uncompress input[0,input_length-1] into *output.
// Returns true if successful, false if the input is invalid lightweight
// compressed data.
//
// REQUIRES: at least the first "n" bytes of output[] must be writable
// where "n" is the result of a successful call to
// Snappy_GetUncompressedLength.
//
// 尝试将输入[0,input_length-1]快速解压到*输出。
// 如果成功则返回true，如果输入无效则返回false。
// 压缩的数据。
//
// 要求：至少输出[]的前 "n "个字节必须是可写的。
// 其中 "n "是成功调用Snappy的结果。
// Snappy_GetUncompressedLength。
//
// input_data: 参数指针指向的数据
// input_length: 表明待解压缩的数据的长度
// 如果无法进行Snappy解压缩, 则返回
bool Snappy_Uncompress(const char* input_data, size_t input_length,
                       char* output);

// ------------------ Miscellaneous -------------------

// If heap profiling is not supported, returns false.
// Else repeatedly calls (*func)(arg, data, n) and then returns true.
// The concatenation of all "data[0,n-1]" fragments is the heap profile.
bool GetHeapProfile(void (*func)(void*, const char*, int), void* arg);

// Extend the CRC to include the first n bytes of buf.
//
// Returns zero if the CRC cannot be extended using acceleration, else returns
// the newly extended CRC value (which may also be zero).
uint32_t AcceleratedCRC32C(uint32_t crc, const char* buf, size_t size);

}  // namespace port
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_PORT_PORT_EXAMPLE_H_
