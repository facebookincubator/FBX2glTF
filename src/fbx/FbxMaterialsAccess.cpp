/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "FbxMaterialsAccess.hpp"

FbxMaterialsAccess::FbxMaterialsAccess(const FbxMesh *pMesh, const std::map<const FbxTexture *, FbxString> &textureLocations) :
    mappingMode(FbxGeometryElement::eNone),
    mesh(nullptr),
    indices(nullptr)
{
    if (pMesh->GetElementMaterialCount() <= 0) {
        return;
    }

    const FbxGeometryElement::EMappingMode materialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
    if (materialMappingMode != FbxGeometryElement::eByPolygon && materialMappingMode != FbxGeometryElement::eAllSame) {
        return;
    }

    const FbxGeometryElement::EReferenceMode materialReferenceMode = pMesh->GetElementMaterial()->GetReferenceMode();
    if (materialReferenceMode != FbxGeometryElement::eIndexToDirect) {
        return;
    }

    mappingMode = materialMappingMode;
    mesh        = pMesh;
    indices     = &pMesh->GetElementMaterial()->GetIndexArray();

    for (int ii = 0; ii < indices->GetCount(); ii++) {
        int materialNum = indices->GetAt(ii);
        if (materialNum < 0) {
            continue;
        }
        if (materialNum >= summaries.size()) {
            summaries.resize(materialNum + 1);
        }
        auto summary = summaries[materialNum];
        if (summary == nullptr) {
            summary = summaries[materialNum] = GetMaterialInfo(
                mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(materialNum),
                textureLocations);
        }
    }
}

const std::shared_ptr<FbxMaterialInfo> FbxMaterialsAccess::GetMaterial(const int polygonIndex) const
{
    if (mappingMode != FbxGeometryElement::eNone) {
        const int materialNum = indices->GetAt((mappingMode == FbxGeometryElement::eByPolygon) ? polygonIndex : 0);
        if (materialNum < 0) {
            return nullptr;
        }
        return summaries.at((unsigned long) materialNum);
    }
    return nullptr;
}

std::unique_ptr<FbxMaterialInfo>
FbxMaterialsAccess::GetMaterialInfo(FbxSurfaceMaterial *material, const std::map<const FbxTexture *, FbxString> &textureLocations)
{
    std::unique_ptr<FbxMaterialInfo> res;
    res = FbxRoughMetMaterialInfo::From(material, textureLocations);
    if (!res) {
        res = FbxTraditionalMaterialInfo::From(material, textureLocations);
    }
    return res;
}

