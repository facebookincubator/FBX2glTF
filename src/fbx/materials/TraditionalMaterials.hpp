/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "FbxMaterials.hpp"

struct FbxTraditionalMaterialInfo : FbxMaterialInfo {
    static constexpr const char *FBX_SHADER_LAMBERT = "Lambert";
    static constexpr const char *FBX_SHADER_BLINN   = "Blinn";
    static constexpr const char *FBX_SHADER_PHONG   = "Phong";

    FbxTraditionalMaterialInfo(const FbxString &name, const FbxString &shadingModel)
        : FbxMaterialInfo(name, shadingModel)
    {}

    FbxFileTexture *texAmbient {};
    FbxVector4     colAmbient {};
    FbxFileTexture *texSpecular {};
    FbxVector4     colSpecular {};
    FbxFileTexture *texDiffuse {};
    FbxVector4     colDiffuse {};
    FbxFileTexture *texEmissive {};
    FbxVector4     colEmissive {};
    FbxFileTexture *texNormal {};
    FbxFileTexture *texShininess {};
    FbxDouble      shininess {};
};

class FbxTraditionalMaterialResolver : FbxMaterialResolver<FbxTraditionalMaterialInfo> {
public:
    FbxTraditionalMaterialResolver(
        FbxSurfaceMaterial *fbxMaterial,
        const std::map<const FbxTexture *, FbxString> &textureLocations)
        : FbxMaterialResolver(fbxMaterial, textureLocations)
    {}

    virtual std::unique_ptr<FbxTraditionalMaterialInfo> resolve() const;
};
