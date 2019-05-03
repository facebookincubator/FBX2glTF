/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

#if defined(_MSC_VER)
#define strncasecmp _strnicmp
#endif

namespace StringUtils {

inline std::string ToLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](uint8_t c) { return std::tolower(c); });
  return s;
}

inline int CompareNoCase(const std::string& s1, const std::string& s2) {
  return strncasecmp(s1.c_str(), s2.c_str(), std::max(s1.length(), s2.length()));
}

} // namespace StringUtils
