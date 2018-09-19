/**
* Copyright (c) 2014-present, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*/

#pragma once

#include <functional>

#include "FBX2glTF.h"

#include <gltf/properties/ImageData.hpp>

using pixel = std::array<float, 4>; // pixel components are floats in [0, 1]
using pixel_merger = std::function<pixel(const std::vector<const pixel *>)>;

class TextureBuilder
{
public:
    TextureBuilder(const RawModel &raw, GltfModel &gltf)
        : raw(raw)
        , gltf(gltf)
    {}
    ~TextureBuilder() {}

    std::shared_ptr<TextureData> combine(
        const std::vector<int> &ixVec,
        const std::string &tag,
        const pixel_merger &mergeFunction
    );

    std::shared_ptr<TextureData> simple(int rawTexIndex, const std::string &tag);

    static std::string texIndicesKey(const std::vector<int> &ixVec, const std::string &tag) {
        std::string result = tag;
        for (int ix : ixVec) {
            result += "_" + std::to_string(ix);
        }
        return result;
    };

    static std::string describeChannel(int channels) {
        switch(channels) {
        case 1: return "G";
        case 2: return "GA";
        case 3: return "RGB";
        case 4: return "RGBA";
        default:
            return fmt::format("?%d?", channels);
        }
    };

    static void WriteToVectorContext(void *context, void *data, int size) {
        auto *vec = static_cast<std::vector<char> *>(context);
        for (int ii = 0; ii < size; ii ++) {
            vec->push_back(((char *) data)[ii]);
        }
    }

private:
    const RawModel &raw;
    GltfModel &gltf;
    std::map<std::string, std::shared_ptr<TextureData>> textureByIndicesKey;
};
