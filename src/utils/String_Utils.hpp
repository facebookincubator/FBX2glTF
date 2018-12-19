/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

#if defined(_MSC_VER)
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

namespace StringUtils {

static const unsigned int MAX_PATH_LENGTH = 1024;

enum PathSeparator { PATH_WIN = '\\', PATH_UNIX = '/' };

PathSeparator operator!(const PathSeparator& s);

PathSeparator GetPathSeparator();
const std::string NormalizePath(const std::string& path);

const std::string GetCleanPathString(
    const std::string& path,
    const PathSeparator separator = PATH_WIN);

template <size_t size>
void GetCleanPath(char (&dest)[size], const char* path, const PathSeparator separator = PATH_WIN) {
  size_t len = size - 1;
  strncpy(dest, path, len);
  char* destPtr = dest;
  while ((destPtr = strchr(destPtr, !separator)) != nullptr) {
    *destPtr = separator;
  }
}

const std::string GetFolderString(const std::string& path);
const std::string GetFileNameString(const std::string& path);
const std::string GetFileBaseString(const std::string& path);
const std::string GetFileSuffixString(const std::string& path);

int CompareNoCase(const std::string& s1, const std::string& s2);

} // namespace StringUtils
