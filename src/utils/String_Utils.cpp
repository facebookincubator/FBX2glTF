/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "String_Utils.hpp"

#include <algorithm>

namespace StringUtils {

int CompareNoCase(const std::string& s1, const std::string& s2) {
  return strncasecmp(s1.c_str(), s2.c_str(), std::max(s1.length(), s2.length()));
}

} // namespace StringUtils
