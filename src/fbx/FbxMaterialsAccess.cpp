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

		FbxSurfaceMaterial* surfaceMaterial = mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(materialNum);
		
        if (materialNum >= summaries.size()) {
            summaries.resize(materialNum + 1);
        }
        auto summary = summaries[materialNum];
        if (summary == nullptr) {
            summary = summaries[materialNum] = GetMaterialInfo(
				surfaceMaterial,
                textureLocations);
        }

		if (materialNum >= userProperties.size()) {
			userProperties.resize(materialNum + 1);
		}
		if (userProperties[materialNum].empty()) {
			FbxProperty objectProperty = surfaceMaterial->GetFirstProperty();
			while (objectProperty.IsValid())
			{
				if (objectProperty.GetFlag(FbxPropertyFlags::eUserDefined)) {
					std::string ename;
					switch (objectProperty.GetPropertyDataType().GetType()) {
						case eFbxBool:      ename = "eFbxBool";      break;
						case eFbxChar:      ename = "eFbxChar";      break;
						case eFbxUChar:     ename = "eFbxUChar";     break;
						case eFbxShort:     ename = "eFbxShort";     break;
						case eFbxUShort:    ename = "eFbxUShort";    break;
						case eFbxInt:       ename = "eFbxInt";       break;
						case eFbxUInt:      ename = "eFbxUint";      break;
						case eFbxLongLong:  ename = "eFbxLongLong";  break;
						case eFbxULongLong: ename = "eFbxULongLong"; break;
						case eFbxFloat:     ename = "eFbxFloat";     break;
						case eFbxHalfFloat: ename = "eFbxHalfFloat"; break;
						case eFbxDouble:    ename = "eFbxDouble";    break;
						case eFbxDouble2:   ename = "eFbxDouble2";   break;
						case eFbxDouble3:   ename = "eFbxDouble3";   break;
						case eFbxDouble4:   ename = "eFbxDouble4";   break;
						case eFbxString:    ename = "eFbxString";    break;

						// Use this as fallback because it does not give very descriptive names
						default: ename = objectProperty.GetPropertyDataType().GetName(); break;
					}

					json p;
					p["type"] = ename;

					// Convert property value
					switch (objectProperty.GetPropertyDataType().GetType()) {
						case eFbxBool:
						case eFbxChar:
						case eFbxUChar:
						case eFbxShort:
						case eFbxUShort:
						case eFbxInt:
						case eFbxUInt:
						case eFbxLongLong: {
							p["value"] = objectProperty.EvaluateValue<long long>(FBXSDK_TIME_INFINITE);
							break;
						}
						case eFbxULongLong: {
							p["value"] = objectProperty.EvaluateValue<unsigned long long>(FBXSDK_TIME_INFINITE);
							break;
						}
						case eFbxFloat:
						case eFbxHalfFloat:
						case eFbxDouble: {
							p["value"] = objectProperty.EvaluateValue<double>(FBXSDK_TIME_INFINITE);
							break;
						}
						case eFbxDouble2: {
							auto v = objectProperty.EvaluateValue<FbxDouble2>(FBXSDK_TIME_INFINITE);
							p["value"] = { v[0], v[1] };
							break;
						}
						case eFbxDouble3: {
							auto v = objectProperty.EvaluateValue<FbxDouble3>(FBXSDK_TIME_INFINITE);
							p["value"] = { v[0], v[1], v[2] };
							break;
						}
						case eFbxDouble4: {
							auto v = objectProperty.EvaluateValue<FbxDouble4>(FBXSDK_TIME_INFINITE);
							p["value"] = { v[0], v[1], v[2], v[3] };
							break;
						}
						case eFbxString: {
							p["value"] = std::string{ objectProperty.Get<FbxString>() };
							break;
						}
						default: {
							p["value"] = "UNSUPPORTED_VALUE_TYPE";
							break;
						}
					}

					json n;
					n[objectProperty.GetNameAsCStr()] = p;

					userProperties[materialNum].push_back(n.dump());
				}

				objectProperty = surfaceMaterial->GetNextProperty(objectProperty);
			}
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

const std::vector<std::string> FbxMaterialsAccess::GetUserProperties(const int polygonIndex) const
{
	if (mappingMode != FbxGeometryElement::eNone) {
		const int materialNum = indices->GetAt((mappingMode == FbxGeometryElement::eByPolygon) ? polygonIndex : 0);
		if (materialNum < 0) {
			return std::vector<std::string>();
		}
		return userProperties.at((unsigned long)materialNum);
	}
	return std::vector<std::string>();
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

