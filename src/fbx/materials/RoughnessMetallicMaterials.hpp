/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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
      const FbxString& name,
      const FbxString& shadingModel,
      FbxDouble4 baseColor,
      FbxDouble metallic,
      FbxDouble roughness)
      : FbxMaterialInfo(name, shadingModel),
        baseColor(baseColor),
        metallic(metallic),
        roughness(roughness) {}

  const FbxVector4 baseColor;
  const FbxDouble metallic;
  const FbxDouble roughness;

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

class Fbx3dsMaxPhysicalMaterialResolver : FbxMaterialResolver<FbxRoughMetMaterialInfo> {
 public:
  Fbx3dsMaxPhysicalMaterialResolver(
      FbxSurfaceMaterial* fbxMaterial,
      const std::map<const FbxTexture*, FbxString>& textureLocations)
      : FbxMaterialResolver(fbxMaterial, textureLocations) {}

  virtual std::unique_ptr<FbxRoughMetMaterialInfo> resolve() const;

 private:
  template <typename T>
  T getValue(const FbxProperty& props, std::string propName, const T& default) const {
    const FbxProperty prop = props.FindHierarchical(propName.c_str());
    return prop.IsValid() ? prop.Get<T>() : default;
  }
};
