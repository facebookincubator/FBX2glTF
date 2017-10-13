/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "ImageData.h"

#include <utility>

#include "BufferViewData.h"

ImageData::ImageData(std::string name, std::string uri)
    : Holdable(),
      name(std::move(name)),
      uri(std::move(uri)),
      bufferView(-1)
{
}

ImageData::ImageData(std::string name, const BufferViewData &bufferView, std::string mimeType)
    : Holdable(),
      name(std::move(name)),
      bufferView(bufferView.ix),
      mimeType(std::move(mimeType))
{
}

json ImageData::serialize() const
{
    if (mimeType.empty()) {
        return {
            { "uri", uri }
        };
    }
    return {
        { "name", name },
        { "bufferView", bufferView },
        { "mimeType", mimeType }
    };
}
