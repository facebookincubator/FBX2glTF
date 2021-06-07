/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include "FbxMaterials.hpp"

struct FbxRoughMetMaterialInfo : FbxMaterialInfo {
  static constexpr const char* FBX_SHADER_METROUGH = "MetallicRoughness";

  static std::unique_ptr<FbxRoughMetMaterialInfo> From(
      FbxSurfaceMaterial* fbxMaterial,
      const std::map<const FbxTexture*, FbxString>& textureLocations);

  FbxRoughMetMaterialInfo(
      const FbxUInt64 id,
      const FbxString& name,
      const FbxString& shadingModel,
      FbxDouble4 baseColor,
      FbxDouble metallic,
      FbxDouble roughness)
      : FbxMaterialInfo(id, name, shadingModel),
        baseColor(baseColor),
        metallic(metallic),
        roughness(roughness) {}

  FbxVector4 baseColor;
  FbxDouble metallic;
  FbxDouble roughness;

  FbxBool invertRoughnessMap = false;
  FbxDouble baseWeight = 1;
  FbxVector4 emissive = FbxVector4(0, 0, 0, 1);
  FbxDouble emissiveIntensity = 1;

  const FbxFileTexture* texNormal = nullptr;
  const FbxFileTexture* texBaseColor = nullptr;
  const FbxFileTexture* texBaseWeight = nullptr;
  const FbxFileTexture* texMetallic = nullptr;
  const FbxFileTexture* texRoughness = nullptr;
  const FbxFileTexture* texEmissive = nullptr;
  const FbxFileTexture* texEmissiveWeight = nullptr;
  const FbxFileTexture* texAmbientOcclusion = nullptr;
};

class FbxStingrayPBSMaterialResolver : FbxMaterialResolver<FbxRoughMetMaterialInfo> {
 public:
  FbxStingrayPBSMaterialResolver(
      FbxSurfaceMaterial* fbxMaterial,
      const std::map<const FbxTexture*, FbxString>& textureLocations)
      : FbxMaterialResolver(fbxMaterial, textureLocations) {}

  virtual std::unique_ptr<FbxRoughMetMaterialInfo> resolve() const;
};

class ArnoldStandardMaterialResolver : FbxMaterialResolver<FbxRoughMetMaterialInfo> {
 public:
  ArnoldStandardMaterialResolver(
      FbxSurfaceMaterial* fbxMaterial,
      const std::map<const FbxTexture*, FbxString>& textureLocations)
      : FbxMaterialResolver(fbxMaterial, textureLocations) {}

  virtual std::unique_ptr<FbxRoughMetMaterialInfo> resolve() const;
};

class Fbx3dsMaxPhysicalMaterialResolver : FbxMaterialResolver<FbxRoughMetMaterialInfo> {
 public:
  Fbx3dsMaxPhysicalMaterialResolver(
      FbxSurfaceMaterial* fbxMaterial,
      const std::map<const FbxTexture*, FbxString>& textureLocations)
      : FbxMaterialResolver(fbxMaterial, textureLocations) {}

  virtual std::unique_ptr<FbxRoughMetMaterialInfo> resolve() const;

 private:
  template <typename T>
  T getValue(const FbxProperty& props, std::string propName, const T& def) const {
    const FbxProperty prop = props.FindHierarchical(propName.c_str());
    return prop.IsValid() ? prop.Get<T>() : def;
  }
};
