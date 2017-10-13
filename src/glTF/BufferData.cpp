/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <cppcodec/base64_default_rfc4648.hpp>

#include "BufferData.h"

BufferData::BufferData(const std::shared_ptr<const std::vector<uint8_t> > &binData)
    : Holdable(),
      isGlb(true),
      binData(binData)
{
}

BufferData::BufferData(std::string uri, const std::shared_ptr<const std::vector<uint8_t> > &binData, bool isEmbedded)
    : Holdable(),
      isGlb(false),
      uri(isEmbedded ? "" : std::move(uri)),
      binData(binData)
{
}

json BufferData::serialize() const
{
    json result{
        {"byteLength", binData->size()}
    };
    if (!isGlb) {
        if (!uri.empty()) {
            result["uri"] = uri;
        } else {
            std::string encoded = base64::encode(*binData);
            result["uri"] = "data:application/octet-stream;base64," + encoded;
        }
    }
    return result;
}
