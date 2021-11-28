/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <set>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

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

inline std::string GetAbsolutePath(const std::string& filePath) {
  return boost::filesystem::absolute(filePath).string();
}

inline std::string GetCurrentFolder() {
  return boost::filesystem::current_path().string();
}

inline bool FileExists(const std::string& filePath) {
  return boost::filesystem::exists(filePath) && boost::filesystem::is_regular_file(filePath);
}

inline bool FolderExists(const std::string& folderPath) {
  return boost::filesystem::exists(folderPath) && boost::filesystem::is_directory(folderPath);
}

inline std::string getFolder(const std::string& path) {
  auto parent = boost::filesystem::path(path).parent_path().string();
  return parent == "" ? "." : parent;
}

inline std::string GetFileName(const std::string& path) {
  return boost::filesystem::path(path).filename().string();
}

inline std::string GetFileBase(const std::string& path) {
  return boost::filesystem::path(path).stem().string();
}

inline boost::optional<std::string> GetFileSuffix(const std::string& path) {
  const auto& extension = boost::filesystem::path(path).extension();
  if (extension.empty()) {
    return boost::none;
  }
  return extension.string().substr(1);
}

inline bool MakeDir(const std::string& path) {
  return boost::filesystem::create_directories(boost::filesystem::path(path));
}

} // namespace FileUtils
