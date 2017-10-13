/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef FBX2GLTF_NODEDATA_H
#define FBX2GLTF_NODEDATA_H

#include "Raw2Gltf.h"

struct NodeData : Holdable
{
    NodeData(std::string name, const Vec3f &translation, const Quatf &rotation, const Vec3f &scale, bool isJoint);

    void AddChildNode(uint32_t childIx);
    void SetMesh(uint32_t meshIx);
    void SetSkin(uint32_t skinIx);
    void AddCamera(std::string camera);

    json serialize() const override;

    const std::string        name;
    const bool               isJoint;
    Vec3f                    translation;
    Quatf                    rotation;
    Vec3f                    scale;
    std::vector<uint32_t>    children;
    int32_t                  mesh;
    std::string              cameraName;
    int32_t                  skin;
    std::vector<std::string> skeletons;
};

#endif //FBX2GLTF_NODEDATA_H
