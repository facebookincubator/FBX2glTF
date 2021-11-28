/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SkinData.hpp"

#include "AccessorData.hpp"
#include "NodeData.hpp"

SkinData::SkinData(
    const std::vector<uint32_t> joints,
    const AccessorData& inverseBindMatricesAccessor,
    const NodeData& skeletonRootNode)
    : Holdable(),
      joints(joints),
      inverseBindMatrices(inverseBindMatricesAccessor.ix),
      skeletonRootNode(skeletonRootNode.ix) {}

json SkinData::serialize() const {
  return {
      {"joints", joints},
      {"inverseBindMatrices", inverseBindMatrices},
      {"skeleton", skeletonRootNode}};
}
