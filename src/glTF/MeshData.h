/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef FBX2GLTF_MESHDATA_H
#define FBX2GLTF_MESHDATA_H

#include <string>

#include <draco/compression/encode.h>

#include "Raw2Gltf.h"

#include "PrimitiveData.h"

struct MeshData : Holdable
{
    MeshData(const std::string &name, const std::vector<float> &weights);

    void AddPrimitive(std::shared_ptr<PrimitiveData> primitive)
    {
        primitives.push_back(std::move(primitive));
    }

    json serialize() const override;

    const std::string                           name;
    const std::vector<float>                    weights;
    std::vector<std::shared_ptr<PrimitiveData>> primitives;
};

#endif //FBX2GLTF_MESHDATA_H
