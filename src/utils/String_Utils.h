/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef _STRING_UTILS_H__
#define _STRING_UTILS_H__

#include <string>
#include <cstdio>
#include <cstring>
#include <cstdarg>

#if defined( _MSC_VER )
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

namespace StringUtils {

    static const unsigned int MAX_PATH_LENGTH = 1024;

    enum PathSeparator
    {
        PATH_WIN  = '\\',
        PATH_UNIX = '/'
    };

    PathSeparator operator!(const PathSeparator &s);

    inline const std::string GetCleanPathString(const std::string &path, const PathSeparator separator = PATH_WIN)
    {
        std::string cleanPath = path;
        for (size_t s = cleanPath.find(!separator, 0); s != std::string::npos; s = cleanPath.find(!separator, s)) {
            cleanPath[s] = separator;
        }
        return cleanPath;
    }
    template<size_t size>
    inline void GetCleanPath(char (&dest)[size], const char *path, const PathSeparator separator = PATH_WIN)
    {
        size_t len = size - 1;
        strncpy(dest, path, len);
        char *destPtr = dest;
        while ((destPtr = strchr(destPtr, !separator)) != nullptr) {
            *destPtr = separator;
        }
    }

    inline const std::string GetFolderString(const std::string &path)
    {
        size_t s = path.rfind(PATH_WIN);
        s = (s != std::string::npos) ? s : path.rfind(PATH_UNIX);
        return path.substr(0, s + 1);
    }

    inline const std::string GetFileNameString(const std::string &path)
    {
        size_t s = path.rfind(PATH_WIN);
        s = (s != std::string::npos) ? s : path.rfind(PATH_UNIX);
        return path.substr(s + 1, std::string::npos);
    }

    inline const std::string GetFileBaseString(const std::string &path)
    {
        const std::string fileName = GetFileNameString(path);
        return fileName.substr(0, fileName.rfind('.')).c_str();
    }

    inline const std::string GetFileSuffixString(const std::string &path)
    {
        const std::string fileName = GetFileNameString(path);
        size_t pos = fileName.rfind('.');
        if (pos == std::string::npos) {
            return "";
        }
        return fileName.substr(++pos);
    }

    inline int CompareNoCase(const std::string &s1, const std::string &s2)
    {
        return strncasecmp(s1.c_str(), s2.c_str(), MAX_PATH_LENGTH);
    }

} // StringUtils
#endif // _STRING_UTILS_H__

