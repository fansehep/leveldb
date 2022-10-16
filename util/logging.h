// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Must not be included from any .h files to avoid polluting the namespace
// with macros.

#ifndef STORAGE_LEVELDB_UTIL_LOGGING_H_
#define STORAGE_LEVELDB_UTIL_LOGGING_H_

#include <cstdint>
#include <cstdio>
#include <string>

#include "port/port.h"

namespace leveldb {

class Slice;
class WritableFile;

// Append a human-readable printout of "num" to *str
void AppendNumberTo(std::string* str, uint64_t num);

// Append a human-readable printout of "value" to *str.
// Escapes any non-printable characters found in "value".
//* 将 "value "的人可读的打印结果附加到*str。
//* 转化 "value "中发现的任何不可打印的字符。
void AppendEscapedStringTo(std::string* str, const Slice& value);

// Return a human-readable printout of "num"
std::string NumberToString(uint64_t num);

// Return a human-readable version of "value".
// Escapes any non-printable characters found in "value".
//*
//* 返回 "value "的人类可读版本。
//* 转化 "value "中发现的任何不可打印的字符。
std::string EscapeString(const Slice& value);

// Parse a human-readable number from "*in" into *value.  On success,
// advances "*in" past the consumed number and sets "*val" to the
// numeric value.  Otherwise, returns false and leaves *in in an
// unspecified state.
//*
//* 从 "*in "中解析一个人类可读的数字到*value中。 一旦成功。
//* 将 "*in "推进到所消耗的数字之上，并将 "*val "设置为
//* 数字值。 否则，返回false并使*in处于一个
//* 未指定的状态。
bool ConsumeDecimalNumber(Slice* in, uint64_t* val);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_UTIL_LOGGING_H_
