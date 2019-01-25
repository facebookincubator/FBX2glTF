/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "fbx/Fbx2Raw.hpp"

#include "FbxMaterials.hpp"
#include "RoughnessMetallicMaterials.hpp"
#include "TraditionalMaterials.hpp"

FbxMaterialsAccess::FbxMaterialsAccess(
    const FbxMesh* pMesh,
    const std::map<const FbxTexture*, FbxString>& textureLocations)
    : mappingMode(FbxGeometryElement::eNone), mesh(nullptr), indices(nullptr) {
  if (pMesh->GetElementMaterialCount() <= 0) {
    return;
  }

  const FbxGeometryElement::EMappingMode materialMappingMode =
      pMesh->GetElementMaterial()->GetMappingMode();
  if (materialMappingMode != FbxGeometryElement::eByPolygon &&
      materialMappingMode != FbxGeometryElement::eAllSame) {
    return;
  }

  const FbxGeometryElement::EReferenceMode materialReferenceMode =
      pMesh->GetElementMaterial()->GetReferenceMode();
  if (materialReferenceMode != FbxGeometryElement::eIndexToDirect) {
    return;
  }

  mappingMode = materialMappingMode;
  mesh = pMesh;
  indices = &pMesh->GetElementMaterial()->GetIndexArray();

  for (int ii = 0; ii < indices->GetCount(); ii++) {
    int materialNum = indices->GetAt(ii);
    if (materialNum < 0) {
      continue;
    }

    FbxSurfaceMaterial* surfaceMaterial =
        mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(materialNum);

    if (materialNum >= summaries.size()) {
      summaries.resize(materialNum + 1);
    }
    auto summary = summaries[materialNum];
    if (summary == nullptr) {
      summary = summaries[materialNum] = GetMaterialInfo(surfaceMaterial, textureLocations);
    }

    if (materialNum >= userProperties.size()) {
      userProperties.resize(materialNum + 1);
    }
    if (userProperties[materialNum].empty()) {
      FbxProperty objectProperty = surfaceMaterial->GetFirstProperty();
      while (objectProperty.IsValid()) {
        if (objectProperty.GetFlag(FbxPropertyFlags::eUserDefined)) {
          userProperties[materialNum].push_back(TranscribeProperty(objectProperty).dump());
        }
        objectProperty = surfaceMaterial->GetNextProperty(objectProperty);
      }
    }
  }
}

const std::shared_ptr<FbxMaterialInfo> FbxMaterialsAccess::GetMaterial(
    const int polygonIndex) const {
  if (mappingMode != FbxGeometryElement::eNone) {
    const int materialNum =
        indices->GetAt((mappingMode == FbxGeometryElement::eByPolygon) ? polygonIndex : 0);
    if (materialNum < 0) {
      return nullptr;
    }
    return summaries.at((unsigned long)materialNum);
  }
  return nullptr;
}

const std::vector<std::string> FbxMaterialsAccess::GetUserProperties(const int polygonIndex) const {
  if (mappingMode != FbxGeometryElement::eNone) {
    const int materialNum =
        indices->GetAt((mappingMode == FbxGeometryElement::eByPolygon) ? polygonIndex : 0);
    if (materialNum < 0) {
      return std::vector<std::string>();
    }
    return userProperties.at((unsigned long)materialNum);
  }
  return std::vector<std::string>();
}

std::unique_ptr<FbxMaterialInfo> FbxMaterialsAccess::GetMaterialInfo(
    FbxSurfaceMaterial* material,
    const std::map<const FbxTexture*, FbxString>& textureLocations) {
  std::unique_ptr<FbxMaterialInfo> res =
      FbxStingrayPBSMaterialResolver(material, textureLocations).resolve();
  if (res == nullptr) {
    res = Fbx3dsMaxPhysicalMaterialResolver(material, textureLocations).resolve();
    if (res == nullptr) {
      res = FbxTraditionalMaterialResolver(material, textureLocations).resolve();
    }
  }
  return res;
}
