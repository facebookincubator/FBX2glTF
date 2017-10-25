/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <string>
#include <vector>
#include <fstream>

#include <stdint.h>
#include <stdio.h>

#if defined( __unix__ ) || defined ( __APPLE__ )

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#define _getcwd getcwd
#define _mkdir(a) mkdir(a, 0777)
#elif defined( _WIN32 )
#include <direct.h>
#include <process.h>
#else
#include <direct.h>
#include <process.h>
#endif

#include <sys/stat.h>

#include "FBX2glTF.h"
#include "String_Utils.h"

namespace FileUtils {

    std::string GetCurrentFolder()
    {
        char cwd[Gltf::StringUtils::MAX_PATH_LENGTH];
        if (!_getcwd(cwd, sizeof(cwd))) {
            return std::string();
        }
        cwd[sizeof(cwd) - 1] = '\0';
        Gltf::StringUtils::GetCleanPath(cwd, cwd, Gltf::StringUtils::PATH_UNIX);
        const size_t length = strlen(cwd);
        if (cwd[length - 1] != '/' && length < Gltf::StringUtils::MAX_PATH_LENGTH - 1) {
            cwd[length + 0] = '/';
            cwd[length + 1] = '\0';
        }
        return std::string(cwd);
    }

    bool FolderExists(const std::string &folderPath)
    {
#if defined( __unix__ ) || defined( __APPLE__ )
        DIR *dir = opendir(folderPath.c_str());
        if (dir) {
            closedir(dir);
            return true;
        }
        return false;
#else
        const DWORD ftyp = GetFileAttributesA( folderPath.c_str() );
        if ( ftyp == INVALID_FILE_ATTRIBUTES )
        {
            return false;  // bad path
        }
        return ( ftyp & FILE_ATTRIBUTE_DIRECTORY ) != 0;
#endif
    }

    bool MatchExtension(const char *fileExtension, const char *matchExtensions)
    {
        if (matchExtensions[0] == '\0') {
            return true;
        }
        if (fileExtension[0] == '.') {
            fileExtension++;
        }
        for (const char *end = matchExtensions; end[0] != '\0';) {
            for (; end[0] == ';'; end++) {}
            const char *ext = end;
            for (; end[0] != ';' && end[0] != '\0'; end++) {}
#if defined( __unix__ ) || defined( __APPLE__ )
            if (strncasecmp(fileExtension, ext, end - ext) == 0)
#else
                if ( _strnicmp( fileExtension, ext, end - ext ) == 0 )
#endif
            {
                return true;
            }
        }
        return false;
    }

    std::vector<std::string> ListFolderFiles(const char *folder, const char *matchExtensions)
    {
        std::vector<std::string> fileList;
#if defined( __unix__ ) || defined( __APPLE__ )
        DIR *dir = opendir(strlen(folder) > 0 ? folder : ".");
        if (dir != nullptr) {
            for (;;) {
                struct dirent *dp = readdir(dir);
                if (dp == nullptr) {
                    break;
                }

                if (dp->d_type == DT_DIR) {
                    continue;
                }

                const char *fileName = dp->d_name;
                const char *fileExt  = strrchr(fileName, '.');

                if (!fileExt || !MatchExtension(fileExt, matchExtensions)) {
                    continue;
                }

                fileList.emplace_back(fileName);
            }

            closedir(dir);
        }
#else
        std::string pathStr = folder;
        pathStr += "*";

        WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFile( pathStr.c_str(), &FindFileData );
        if ( hFind != INVALID_HANDLE_VALUE )
        {
            do
            {
                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    std::string fileName = FindFileData.cFileName;
                    std::string::size_type extPos = fileName.rfind('.');
                    if (extPos != std::string::npos &&
                        MatchExtension(fileName.substr(extPos + 1).c_str(), matchExtensions)) {
                        fileList.push_back(fileName);
                    }
                }
            } while ( FindNextFile( hFind, &FindFileData ) );

            FindClose( hFind );
        }
#endif
        return fileList;
    }

    bool CreatePath(const char *path)
    {
#if defined( __unix__ ) || defined( __APPLE__ )
        Gltf::StringUtils::PathSeparator separator = Gltf::StringUtils::PATH_UNIX;
#else
        Gltf::StringUtils::PathSeparator separator = Gltf::StringUtils::PATH_WIN;
#endif
        std::string folder = Gltf::StringUtils::GetFolderString(path);
        std::string clean = Gltf::StringUtils::GetCleanPathString(folder, separator);
        std::string build = clean;
        for (int i = 0; i < clean.length(); i ++) {
            if (clean[i] == separator && i > 0) {
                build[i] = '\0';
                if (i > 1 || build[1] != ':') {
                    if (_mkdir(build.c_str()) != 0 && errno != EEXIST) {
                        return false;
                    }
                }
            }
            build[i] = clean[i];
        }
        return true;
    }

    bool CopyFile(const std::string &srcFilename, const std::string &dstFilename) {
        std::ifstream srcFile(srcFilename, std::ios::binary);
        if (!srcFile) {
            fmt::printf("Warning: Couldn't open file %s for reading.\n", srcFilename);
            return false;
        }
        // find source file length
        srcFile.seekg(0, std::ios::end);
        std::streamsize srcSize = srcFile.tellg();
        srcFile.seekg(0, std::ios::beg);

        std::ofstream dstFile(dstFilename, std::ios::binary | std::ios::trunc);
        if (!dstFile) {
            fmt::printf("Warning: Couldn't open file %s for writing.\n", srcFilename);
            return false;
        }
        dstFile << srcFile.rdbuf();
        std::streamsize dstSize = dstFile.tellp();
        if (srcSize == dstSize) {
            return true;
        }
        fmt::printf("Warning: Only copied %lu bytes to %s, when %s is %lu bytes long.\n", dstSize, dstFilename, srcFilename, srcSize);
        return false;
    }
}
