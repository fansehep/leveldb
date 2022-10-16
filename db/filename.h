// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// File names used by DB code

#ifndef STORAGE_LEVELDB_DB_FILENAME_H_
#define STORAGE_LEVELDB_DB_FILENAME_H_

#include <cstdint>
#include <string>

#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "port/port.h"

namespace leveldb {

class Env;

enum FileType {
  kLogFile,
  kDBLockFile,
  kTableFile,
  kDescriptorFile,
  kCurrentFile,
  kTempFile,
  kInfoLogFile  // Either the current one, or an old one
};

// Return the name of the log file with the specified number
// in the db named by "dbname".  The result will be prefixed with
// "dbname".
//* 返回指定编号的日志文件的名称
//* 在以 "dbname "命名的数据库中。 结果的前缀将是
//* "dbname"。
std::string LogFileName(const std::string& dbname, uint64_t number);

// Return the name of the sstable with the specified number
// in the db named by "dbname".  The result will be prefixed with
// "dbname".
//*
//* 返回指定编号的sstable的名称
//* 在以 "dbname "命名的数据库中。 结果的前缀将是
//* "dbname"。
std::string TableFileName(const std::string& dbname, uint64_t number);

// Return the legacy file name for an sstable with the specified number
// in the db named by "dbname". The result will be prefixed with
// "dbname".
//*
//* 返回指定编号的sstable的遗留文件名
//* 在以 "dbname "命名的数据库中的sstable的传统文件名。结果的前缀将是
//* "dbname"。
std::string SSTTableFileName(const std::string& dbname, uint64_t number);

// Return the name of the descriptor file for the db named by
// "dbname" and the specified incarnation number.  The result will be
// prefixed with "dbname".
//*
//* 返回以 "dbname "命名的数据库的描述符文件的名称和指定的版本号。
//* "dbname "和指定的版本号命名的db的描述符文件名称。 结果将是
//* 以 "dbname "为前缀。
std::string DescriptorFileName(const std::string& dbname, uint64_t number);

// Return the name of the current file.  This file contains the name
// of the current manifest file.  The result will be prefixed with
// "dbname".
//
//* 返回当前文件的名称。 该文件包含
//* 当前清单文件的名称。 结果的前缀将是
//* "dbname"。
std::string CurrentFileName(const std::string& dbname);

// Return the name of the lock file for the db named by
// "dbname".  The result will be prefixed with "dbname".
//*
//* 返回以 "dbname "命名的数据库的锁文件的名称。
//* "dbname "命名的数据库的锁文件名称。 该结果将以 "dbname "为前缀。
std::string LockFileName(const std::string& dbname);

// Return the name of a temporary file owned by the db named "dbname".
// The result will be prefixed with "dbname".
//
//* 返回由db拥有的名为 "dbname "的临时文件的名称。
//* 结果将以 "dbname "为前缀。
std::string TempFileName(const std::string& dbname, uint64_t number);

// Return the name of the info log file for "dbname".
//* 返回 "dbname "的信息日志文件的名称。
std::string InfoLogFileName(const std::string& dbname);

// Return the name of the old info log file for "dbname".
//* 返回 "dbname "的旧信息日志文件的名称。
std::string OldInfoLogFileName(const std::string& dbname);

// If filename is a leveldb file, store the type of the file in *type.
// The number encoded in the filename is stored in *number.  If the
// filename was successfully parsed, returns true.  Else return false.
//*
//* 如果文件名是一个leveldb文件，在*type中存储该文件的类型。
//* 在文件名中编码的数字被存储在*number中。 如果
//* 文件名被成功解析，返回true。 否则返回false。
bool ParseFileName(const std::string& filename, uint64_t* number,
                   FileType* type);

// Make the CURRENT file point to the descriptor file with the
// specified number.
Status SetCurrentFile(Env* env, const std::string& dbname,
                      uint64_t descriptor_number);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_FILENAME_H_
