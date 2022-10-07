// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_PORT_PORT_H_
#define STORAGE_LEVELDB_PORT_PORT_H_

#include <string.h>

// Include the appropriate platform specific file below.  If you are
// porting to a new platform, see "port_example.h" for documentation
// of what the new port_<platform>.h file must provide.

// 包括下面适当的平台特定文件。 如果你要
// 移植到一个新的平台上， 请参阅 "port_example.h" 以了解文件的内容。
// 新的 port_<platform>.h 文件必须提供什么。

#if defined(LEVELDB_PLATFORM_POSIX) || defined(LEVELDB_PLATFORM_WINDOWS)
#include "port/port_stdcxx.h"
#elif defined(LEVELDB_PLATFORM_CHROMIUM)
#include "port/port_chromium.h"
#endif

#endif  // STORAGE_LEVELDB_PORT_PORT_H_
