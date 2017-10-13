/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "SkinData.h"

#include "AccessorData.h"
#include "NodeData.h"

SkinData::SkinData(
    const std::vector<uint32_t> joints, const AccessorData &inverseBindMatricesAccessor,
    const NodeData &skeletonRootNode)
    : Holdable(),
      joints(joints),
      inverseBindMatrices(inverseBindMatricesAccessor.ix),
      skeletonRootNode(skeletonRootNode.ix)
{
}

json SkinData::serialize() const
{
    return {
        { "joints", joints },
        { "inverseBindMatrices", inverseBindMatrices },
        { "skeleton", skeletonRootNode }
    };
}
