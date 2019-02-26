/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "TraditionalMaterials.hpp"

std::unique_ptr<FbxTraditionalMaterialInfo> FbxTraditionalMaterialResolver::resolve() const {
  auto getSurfaceScalar = [&](const char* propName) -> std::tuple<FbxDouble, FbxFileTexture*> {
    const FbxProperty prop = fbxMaterial->FindProperty(propName);

    FbxDouble val(0);
    FbxFileTexture* tex = prop.GetSrcObject<FbxFileTexture>();
    if (tex != nullptr && textureLocations.find(tex) == textureLocations.end()) {
      tex = nullptr;
    }
    if (tex == nullptr && prop.IsValid()) {
      val = prop.Get<FbxDouble>();
    }
    return std::make_tuple(val, tex);
  };

  auto getSurfaceVector = [&](const char* propName) -> std::tuple<FbxDouble3, FbxFileTexture*> {
    const FbxProperty prop = fbxMaterial->FindProperty(propName);

    FbxDouble3 val(1, 1, 1);
    FbxFileTexture* tex = prop.GetSrcObject<FbxFileTexture>();
    if (tex != nullptr && textureLocations.find(tex) == textureLocations.end()) {
      tex = nullptr;
    }
    if (tex == nullptr && prop.IsValid()) {
      val = prop.Get<FbxDouble3>();
    }
    return std::make_tuple(val, tex);
  };

  auto getSurfaceValues =
      [&](const char* colName,
          const char* facName) -> std::tuple<FbxVector4, FbxFileTexture*, FbxFileTexture*> {
    const FbxProperty colProp = fbxMaterial->FindProperty(colName);
    const FbxProperty facProp = fbxMaterial->FindProperty(facName);

    FbxDouble3 colorVal(1, 1, 1);
    FbxDouble factorVal(1);

    FbxFileTexture* colTex = colProp.GetSrcObject<FbxFileTexture>();
    if (colTex != nullptr && textureLocations.find(colTex) == textureLocations.end()) {
      colTex = nullptr;
    }
    if (colTex == nullptr && colProp.IsValid()) {
      colorVal = colProp.Get<FbxDouble3>();
    }
    FbxFileTexture* facTex = facProp.GetSrcObject<FbxFileTexture>();
    if (facTex != nullptr && textureLocations.find(facTex) == textureLocations.end()) {
      facTex = nullptr;
    }
    if (facTex == nullptr && facProp.IsValid()) {
      factorVal = facProp.Get<FbxDouble>();
    }

    auto val = FbxVector4(
        colorVal[0] * factorVal, colorVal[1] * factorVal, colorVal[2] * factorVal, factorVal);
    return std::make_tuple(val, colTex, facTex);
  };

  std::string name = fbxMaterial->GetName();
  std::unique_ptr<FbxTraditionalMaterialInfo> res(new FbxTraditionalMaterialInfo(
      fbxMaterial->GetUniqueID(), name.c_str(), fbxMaterial->ShadingModel.Get()));

  // four properties are on the same structure and follow the same rules
  auto handleBasicProperty = [&](const char* colName,
                                 const char* facName) -> std::tuple<FbxVector4, FbxFileTexture*> {
    FbxFileTexture *colTex, *facTex;
    FbxVector4 vec;

    std::tie(vec, colTex, facTex) = getSurfaceValues(colName, facName);
    if (colTex) {
      if (facTex) {
        fmt::printf(
            "Warning: Mat [%s]: Can't handle both %s and %s textures; discarding %s.\n",
            name,
            colName,
            facName,
            facName);
      }
      return std::make_tuple(vec, colTex);
    }
    return std::make_tuple(vec, facTex);
  };

  std::tie(res->colAmbient, res->texAmbient) =
      handleBasicProperty(FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor);
  std::tie(res->colSpecular, res->texSpecular) =
      handleBasicProperty(FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor);
  std::tie(res->colDiffuse, res->texDiffuse) =
      handleBasicProperty(FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor);
  std::tie(res->colEmissive, res->texEmissive) =
      handleBasicProperty(FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor);

  // the normal map can only ever be a map, ignore everything else
  tie(std::ignore, res->texNormal) = getSurfaceVector(FbxSurfaceMaterial::sNormalMap);

  // shininess can be a map or a factor; afaict the map is always 'ShininessExponent' and the
  // value is always found in 'Shininess' but only sometimes in 'ShininessExponent'.
  tie(std::ignore, res->texShininess) = getSurfaceScalar("ShininessExponent");
  tie(res->shininess, std::ignore) = getSurfaceScalar("Shininess");

  // for transparency we just want a constant vector value;
  FbxVector4 transparency;
  // extract any existing textures only so we can warn that we're throwing them away
  FbxFileTexture *colTex, *facTex;
  std::tie(transparency, colTex, facTex) = getSurfaceValues(
      FbxSurfaceMaterial::sTransparentColor, FbxSurfaceMaterial::sTransparencyFactor);
  if (colTex) {
    fmt::printf(
        "Warning: Mat [%s]: Can't handle texture for %s; discarding.\n",
        name,
        FbxSurfaceMaterial::sTransparentColor);
  }
  if (facTex) {
    fmt::printf(
        "Warning: Mat [%s]: Can't handle texture for %s; discarding.\n",
        name,
        FbxSurfaceMaterial::sTransparencyFactor);
  }
  // FBX color is RGB, so we calculate the A channel as the average of the FBX transparency color
  // vector
  res->colDiffuse[3] = 1.0 - (transparency[0] + transparency[1] + transparency[2]) / 3.0;

  return res;
}
