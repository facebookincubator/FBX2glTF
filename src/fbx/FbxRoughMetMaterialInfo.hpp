/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <fstream>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "FbxMaterialInfo.hpp"

struct FbxRoughMetMaterialInfo : FbxMaterialInfo {
    static constexpr const char *FBX_SHADER_METROUGH = "MetallicRoughness";

    static std::unique_ptr<FbxRoughMetMaterialInfo> From(
        FbxSurfaceMaterial *fbxMaterial,
        const std::map<const FbxTexture *, FbxString> &textureLocations);

    FbxRoughMetMaterialInfo(const FbxString &name, const FbxString &shadingModel)
        : FbxMaterialInfo(name, shadingModel)
    {}

    const FbxFileTexture *texColor {};
    FbxVector4           colBase {};
    const FbxFileTexture *texNormal {};
    const FbxFileTexture *texMetallic {};
    FbxDouble            metallic {};
    const FbxFileTexture *texRoughness {};
    FbxDouble            roughness {};
    const FbxFileTexture *texEmissive {};
    FbxVector4           colEmissive {};
    FbxDouble            emissiveIntensity {};
    const FbxFileTexture *texAmbientOcclusion {};
};
