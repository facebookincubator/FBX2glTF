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
  json jsonTargetNamesArray = json::array();
  for (const auto& primitive : primitives) {
    jsonPrimitivesArray.push_back(*primitive);
    if (!primitive->targetNames.empty() && jsonTargetNamesArray.empty()) {
      for (auto targetName : primitive->targetNames) {
        jsonTargetNamesArray.push_back(targetName);
      }
    }
  }
  json result = {{"name", name}, {"primitives", jsonPrimitivesArray}};
  if (!weights.empty()) {
    result["weights"] = weights;
  }
  if (!jsonTargetNamesArray.empty()) {
    result["extras"]["targetNames"] = jsonTargetNamesArray;
  }
  return result;
}
