/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef FBX2GLTF_SKINDATA_H
#define FBX2GLTF_SKINDATA_H

#include "Raw2Gltf.h"

struct SkinData : Holdable
{
    SkinData(
        const std::vector<uint32_t> joints, const AccessorData &inverseBindMatricesAccessor,
        const NodeData &skeletonRootNode);

    json serialize() const override;

    const std::vector<uint32_t> joints;
    const uint32_t              skeletonRootNode;
    const uint32_t              inverseBindMatrices;
};

#endif //FBX2GLTF_SKINDATA_H
