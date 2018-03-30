/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "String_Utils.h"

namespace StringUtils {

    PathSeparator operator!(const PathSeparator &s)
    {
        return (s == PATH_WIN) ? PATH_UNIX : PATH_WIN;
    }

        PathSeparator GetPathSeparator() {
        #if defined( __unix__ ) || defined( __APPLE__ )
            return PATH_UNIX;
        #else
            return PATH_WIN;
        #endif
    }

    const std::string NormalizePath(const std::string &path)
    {
        PathSeparator separator = GetPathSeparator();

        char replace;

        if (separator == PATH_WIN) {
            replace = PATH_UNIX;
        }
        else {
            replace = PATH_WIN;
        }

        std::string normalizedPath = path;
        for (size_t s = normalizedPath.find(replace, 0); s != std::string::npos; s = normalizedPath.find(replace, s)) {
            normalizedPath[s] = separator;
        }
        return normalizedPath;
    }

}
