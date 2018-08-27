/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "String_Utils.hpp"

namespace StringUtils {

    PathSeparator operator!(const PathSeparator &s)
    {
        return (s == PATH_WIN) ? PATH_UNIX : PATH_WIN;
    }

    const std::string GetFolderString(const std::string &path)
    {
        size_t s = path.rfind(PATH_WIN);
        s = (s != std::string::npos) ? s : path.rfind(PATH_UNIX);
        return path.substr(0, s + 1);
    }

    const std::string GetCleanPathString(const std::string &path, const PathSeparator separator)
    {
        std::string cleanPath = path;
        for (size_t s = cleanPath.find(!separator, 0); s != std::string::npos; s = cleanPath.find(!separator, s)) {
            cleanPath[s] = separator;
        }
        return cleanPath;
    }

    const std::string GetFileNameString(const std::string &path)
    {
        size_t s = path.rfind(PATH_WIN);
        s = (s != std::string::npos) ? s : path.rfind(PATH_UNIX);
        return path.substr(s + 1, std::string::npos);
    }

    const std::string GetFileBaseString(const std::string &path)
    {
        const std::string fileName = GetFileNameString(path);
        return fileName.substr(0, fileName.rfind('.')).c_str();
    }

    const std::string GetFileSuffixString(const std::string &path)
    {
        const std::string fileName = GetFileNameString(path);
        size_t pos = fileName.rfind('.');
        if (pos == std::string::npos) {
            return "";
        }
        return fileName.substr(++pos);
    }

    int CompareNoCase(const std::string &s1, const std::string &s2)
    {
        return strncasecmp(s1.c_str(), s2.c_str(), MAX_PATH_LENGTH);
    }

}
