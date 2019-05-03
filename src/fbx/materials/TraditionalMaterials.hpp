/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FbxMaterials.hpp"

struct FbxTraditionalMaterialInfo : FbxMaterialInfo {
  static constexpr const char* FBX_SHADER_LAMBERT = "Lambert";
  static constexpr const char* FBX_SHADER_BLINN = "Blinn";
  static constexpr const char* FBX_SHADER_PHONG = "Phong";

  FbxTraditionalMaterialInfo(
      const FbxUInt64 id,
      const FbxString& name,
      const FbxString& shadingModel)
      : FbxMaterialInfo(id, name, shadingModel) {}

  FbxFileTexture* texAmbient{};
  FbxVector4 colAmbient{};
  FbxFileTexture* texSpecular{};
  FbxVector4 colSpecular{};
  FbxFileTexture* texDiffuse{};
  FbxVector4 colDiffuse{};
  FbxFileTexture* texEmissive{};
  FbxVector4 colEmissive{};
  FbxFileTexture* texNormal{};
  FbxFileTexture* texShininess{};
  FbxDouble shininess{};
};

class FbxTraditionalMaterialResolver : FbxMaterialResolver<FbxTraditionalMaterialInfo> {
 public:
  FbxTraditionalMaterialResolver(
      FbxSurfaceMaterial* fbxMaterial,
      const std::map<const FbxTexture*, FbxString>& textureLocations)
      : FbxMaterialResolver(fbxMaterial, textureLocations) {}

  virtual std::unique_ptr<FbxTraditionalMaterialInfo> resolve() const;
};
