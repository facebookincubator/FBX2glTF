/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "RoughnessMetallicMaterials.hpp"

std::unique_ptr<FbxRoughMetMaterialInfo> FbxStingrayPBSMaterialResolver::resolve() const {
  const FbxProperty mayaProp = fbxMaterial->FindProperty("Maya");
  if (mayaProp.GetPropertyDataType() != FbxCompoundDT) {
    return nullptr;
  }
  if (!fbxMaterial->ShadingModel.Get().IsEmpty()) {
    ::fmt::printf(
        "Warning: Material %s has surprising shading model: %s\n",
        fbxMaterial->GetName(),
        fbxMaterial->ShadingModel.Get());
  }

  auto getTex = [&](std::string propName) {
    const FbxFileTexture* ptr = nullptr;

    const FbxProperty useProp = mayaProp.FindHierarchical(("use_" + propName + "_map").c_str());
    if (useProp.IsValid() && useProp.Get<bool>()) {
      const FbxProperty texProp = mayaProp.FindHierarchical(("TEX_" + propName + "_map").c_str());
      if (texProp.IsValid()) {
        ptr = texProp.GetSrcObject<FbxFileTexture>();
        if (ptr != nullptr && textureLocations.find(ptr) == textureLocations.end()) {
          ptr = nullptr;
        }
      }
    } else if (verboseOutput && useProp.IsValid()) {
      fmt::printf(
          "Note: Property '%s' of Stingray PBS material '%s' exists, but is flagged as 'do not use'.\n",
          propName,
          fbxMaterial->GetName());
    }
    return ptr;
  };

  auto getVec = [&](std::string propName) -> FbxDouble3 {
    const FbxProperty vecProp = mayaProp.FindHierarchical(propName.c_str());
    return vecProp.IsValid() ? vecProp.Get<FbxDouble3>() : FbxDouble3(1, 1, 1);
  };

  auto getVal = [&](std::string propName) -> FbxDouble {
    const FbxProperty vecProp = mayaProp.FindHierarchical(propName.c_str());
    return vecProp.IsValid() ? vecProp.Get<FbxDouble>() : 0;
  };

  FbxDouble3 baseColor = getVec("base_color");
  std::unique_ptr<FbxRoughMetMaterialInfo> res(new FbxRoughMetMaterialInfo(
      fbxMaterial->GetName(),
      FbxRoughMetMaterialInfo::FBX_SHADER_METROUGH,
      FbxDouble4(baseColor[0], baseColor[1], baseColor[2], 1),
      getVal("metallic"),
      getVal("roughness")));
  res->texNormal = getTex("normal");
  res->texBaseColor = getTex("color");
  res->texAmbientOcclusion = getTex("ao");
  res->texEmissive = getTex("emissive");
  res->emissive = getVec("emissive");
  res->emissiveIntensity = getVal("emissive_intensity");
  res->texMetallic = getTex("metallic");
  res->texRoughness = getTex("roughness");

  return res;
};
