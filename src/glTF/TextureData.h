/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef FBX2GLTF_TEXTUREDATA_H
#define FBX2GLTF_TEXTUREDATA_H

#include "Raw2Gltf.h"

struct TextureData : Holdable
{
    TextureData(std::string name, const SamplerData &sampler, const ImageData &source);

    json serialize() const override;

    const std::string name;
    const uint32_t    sampler;
    const uint32_t    source;
};

#endif //FBX2GLTF_TEXTUREDATA_H
