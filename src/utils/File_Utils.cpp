/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "File_Utils.hpp"
#include "String_Utils.hpp"

#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <fmt/printf.h>
#include <stdio.h>

#include "String_Utils.hpp"

std::vector<std::string> FileUtils::ListFolderFiles(
    std::string folder,
    const std::set<std::string>& matchExtensions) {
  std::vector<std::string> fileList;
  if (folder.empty()) {
    folder = ".";
  }
  for (const auto& entry : std::filesystem::directory_iterator(folder)) {
    const auto& suffix = FileUtils::GetFileSuffix(entry.path().string());
    if (suffix.has_value()) {
      const auto& suffix_str = StringUtils::ToLower(suffix.value());
      if (matchExtensions.find(suffix_str) != matchExtensions.end()) {
        fileList.push_back(entry.path().filename().string());
      }
    }
  }
  return fileList;
}

bool FileUtils::CreatePath(const std::string path) {
  const auto& parent = std::filesystem::path(path).parent_path();
  if (parent.empty()) {
    return true;
  }
  if (std::filesystem::exists(parent)) {
    return std::filesystem::is_directory(parent);
  }
  return std::filesystem::create_directory(parent);
}

bool FileUtils::CopyFile(const std::string& srcFilename, const std::string& dstFilename, bool createPath) {
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
