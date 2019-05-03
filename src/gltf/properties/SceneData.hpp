/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct SceneData : Holdable {
  SceneData(std::string name, const NodeData& rootNode);

  json serialize() const override;

  const std::string name;
  std::vector<uint32_t> nodes;
};
