/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <filesystem>

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace FileUtils {

std::string GetCurrentFolder();

bool FileExists(const std::string& folderPath);
bool FolderExists(const std::string& folderPath);

std::vector<std::string> ListFolderFiles(
    const std::string folder,
    const std::set<std::string>& matchExtensions);

bool CreatePath(std::string path);

bool CopyFile(
    const std::string& srcFilename,
    const std::string& dstFilename,
    bool createPath = false);

inline std::string GetCurrentFolder() {
  return std::filesystem::current_path().string() + "/";
}

inline bool FileExists(const std::string& filePath) {
  return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

inline bool FolderExists(const std::string& folderPath) {
  return std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath);
}

inline std::string getFolder(const std::string& path) {
  return std::filesystem::path(path).parent_path().string();
}

inline std::string GetFileName(const std::string& path) {
  return std::filesystem::path(path).filename().string();
}

inline std::string GetFileBase(const std::string& path) {
  return std::filesystem::path(path).stem().string();
}

inline std::optional<std::string> GetFileSuffix(const std::string& path) {
  const auto& extension = std::filesystem::path(path).extension();
  if (extension.empty()) {
    return std::nullopt;
  }
  return extension.string().substr(1);
}

} // namespace FileUtils
