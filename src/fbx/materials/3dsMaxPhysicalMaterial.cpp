/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "RoughnessMetallicMaterials.hpp"

std::unique_ptr<FbxRoughMetMaterialInfo> Fbx3dsMaxPhysicalMaterialResolver::resolve() const {
  const FbxProperty topProp = fbxMaterial->FindProperty("3dsMax", false);
  if (topProp.GetPropertyDataType() != FbxCompoundDT) {
    return nullptr;
  }
  const FbxProperty props = topProp.Find("Parameters", false);
  if (!props.IsValid()) {
    return nullptr;
  }

  FbxString shadingModel = fbxMaterial->ShadingModel.Get();
  if (!shadingModel.IsEmpty() && shadingModel != "unknown") {
    ::fmt::printf(
        "Warning: Material %s has surprising shading model: %s\n",
        fbxMaterial->GetName(),
        shadingModel);
  }

  auto getTex = [&](std::string propName) -> const FbxFileTexture* {
    const FbxFileTexture* ptr = nullptr;
    const FbxProperty texProp = props.Find((propName + "_map").c_str(), false);
    if (texProp.IsValid()) {
      const FbxProperty useProp = props.Find((propName + "_map_on").c_str(), false);
      if (useProp.IsValid() && !useProp.Get<FbxBool>()) {
        // skip this texture if the _on property exists *and* is explicitly false
        return nullptr;
      }
      ptr = texProp.GetSrcObject<FbxFileTexture>();
      if (ptr != nullptr && textureLocations.find(ptr) == textureLocations.end()) {
        ptr = nullptr;
      }
    }
    return ptr;
  };

  FbxDouble baseWeight = getValue(props, "base_weight", 1.0);
  const auto* baseWeightMap = getTex("base_weight");
  FbxDouble4 baseCol = getValue(props, "base_color", FbxDouble4(0.5, 0.5, 0.5, 1.0));
  const auto* baseTex = getTex("base_color");

  double emissiveWeight = getValue(props, "emission", 0.0);
  const auto* emissiveWeightMap = getTex("emission");
  FbxDouble4 emissiveColor = getValue(props, "emit_color", FbxDouble4(1, 1, 1, 1));
  const auto* emissiveColorMap = getTex("emit_color");

  double roughness = getValue(props, "roughness", 0.0);
  const auto* roughnessMap = getTex("roughness");
  double metalness = getValue(props, "metalness", 0.0);
  const auto* metalnessMap = getTex("metalness");

  // TODO: we need this to affect roughness map, too.
  bool invertRoughness = getValue(props, "inv_roughness", false);
  if (invertRoughness) {
    roughness = 1.0f - roughness;
  }

  std::string unsupported;
  const auto addUnsupported = [&](const std::string bit) {
    if (!unsupported.empty()) {
      unsupported += ", ";
    }
    unsupported += bit;
  };

  // TODO: turn this into a normal map through simple numerial differentiation
  const auto* bumpMap = getTex("bump");
  if (bumpMap != nullptr) {
    addUnsupported("bump map");
  }

  // TODO: bake transparency > 0.0f into the alpha of baseColor?
  double transparency = getValue(props, "transparency", 0.0);
  const auto* transparencyMap = getTex("transparency");
  if (transparency != 0.0 || transparencyMap != nullptr) {
    addUnsupported("transparency");
  }

  // TODO: if/when we bake transparency, we'll need this
  // double transparencyDepth = getValue(props, "trans_depth", 0.0);
  // if (transparencyDepth != 0.0) {
  //   addUnsupported("transparency depth");
  // }
  // double transparencyColor = getValue(props, "trans_color", 0.0);
  // const auto* transparencyColorMap = getTex("trans_color");
  // if (transparencyColor != 0.0 || transparencyColorMap != nullptr) {
  //   addUnsupported("transparency color");
  // }
  // double thinWalledTransparency = getValue(props, "thin_walled", false);
  // if (thinWalledTransparency) {
  //   addUnsupported("thin-walled transparency");
  // }

  const auto* displacementMap = getTex("displacement");
  if (displacementMap != nullptr) {
    addUnsupported("displacement");
  }

  double reflectivityWeight = getValue(props, "reflectivity", 1.0);
  const auto* reflectivityWeightMap = getTex("reflectivity");
  FbxDouble4 reflectivityColor = getValue(props, "refl_color", FbxDouble4(1, 1, 1, 1));
  const auto* reflectivityColorMap = getTex("refl_color");
  if (reflectivityWeight != 1.0 || reflectivityWeightMap != nullptr ||
      reflectivityColor != FbxDouble4(1, 1, 1, 1) || reflectivityColorMap != nullptr) {
    addUnsupported("reflectivity");
  }

  double scattering = getValue(props, "scattering", 0.0);
  const auto* scatteringMap = getTex("scattering");
  if (scattering != 0.0 || scatteringMap != nullptr) {
    addUnsupported("sub-surface scattering");
  }

  double coating = getValue(props, "coating", 0.0);
  if (coating != 0.0) {
    addUnsupported("coating");
  }

  double diffuseRoughness = getValue(props, "diff_roughness", 0.0);
  if (diffuseRoughness != 0.0) {
    addUnsupported("diffuse roughness");
  }

  bool isBrdfMode = getValue(props, "brdf_mode", false);
  if (isBrdfMode) {
    addUnsupported("advanced reflectance custom curve");
  }

  double anisotropy = getValue(props, "anisotropy", 1.0);
  if (anisotropy != 1.0) {
    addUnsupported("anisotropy");
  }

  if (verboseOutput && !unsupported.empty()) {
    fmt::printf(
        "Warning: 3dsMax Physical Material %s uses features glTF cannot express:\n  %s\n",
        fbxMaterial->GetName(),
        unsupported);
  }

  std::unique_ptr<FbxRoughMetMaterialInfo> res(new FbxRoughMetMaterialInfo(
      fbxMaterial->GetName(),
      FbxRoughMetMaterialInfo::FBX_SHADER_METROUGH,
      baseCol,
      metalness,
      roughness));
  res->texBaseColor = baseTex;
  res->baseWeight = baseWeight;
  res->texBaseWeight = baseWeightMap;

  res->texMetallic = metalnessMap;
  res->texRoughness = roughnessMap;
  res->invertRoughnessMap = invertRoughness;

  res->emissive = emissiveColor;
  res->emissiveIntensity = emissiveWeight;
  res->texEmissive = emissiveColorMap;
  res->texEmissiveWeight = emissiveWeightMap;

  return res;
}
