/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MeshData.hpp"
#include "PrimitiveData.hpp"

MeshData::MeshData(const std::string& name, const std::vector<float>& weights)
    : Holdable(), name(name), weights(weights) {}

json MeshData::serialize() const {
  json jsonPrimitivesArray = json::array();
  json targetNames = json::array();
  for (const auto& primitive : primitives) {
    jsonPrimitivesArray.push_back(*primitive);
    if (!primitive->targetNames.empty()) {
      for (auto targetName : primitive->targetNames) {
        targetNames.push_back(targetName);
      }
    }
  }
  json result = {{"name", name}, {"primitives", jsonPrimitivesArray}};
  if (!weights.empty()) {
    result["weights"] = weights;
  }
  if (!targetNames.empty()) {
    result["extras"]["targetNames"] = targetNames;
  }
  return result;
}
