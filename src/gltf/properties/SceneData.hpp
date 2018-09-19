/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "gltf/Raw2Gltf.hpp"

struct SceneData : Holdable
{
    SceneData(std::string name, const NodeData &rootNode);

    json serialize() const override;

    const std::string     name;
    std::vector<uint32_t> nodes;
};
