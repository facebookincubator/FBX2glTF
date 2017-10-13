/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "MeshData.h"
#include "PrimitiveData.h"

MeshData::MeshData(std::string name)
    : Holdable(),
      name(std::move(name))
{
}

json MeshData::serialize() const
{
    json jsonPrimitivesArray = json::array();
    for (const auto &primitive : primitives) {
        jsonPrimitivesArray.push_back(*primitive);
    }
    return {
        { "name", name },
        { "primitives", jsonPrimitivesArray }
    };
}
