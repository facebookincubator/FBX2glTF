/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "File_Utils.hpp"

#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <stdint.h>
#include <stdio.h>

#include "FBX2glTF.h"
#include "String_Utils.hpp"

namespace FileUtils {

std::vector<std::string> ListFolderFiles(
    const std::string folder,
    const std::set<std::string>& matchExtensions) {
  std::vector<std::string> fileList;
  for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folder)) {
    const std::filesystem::path& path = entry.path();
    if (matchExtensions.find(path.extension().string()) != matchExtensions.end()) {
      fileList.push_back(path.string());
    }
  }
  return fileList;
}

bool CreatePath(const std::string path) {
  const auto& parent = std::filesystem::path(path).parent_path();
  if (parent.empty()) {
    // this is either CWD or std::filesystem root; either way it exists
    return true;
  }
  if (std::filesystem::exists(parent)) {
    return std::filesystem::is_directory(parent);
  }
  return std::filesystem::create_directory(parent);
}

bool CopyFile(const std::string& srcFilename, const std::string& dstFilename, bool createPath) {
  std::ifstream srcFile(srcFilename, std::ios::binary);
  if (!srcFile) {
    fmt::printf("Warning: Couldn't open file %s for reading.\n", srcFilename);
    return false;
  }
  // find source file length
  srcFile.seekg(0, std::ios::end);
  std::streamsize srcSize = srcFile.tellg();
  srcFile.seekg(0, std::ios::beg);

  if (createPath && !CreatePath(dstFilename.c_str())) {
    fmt::printf("Warning: Couldn't create directory %s.\n", dstFilename);
    return false;
  }

  std::ofstream dstFile(dstFilename, std::ios::binary | std::ios::trunc);
  if (!dstFile) {
    fmt::printf("Warning: Couldn't open file %s for writing.\n", dstFilename);
    return false;
  }
  dstFile << srcFile.rdbuf();
  std::streamsize dstSize = dstFile.tellp();
  if (srcSize == dstSize) {
    return true;
  }
  fmt::printf(
      "Warning: Only copied %lu bytes to %s, when %s is %lu bytes long.\n",
      dstSize,
      dstFilename,
      srcFilename,
      srcSize);
  return false;
}
} // namespace FileUtils
