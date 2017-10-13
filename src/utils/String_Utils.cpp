/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "String_Utils.h"

namespace Gltf {
    namespace StringUtils {

        PathSeparator operator!(const PathSeparator &s)
        {
            return (s == PATH_WIN) ? PATH_UNIX : PATH_WIN;
        }

    }
}// namespace Gltf
