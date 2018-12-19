/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct NodeData : Holdable
{
    NodeData(std::string name, const Vec3f &translation, const Quatf &rotation, const Vec3f &scale, bool isJoint);

    void AddChildNode(uint32_t childIx);
    void SetMesh(uint32_t meshIx);
    void SetSkin(uint32_t skinIx);
    void SetCamera(uint32_t camera);
    void SetLight(uint32_t light);

    json serialize() const override;

    const std::string        name;
    const bool               isJoint;
    Vec3f                    translation;
    Quatf                    rotation;
    Vec3f                    scale;
    std::vector<uint32_t>    children;
    int32_t                  mesh;
    int32_t                  camera;
    int32_t                  light;
    int32_t                  skin;
    std::vector<std::string> skeletons;
    std::vector<std::string> userProperties;
};
