/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

 #include <tobiaslocker/base64.hpp>
 #include <string>

#include "BufferData.hpp"

BufferData::BufferData(const std::shared_ptr<const std::vector<uint8_t>>& binData)
    : Holdable(), isGlb(true), binData(binData) {}

BufferData::BufferData(
    std::string uri,
    const std::shared_ptr<const std::vector<uint8_t>>& binData,
    bool isEmbedded)
    : Holdable(), isGlb(false), uri(isEmbedded ? "" : std::move(uri)), binData(binData) {}

json BufferData::serialize() const {
  json result{{"byteLength", binData->size()}};
  if (!isGlb) {
    if (!uri.empty()) {
      result["uri"] = uri;
    } else if (binData && !binData->empty()) {
        const auto &data = *binData;
        std::string raw(reinterpret_cast<const char *>(data.data()), data.size());
        std::string b64 = base64::to_base64(raw);
        result["uri"] = "data:application/octet-stream;base64," + b64;
    } else {
        result["uri"] = std::string();
    }
  }
  return result;
}
