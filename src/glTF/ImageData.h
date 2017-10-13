/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef FBX2GLTF_IMAGEDATA_H
#define FBX2GLTF_IMAGEDATA_H

#include "Raw2Gltf.h"

struct ImageData : Holdable
{
    ImageData(std::string name, std::string uri);
    ImageData(std::string name, const BufferViewData &bufferView, std::string mimeType);

    json serialize() const override;

    const std::string name;
    const std::string uri; // non-empty in gltf mode
    const int32_t     bufferView; // non-negative in glb mode
    const std::string mimeType;
};

#endif //FBX2GLTF_IMAGEDATA_H
