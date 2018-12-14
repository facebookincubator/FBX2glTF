/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "Fbx2Raw.hpp"
#include "FbxMaterialInfo.hpp"
#include "FbxTraditionalMaterialInfo.hpp"
#include "FbxRoughMetMaterialInfo.hpp"

class FbxMaterialsAccess
{
public:

    FbxMaterialsAccess(const FbxMesh *pMesh, const std::map<const FbxTexture *, FbxString> &textureLocations);

    const std::shared_ptr<FbxMaterialInfo> GetMaterial(const int polygonIndex) const;

	const std::vector<std::string> GetUserProperties(const int polygonIndex) const;

    std::unique_ptr<FbxMaterialInfo>
    GetMaterialInfo(FbxSurfaceMaterial *material, const std::map<const FbxTexture *, FbxString> &textureLocations);

private:
    FbxGeometryElement::EMappingMode              mappingMode;
    std::vector<std::shared_ptr<FbxMaterialInfo>> summaries {};
	std::vector<std::vector<std::string>>         userProperties;
    const FbxMesh                                 *mesh;
    const FbxLayerElementArrayTemplate<int>       *indices;
};
