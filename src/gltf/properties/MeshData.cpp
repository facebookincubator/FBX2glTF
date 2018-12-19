/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "MeshData.hpp"
#include "PrimitiveData.hpp"

MeshData::MeshData(const std::string& name, const std::vector<float>& weights)
    : Holdable(), name(name), weights(weights) {}

json MeshData::serialize() const {
  json jsonPrimitivesArray = json::array();
  for (const auto& primitive : primitives) {
    jsonPrimitivesArray.push_back(*primitive);
  }
  json result = {{"name", name}, {"primitives", jsonPrimitivesArray}};
  if (!weights.empty()) {
    result["weights"] = weights;
  }
  return result;
}
