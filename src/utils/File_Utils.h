/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

namespace FileUtils {
    std::string GetCurrentFolder();

    bool FolderExists(const std::string &folderPath);

    bool MatchExtension(const char *fileExtension, const char *matchExtensions);
    std::vector<std::string> ListFolderFiles(const char *folder, const char *matchExtensions);

    bool CreatePath(const char *path);

    bool CopyFile(const std::string &srcFilename, const std::string &dstFilename);
}

#endif // !__FILE_UTILS_H__
