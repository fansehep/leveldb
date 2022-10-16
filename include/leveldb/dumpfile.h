// Copyright (c) 2014 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_INCLUDE_DUMPFILE_H_
#define STORAGE_LEVELDB_INCLUDE_DUMPFILE_H_

#include <string>

#include "leveldb/env.h"
#include "leveldb/export.h"
#include "leveldb/status.h"

namespace leveldb {

// Dump the contents of the file named by fname in text format to
// *dst.  Makes a sequence of dst->Append() calls; each call is passed
// the newline-terminated text corresponding to a single item found
// in the file.
//
// Returns a non-OK result if fname does not name a leveldb storage
// file, or if the file cannot be read.
//
// 将fname命名的文件的内容以文本格式转储到
// *dst。 进行一连串的dst->Append()调用；每次调用都会传给
// 与在文件中找到的单个项目相对应的以新行结尾的文本。
// 文件中的一个项目。
//
// 如果fname没有命名一个leveldb存储文件，或者如果该文件不能使用，则返回一个非确定的结果。
// 如果fname没有命名一个leveldb存储文件，或者该文件不能被读取，则返回一个非OK的结果。

LEVELDB_EXPORT Status DumpFile(Env* env, const std::string& fname,
                               WritableFile* dst);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_DUMPFILE_H_
