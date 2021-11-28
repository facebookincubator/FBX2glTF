/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

#include "FBX2glTF.h"

#include <gltf/properties/ImageData.hpp>

#include "GltfModel.hpp"

class TextureBuilder {
 public:
  using pixel = std::array<float, 4>; // pixel components are floats in [0, 1]
  using pixel_merger = std::function<pixel(const std::vector<const pixel*>)>;

  TextureBuilder(
      const RawModel& raw,
      const GltfOptions& options,
      const std::string& outputFolder,
      GltfModel& gltf)
      : raw(raw), options(options), outputFolder(outputFolder), gltf(gltf) {
      if (!outputFolder.empty()) {
            if (outputFolder[outputFolder.size() - 1] == '/') {
                this->outputFolder = outputFolder.substr(0, outputFolder.size() - 1) ;
            }
      }
  }
  ~TextureBuilder() {}

  std::shared_ptr<TextureData> combine(
      const std::vector<int>& ixVec,
      const std::string& tag,
      const pixel_merger& mergeFunction,
      bool transparency);

  std::shared_ptr<TextureData> simple(int rawTexIndex, const std::string& tag);

  static std::string texIndicesKey(const std::vector<int>& ixVec, const std::string& tag) {
    std::string result = tag;
    for (int ix : ixVec) {
      result += "_" + std::to_string(ix);
    }
    return result;
  };

  static std::string describeChannel(int channels) {
    switch (channels) {
      case 1:
        return "G";
      case 2:
        return "GA";
      case 3:
        return "RGB";
      case 4:
        return "RGBA";
      default:
        return fmt::format("?%d?", channels);
    }
  };

  static void WriteToVectorContext(void* context, void* data, int size) {
    auto* vec = static_cast<std::vector<uint8_t>*>(context);
    for (int ii = 0; ii < size; ii++) {
      vec->push_back(((uint8_t*)data)[ii]);
    }
  }

 private:
  const RawModel& raw;
  const GltfOptions& options;
  std::string outputFolder;
  GltfModel& gltf;

  std::map<std::string, std::shared_ptr<TextureData>> textureByIndicesKey;
};
