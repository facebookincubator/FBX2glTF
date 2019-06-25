/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct SkinData : Holdable {
  SkinData(
      const std::vector<uint32_t> joints,
      const AccessorData& inverseBindMatricesAccessor,
      const NodeData& skeletonRootNode);

  json serialize() const override;

  const std::vector<uint32_t> joints;
  const uint32_t skeletonRootNode;
  const uint32_t inverseBindMatrices;
};
