/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>
#include <vector>

#include "FBX2glTF.h"

class FbxMaterialInfo {
 public:
  FbxMaterialInfo(const FbxUInt64 id, const FbxString& name, const FbxString& shadingModel)
      : id(id), name(name), shadingModel(shadingModel) {}

  const FbxUInt64 id;
  const FbxString name;
  const FbxString shadingModel;
};

template <class T>
class FbxMaterialResolver {
 public:
  FbxMaterialResolver(
      FbxSurfaceMaterial* fbxMaterial,
      const std::map<const FbxTexture*, FbxString>& textureLocations)
      : fbxMaterial(fbxMaterial), textureLocations(textureLocations) {}
  virtual std::unique_ptr<T> resolve() const = 0;

 protected:
  const FbxSurfaceMaterial* fbxMaterial;
  const std::map<const FbxTexture*, FbxString> textureLocations;
};

class FbxMaterialsAccess {
 public:
  FbxMaterialsAccess(
      const FbxMesh* pMesh,
      const std::map<const FbxTexture*, FbxString>& textureLocations);

  const std::shared_ptr<FbxMaterialInfo> GetMaterial(const int polygonIndex) const;

  const std::vector<std::string> GetUserProperties(const int polygonIndex) const;

  std::unique_ptr<FbxMaterialInfo> GetMaterialInfo(
      FbxSurfaceMaterial* material,
      const std::map<const FbxTexture*, FbxString>& textureLocations);

 private:
  FbxGeometryElement::EMappingMode mappingMode;
  std::vector<std::shared_ptr<FbxMaterialInfo>> summaries{};
  std::vector<std::vector<std::string>> userProperties;
  const FbxMesh* mesh;
  const FbxLayerElementArrayTemplate<int>* indices;
};
