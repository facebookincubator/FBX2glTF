/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "FbxRoughMetMaterialInfo.hpp"

std::unique_ptr<FbxRoughMetMaterialInfo> FbxRoughMetMaterialInfo::From(
    FbxSurfaceMaterial* fbxMaterial,
    const std::map<const FbxTexture*, FbxString>& textureLocations) {
  std::unique_ptr<FbxRoughMetMaterialInfo> res(
      new FbxRoughMetMaterialInfo(fbxMaterial->GetName(), FBX_SHADER_METROUGH));

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
          "Note: Property '%s' of material '%s' exists, but is flagged as 'do not use'.\n",
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

  res->texNormal = getTex("normal");
  res->texColor = getTex("color");
  res->colBase = getVec("base_color");
  res->texAmbientOcclusion = getTex("ao");
  res->texEmissive = getTex("emissive");
  res->colEmissive = getVec("emissive");
  res->emissiveIntensity = getVal("emissive_intensity");
  res->texMetallic = getTex("metallic");
  res->metallic = getVal("metallic");
  res->texRoughness = getTex("roughness");
  res->roughness = getVal("roughness");

  return res;
}
