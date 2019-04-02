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

inline int CompareNoCase(const std::string& s1, const std::string& s2) {
  return strncasecmp(s1.c_str(), s2.c_str(), std::max(s1.length(), s2.length()));
}

} // namespace StringUtils
