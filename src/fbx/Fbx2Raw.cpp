/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Fbx2Raw.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "FBX2glTF.h"

#include "raw/RawModel.hpp"
#include "utils/File_Utils.hpp"
#include "utils/String_Utils.hpp"

#include "FbxBlendShapesAccess.hpp"
#include "FbxLayerElementAccess.hpp"
#include "FbxSkinningAccess.hpp"
#include "materials/RoughnessMetallicMaterials.hpp"
#include "materials/TraditionalMaterials.hpp"

#ifdef _WIN32
#define SLASH_CHAR '\\'
#define ALTERNATIVE_SLASH_CHAR '/'
#else
#define SLASH_CHAR '/'
#define ALTERNATIVE_SLASH_CHAR '\\'
#endif

float scaleFactor;

static std::string NativeToUTF8(const std::string& str) {
#if _WIN32
  char* u8cstr = nullptr;
#if (_UNICODE || UNICODE)
  FbxWCToUTF8(reinterpret_cast<const char*>(str.c_str()), u8cstr);
#else
  FbxAnsiToUTF8(str.c_str(), u8cstr);
#endif
  if (!u8cstr) {
    return str;
  } else {
    std::string u8str = u8cstr;
    delete[] u8cstr;
    return u8str;
  }
#else
  return str;
#endif
}

static bool TriangleTexturePolarity(const Vec2f& uv0, const Vec2f& uv1, const Vec2f& uv2) {
  const Vec2f d0 = uv1 - uv0;
  const Vec2f d1 = uv2 - uv0;

  return (d0[0] * d1[1] - d0[1] * d1[0] < 0.0f);
}

static RawMaterialType GetMaterialType(
    const RawModel& raw,
    const int textures[RAW_TEXTURE_USAGE_MAX],
    const bool vertexTransparency,
    const bool skinned) {
  // DIFFUSE and ALBEDO are different enough to represent distinctly, but they both help determine
  // transparency.
  int diffuseTexture = textures[RAW_TEXTURE_USAGE_DIFFUSE];
  if (diffuseTexture < 0) {
    diffuseTexture = textures[RAW_TEXTURE_USAGE_ALBEDO];
  }
  // determine material type based on texture occlusion.
  if (diffuseTexture >= 0) {
    return (raw.GetTexture(diffuseTexture).occlusion == RAW_TEXTURE_OCCLUSION_OPAQUE)
        ? (skinned ? RAW_MATERIAL_TYPE_SKINNED_OPAQUE : RAW_MATERIAL_TYPE_OPAQUE)
        : (skinned ? RAW_MATERIAL_TYPE_SKINNED_TRANSPARENT : RAW_MATERIAL_TYPE_TRANSPARENT);
  }

  // else if there is any vertex transparency, treat whole mesh as transparent
  if (vertexTransparency) {
    return skinned ? RAW_MATERIAL_TYPE_SKINNED_TRANSPARENT : RAW_MATERIAL_TYPE_TRANSPARENT;
  }

  // Default to simply opaque.
  return skinned ? RAW_MATERIAL_TYPE_SKINNED_OPAQUE : RAW_MATERIAL_TYPE_OPAQUE;
}

static void ReadMesh(
    RawModel& raw,
    FbxScene* pScene,
    FbxNode* pNode,
    const std::map<const FbxTexture*, FbxString>& textureLocations) {
  FbxGeometryConverter meshConverter(pScene->GetFbxManager());
  meshConverter.Triangulate(pNode->GetNodeAttribute(), true);
  FbxMesh* pMesh = pNode->GetMesh();

  // Obtains the surface Id
  const long surfaceId = pMesh->GetUniqueID();

  // Associate the node to this surface
  int nodeId = raw.GetNodeById(pNode->GetUniqueID());
  if (nodeId >= 0) {
    RawNode& node = raw.GetNode(nodeId);
    node.surfaceId = surfaceId;
  }

  if (raw.GetSurfaceById(surfaceId) >= 0) {
    // This surface is already loaded
    return;
  }

  const char* meshName = (pNode->GetName()[0] != '\0') ? pNode->GetName() : pMesh->GetName();
  const int rawSurfaceIndex = raw.AddSurface(meshName, surfaceId);

  const FbxVector4* controlPoints = pMesh->GetControlPoints();
  const FbxLayerElementAccess<FbxVector4> normalLayer(
      pMesh->GetElementNormal(), pMesh->GetElementNormalCount());
  const FbxLayerElementAccess<FbxVector4> binormalLayer(
      pMesh->GetElementBinormal(), pMesh->GetElementBinormalCount());
  const FbxLayerElementAccess<FbxVector4> tangentLayer(
      pMesh->GetElementTangent(), pMesh->GetElementTangentCount());
  const FbxLayerElementAccess<FbxColor> colorLayer(
      pMesh->GetElementVertexColor(), pMesh->GetElementVertexColorCount());
  const FbxLayerElementAccess<FbxVector2> uvLayer0(
      pMesh->GetElementUV(0), pMesh->GetElementUVCount());
  const FbxLayerElementAccess<FbxVector2> uvLayer1(
      pMesh->GetElementUV(1), pMesh->GetElementUVCount());
  const FbxSkinningAccess skinning(pMesh, pScene, pNode);
  const FbxMaterialsAccess materials(pMesh, textureLocations);
  const FbxBlendShapesAccess blendShapes(pMesh);

  if (verboseOutput) {
    fmt::printf(
        "mesh %d: %s (skinned: %s)\n",
        rawSurfaceIndex,
        meshName,
        skinning.IsSkinned() ? raw.GetNode(raw.GetNodeById(skinning.GetRootNode())).name.c_str()
                             : "NO");
  }

  // The FbxNode geometric transformation describes how a FbxNodeAttribute is offset from
  // the FbxNode's local frame of reference. These geometric transforms are applied to the
  // FbxNodeAttribute after the FbxNode's local transforms are computed, and are not
  // inherited across the node hierarchy.
  // Apply the geometric transform to the mesh geometry (vertices, normal etc.) because
  // glTF does not have an equivalent to the geometric transform.
  const FbxVector4 meshTranslation = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
  const FbxVector4 meshRotation = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
  const FbxVector4 meshScaling = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
  const FbxAMatrix meshTransform(meshTranslation, meshRotation, meshScaling);
  const FbxMatrix transform = meshTransform;

  // Remove translation & scaling from transforms that will bi applied to normals, tangents &
  // binormals
  const FbxMatrix normalTransform(FbxVector4(), meshRotation, meshScaling);
  const FbxMatrix inverseTransposeTransform = normalTransform.Inverse().Transpose();

  raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_POSITION);
  if (normalLayer.LayerPresent()) {
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_NORMAL);
  }
  if (tangentLayer.LayerPresent()) {
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_TANGENT);
  }
  if (binormalLayer.LayerPresent()) {
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_BINORMAL);
  }
  if (colorLayer.LayerPresent()) {
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_COLOR);
  }
  if (uvLayer0.LayerPresent()) {
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_UV0);
  }
  if (uvLayer1.LayerPresent()) {
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_UV1);
  }
  if (skinning.IsSkinned()) {
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS);
    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_JOINT_INDICES);
  }

  RawSurface& rawSurface = raw.GetSurface(rawSurfaceIndex);

  Mat4f scaleMatrix = Mat4f::FromScaleVector(Vec3f(scaleFactor, scaleFactor, scaleFactor));
  Mat4f invScaleMatrix = scaleMatrix.Inverse();

  rawSurface.skeletonRootId =
      (skinning.IsSkinned()) ? skinning.GetRootNode() : pNode->GetUniqueID();
  for (int jointIndex = 0; jointIndex < skinning.GetNodeCount(); jointIndex++) {
    const long jointId = skinning.GetJointId(jointIndex);
    raw.GetNode(raw.GetNodeById(jointId)).isJoint = true;

    rawSurface.jointIds.emplace_back(jointId);
    rawSurface.inverseBindMatrices.push_back(
        invScaleMatrix * toMat4f(skinning.GetInverseBindMatrix(jointIndex)) * scaleMatrix);
    rawSurface.jointGeometryMins.emplace_back(FLT_MAX, FLT_MAX, FLT_MAX);
    rawSurface.jointGeometryMaxs.emplace_back(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  }

  rawSurface.blendChannels.clear();
  std::vector<const FbxBlendShapesAccess::TargetShape*> targetShapes;
  for (size_t channelIx = 0; channelIx < blendShapes.GetChannelCount(); channelIx++) {
    for (size_t targetIx = 0; targetIx < blendShapes.GetTargetShapeCount(channelIx); targetIx++) {
      const FbxBlendShapesAccess::TargetShape& shape =
          blendShapes.GetTargetShape(channelIx, targetIx);
      targetShapes.push_back(&shape);
      auto& blendChannel = blendShapes.GetBlendChannel(channelIx);

      rawSurface.blendChannels.push_back(
          RawBlendChannel{static_cast<float>(blendChannel.deformPercent),
                          shape.normals.LayerPresent(),
                          shape.tangents.LayerPresent(),
                          blendChannel.name});
    }
  }

  int polygonVertexIndex = 0;
  for (int polygonIndex = 0; polygonIndex < pMesh->GetPolygonCount(); polygonIndex++) {
    FBX_ASSERT(pMesh->GetPolygonSize(polygonIndex) == 3);
    const std::shared_ptr<FbxMaterialInfo> fbxMaterial = materials.GetMaterial(polygonIndex);
    const std::vector<std::string> userProperties = materials.GetUserProperties(polygonIndex);

    int textures[RAW_TEXTURE_USAGE_MAX];
    std::fill_n(textures, (int)RAW_TEXTURE_USAGE_MAX, -1);

    std::shared_ptr<RawMatProps> rawMatProps;
    FbxString materialName;
    long materialId;

    if (fbxMaterial == nullptr) {
      materialName = "DefaultMaterial";
      materialId = -1;
      rawMatProps.reset(new RawTraditionalMatProps(
          RAW_SHADING_MODEL_LAMBERT,
          Vec3f(0, 0, 0),
          Vec4f(.5, .5, .5, 1),
          Vec3f(0, 0, 0),
          Vec3f(0, 0, 0),
          0.5));

    } else {
      materialName = fbxMaterial->name;
      materialId = fbxMaterial->id;

      const auto maybeAddTexture = [&](const FbxFileTexture* tex, RawTextureUsage usage) {
        if (tex != nullptr) {
          // dig out the inferred filename from the textureLocations map
          FbxString inferredPath = textureLocations.find(tex)->second;
          textures[usage] =
              raw.AddTexture(tex->GetName(), tex->GetFileName(), inferredPath.Buffer(), usage);
        }
      };

      std::shared_ptr<RawMatProps> matInfo;
      if (fbxMaterial->shadingModel == FbxRoughMetMaterialInfo::FBX_SHADER_METROUGH) {
        FbxRoughMetMaterialInfo* fbxMatInfo =
            static_cast<FbxRoughMetMaterialInfo*>(fbxMaterial.get());

        maybeAddTexture(fbxMatInfo->texBaseColor, RAW_TEXTURE_USAGE_ALBEDO);
        maybeAddTexture(fbxMatInfo->texNormal, RAW_TEXTURE_USAGE_NORMAL);
        maybeAddTexture(fbxMatInfo->texEmissive, RAW_TEXTURE_USAGE_EMISSIVE);
        maybeAddTexture(fbxMatInfo->texRoughness, RAW_TEXTURE_USAGE_ROUGHNESS);
        maybeAddTexture(fbxMatInfo->texMetallic, RAW_TEXTURE_USAGE_METALLIC);
        maybeAddTexture(fbxMatInfo->texAmbientOcclusion, RAW_TEXTURE_USAGE_OCCLUSION);
        rawMatProps.reset(new RawMetRoughMatProps(
            RAW_SHADING_MODEL_PBR_MET_ROUGH,
            toVec4f(fbxMatInfo->baseColor),
            toVec3f(fbxMatInfo->emissive),
            fbxMatInfo->emissiveIntensity,
            fbxMatInfo->metallic,
            fbxMatInfo->roughness,
            fbxMatInfo->invertRoughnessMap));
      } else {
        FbxTraditionalMaterialInfo* fbxMatInfo =
            static_cast<FbxTraditionalMaterialInfo*>(fbxMaterial.get());
        RawShadingModel shadingModel;
        if (fbxMaterial->shadingModel == "Lambert") {
          shadingModel = RAW_SHADING_MODEL_LAMBERT;
        } else if (0 == fbxMaterial->shadingModel.CompareNoCase("Blinn")) {
          shadingModel = RAW_SHADING_MODEL_BLINN;
        } else if (0 == fbxMaterial->shadingModel.CompareNoCase("Phong")) {
          shadingModel = RAW_SHADING_MODEL_PHONG;
        } else if (0 == fbxMaterial->shadingModel.CompareNoCase("Constant")) {
          shadingModel = RAW_SHADING_MODEL_PHONG;
        } else {
          shadingModel = RAW_SHADING_MODEL_UNKNOWN;
        }
        maybeAddTexture(fbxMatInfo->texDiffuse, RAW_TEXTURE_USAGE_DIFFUSE);
        maybeAddTexture(fbxMatInfo->texNormal, RAW_TEXTURE_USAGE_NORMAL);
        maybeAddTexture(fbxMatInfo->texEmissive, RAW_TEXTURE_USAGE_EMISSIVE);
        maybeAddTexture(fbxMatInfo->texShininess, RAW_TEXTURE_USAGE_SHININESS);
        maybeAddTexture(fbxMatInfo->texAmbient, RAW_TEXTURE_USAGE_AMBIENT);
        maybeAddTexture(fbxMatInfo->texSpecular, RAW_TEXTURE_USAGE_SPECULAR);
        rawMatProps.reset(new RawTraditionalMatProps(
            shadingModel,
            toVec3f(fbxMatInfo->colAmbient),
            toVec4f(fbxMatInfo->colDiffuse),
            toVec3f(fbxMatInfo->colEmissive),
            toVec3f(fbxMatInfo->colSpecular),
            fbxMatInfo->shininess));
      }
    }

    RawVertex rawVertices[3];
    bool vertexTransparency = false;
    for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++, polygonVertexIndex++) {
      const int controlPointIndex = pMesh->GetPolygonVertex(polygonIndex, vertexIndex);

      // Note that the default values here must be the same as the RawVertex default values!
      const FbxVector4 fbxPosition = transform.MultNormalize(controlPoints[controlPointIndex]);
      const FbxVector4 fbxNormal = normalLayer.GetElement(
          polygonIndex,
          polygonVertexIndex,
          controlPointIndex,
          FbxVector4(0.0f, 0.0f, 0.0f, 0.0f),
          inverseTransposeTransform,
          true);
      const FbxVector4 fbxTangent = tangentLayer.GetElement(
          polygonIndex,
          polygonVertexIndex,
          controlPointIndex,
          FbxVector4(0.0f, 0.0f, 0.0f, 0.0f),
          inverseTransposeTransform,
          true);
      const FbxVector4 fbxBinormal = binormalLayer.GetElement(
          polygonIndex,
          polygonVertexIndex,
          controlPointIndex,
          FbxVector4(0.0f, 0.0f, 0.0f, 0.0f),
          inverseTransposeTransform,
          true);
      const FbxColor fbxColor = colorLayer.GetElement(
          polygonIndex, polygonVertexIndex, controlPointIndex, FbxColor(0.0f, 0.0f, 0.0f, 0.0f));
      const FbxVector2 fbxUV0 = uvLayer0.GetElement(
          polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector2(0.0f, 0.0f));
      const FbxVector2 fbxUV1 = uvLayer1.GetElement(
          polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector2(0.0f, 0.0f));

      RawVertex& vertex = rawVertices[vertexIndex];
      vertex.position[0] = (float)fbxPosition[0] * scaleFactor;
      vertex.position[1] = (float)fbxPosition[1] * scaleFactor;
      vertex.position[2] = (float)fbxPosition[2] * scaleFactor;
      vertex.normal[0] = (float)fbxNormal[0];
      vertex.normal[1] = (float)fbxNormal[1];
      vertex.normal[2] = (float)fbxNormal[2];
      vertex.tangent[0] = (float)fbxTangent[0];
      vertex.tangent[1] = (float)fbxTangent[1];
      vertex.tangent[2] = (float)fbxTangent[2];
      vertex.tangent[3] = (float)fbxTangent[3];
      vertex.binormal[0] = (float)fbxBinormal[0];
      vertex.binormal[1] = (float)fbxBinormal[1];
      vertex.binormal[2] = (float)fbxBinormal[2];
      vertex.color[0] = (float)fbxColor.mRed;
      vertex.color[1] = (float)fbxColor.mGreen;
      vertex.color[2] = (float)fbxColor.mBlue;
      vertex.color[3] = (float)fbxColor.mAlpha;
      vertex.uv0[0] = (float)fbxUV0[0];
      vertex.uv0[1] = (float)fbxUV0[1];
      vertex.uv1[0] = (float)fbxUV1[0];
      vertex.uv1[1] = (float)fbxUV1[1];
      vertex.jointIndices = skinning.GetVertexIndices(controlPointIndex);
      vertex.jointWeights = skinning.GetVertexWeights(controlPointIndex);
      vertex.polarityUv0 = false;

      // flag this triangle as transparent if any of its corner vertices substantially deviates from
      // fully opaque
      vertexTransparency |= colorLayer.LayerPresent() && (fabs(fbxColor.mAlpha - 1.0) > 1e-3);

      rawSurface.bounds.AddPoint(vertex.position);

      if (!targetShapes.empty()) {
        vertex.blendSurfaceIx = rawSurfaceIndex;
        for (const auto* targetShape : targetShapes) {
          RawBlendVertex blendVertex;
          // the morph target data must be transformed just as with the vertex positions above
          const FbxVector4& shapePosition =
              transform.MultNormalize(targetShape->positions[controlPointIndex]);
          blendVertex.position = toVec3f(shapePosition - fbxPosition) * scaleFactor;
          if (targetShape->normals.LayerPresent()) {
            const FbxVector4& normal = targetShape->normals.GetElement(
                polygonIndex,
                polygonVertexIndex,
                controlPointIndex,
                FbxVector4(0.0f, 0.0f, 0.0f, 0.0f),
                inverseTransposeTransform,
                true);
            blendVertex.normal = toVec3f(normal - fbxNormal);
          }
          if (targetShape->tangents.LayerPresent()) {
            const FbxVector4& tangent = targetShape->tangents.GetElement(
                polygonIndex,
                polygonVertexIndex,
                controlPointIndex,
                FbxVector4(0.0f, 0.0f, 0.0f, 0.0f),
                inverseTransposeTransform,
                true);
            blendVertex.tangent = toVec4f(tangent - fbxTangent);
          }
          vertex.blends.push_back(blendVertex);
        }
      } else {
        vertex.blendSurfaceIx = -1;
      }

      if (skinning.IsSkinned()) {
        const int jointIndices[FbxSkinningAccess::MAX_WEIGHTS] = {vertex.jointIndices[0],
                                                                  vertex.jointIndices[1],
                                                                  vertex.jointIndices[2],
                                                                  vertex.jointIndices[3]};
        const float jointWeights[FbxSkinningAccess::MAX_WEIGHTS] = {vertex.jointWeights[0],
                                                                    vertex.jointWeights[1],
                                                                    vertex.jointWeights[2],
                                                                    vertex.jointWeights[3]};
        const FbxMatrix skinningMatrix =
            skinning.GetJointSkinningTransform(jointIndices[0]) * jointWeights[0] +
            skinning.GetJointSkinningTransform(jointIndices[1]) * jointWeights[1] +
            skinning.GetJointSkinningTransform(jointIndices[2]) * jointWeights[2] +
            skinning.GetJointSkinningTransform(jointIndices[3]) * jointWeights[3];

        const FbxVector4 globalPosition = skinningMatrix.MultNormalize(fbxPosition);
        for (int i = 0; i < FbxSkinningAccess::MAX_WEIGHTS; i++) {
          if (jointWeights[i] > 0.0f) {
            const FbxVector4 localPosition =
                skinning.GetJointInverseGlobalTransforms(jointIndices[i])
                    .MultNormalize(globalPosition);

            Vec3f& mins = rawSurface.jointGeometryMins[jointIndices[i]];
            mins[0] = std::min(mins[0], (float)localPosition[0]);
            mins[1] = std::min(mins[1], (float)localPosition[1]);
            mins[2] = std::min(mins[2], (float)localPosition[2]);

            Vec3f& maxs = rawSurface.jointGeometryMaxs[jointIndices[i]];
            maxs[0] = std::max(maxs[0], (float)localPosition[0]);
            maxs[1] = std::max(maxs[1], (float)localPosition[1]);
            maxs[2] = std::max(maxs[2], (float)localPosition[2]);
          }
        }
      }
    }

    if (textures[RAW_TEXTURE_USAGE_NORMAL] != -1) {
      // Distinguish vertices that are used by triangles with a different texture polarity to avoid
      // degenerate tangent space smoothing.
      const bool polarity =
          TriangleTexturePolarity(rawVertices[0].uv0, rawVertices[1].uv0, rawVertices[2].uv0);
      rawVertices[0].polarityUv0 = polarity;
      rawVertices[1].polarityUv0 = polarity;
      rawVertices[2].polarityUv0 = polarity;
    }

    int rawVertexIndices[3];
    for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
      rawVertexIndices[vertexIndex] = raw.AddVertex(rawVertices[vertexIndex]);
    }

    const RawMaterialType materialType =
        GetMaterialType(raw, textures, vertexTransparency, skinning.IsSkinned());
    const int rawMaterialIndex = raw.AddMaterial(
        materialId, materialName, materialType, textures, rawMatProps, userProperties);

    raw.AddTriangle(
        rawVertexIndices[0],
        rawVertexIndices[1],
        rawVertexIndices[2],
        rawMaterialIndex,
        rawSurfaceIndex);
  }
}

// ar : aspectY / aspectX
double HFOV2VFOV(double h, double ar) {
  return 2.0 * std::atan((ar)*std::tan((h * FBXSDK_PI_DIV_180) * 0.5)) * FBXSDK_180_DIV_PI;
};

// ar : aspectX / aspectY
double VFOV2HFOV(double v, double ar) {
  return 2.0 * std::atan((ar)*std::tan((v * FBXSDK_PI_DIV_180) * 0.5)) * FBXSDK_180_DIV_PI;
}

static void ReadLight(RawModel& raw, FbxScene* pScene, FbxNode* pNode) {
  const FbxLight* pLight = pNode->GetLight();

  int lightIx;
  float intensity = (float)pLight->Intensity.Get();
  Vec3f color = toVec3f(pLight->Color.Get());
  switch (pLight->LightType.Get()) {
    case FbxLight::eDirectional: {
      lightIx = raw.AddLight(pLight->GetName(), RAW_LIGHT_TYPE_DIRECTIONAL, color, intensity, 0, 0);
      break;
    }
    case FbxLight::ePoint: {
      lightIx = raw.AddLight(pLight->GetName(), RAW_LIGHT_TYPE_POINT, color, intensity, 0, 0);
      break;
    }
    case FbxLight::eSpot: {
      lightIx = raw.AddLight(
          pLight->GetName(),
          RAW_LIGHT_TYPE_SPOT,
          color,
          intensity,
          (float)pLight->InnerAngle.Get() * M_PI / 180,
          (float)pLight->OuterAngle.Get() * M_PI / 180);
      break;
    }
    default: {
      fmt::printf("Warning:: Ignoring unsupported light type.\n");
      return;
    }
  }

  int nodeId = raw.GetNodeById(pNode->GetUniqueID());
  RawNode& node = raw.GetNode(nodeId);
  node.lightIx = lightIx;
}

// Largely adopted from fbx example
static void ReadCamera(RawModel& raw, FbxScene* pScene, FbxNode* pNode) {
  const FbxCamera* pCamera = pNode->GetCamera();

  double filmHeight = pCamera->GetApertureHeight();
  double filmWidth = pCamera->GetApertureWidth() * pCamera->GetSqueezeRatio();

  // note Height : Width
  double apertureRatio = filmHeight / filmWidth;

  double fovx = 0.0f;
  double fovy = 0.0f;

  switch (pCamera->GetApertureMode()) {
    case FbxCamera::EApertureMode::eHorizAndVert: {
      fovx = pCamera->FieldOfViewX;
      fovy = pCamera->FieldOfViewY;
      break;
    }
    case FbxCamera::EApertureMode::eHorizontal: {
      fovx = pCamera->FieldOfView;
      fovy = HFOV2VFOV(fovx, apertureRatio);
      break;
    }
    case FbxCamera::EApertureMode::eVertical: {
      fovy = pCamera->FieldOfView;
      fovx = VFOV2HFOV(fovy, 1.0 / apertureRatio);
      break;
    }
    case FbxCamera::EApertureMode::eFocalLength: {
      fovx = pCamera->ComputeFieldOfView(pCamera->FocalLength);
      fovy = HFOV2VFOV(fovx, apertureRatio);
      break;
    }
    default: {
      fmt::printf("Warning:: Unsupported ApertureMode. Setting FOV to 0.\n");
      break;
    }
  }

  if (pCamera->ProjectionType.Get() == FbxCamera::EProjectionType::ePerspective) {
    raw.AddCameraPerspective(
        "",
        pNode->GetUniqueID(),
        (float)pCamera->FilmAspectRatio,
        (float)fovx,
        (float)fovy,
        (float)pCamera->NearPlane,
        (float)pCamera->FarPlane);
  } else {
    raw.AddCameraOrthographic(
        "",
        pNode->GetUniqueID(),
        (float)pCamera->OrthoZoom,
        (float)pCamera->OrthoZoom,
        (float)pCamera->FarPlane,
        (float)pCamera->NearPlane);
  }

  // Cameras in FBX coordinate space face +X when rotation is (0,0,0)
  // We need to adjust this to face glTF specified -Z
  auto nodeIdx = raw.GetNodeById(pNode->GetUniqueID());
  auto& rawNode = raw.GetNode(nodeIdx);

  auto r = Quatf::FromAngleAxis(-90 * ((float)M_PI / 180.0f), {0.0, 1.0, 0.0});
  rawNode.rotation = rawNode.rotation * r;
}

static void ReadNodeProperty(RawModel& raw, FbxNode* pNode, FbxProperty& prop) {
  int nodeId = raw.GetNodeById(pNode->GetUniqueID());
  if (nodeId >= 0) {
    RawNode& node = raw.GetNode(nodeId);
    node.userProperties.push_back(TranscribeProperty(prop).dump());
  }
}

static void ReadNodeAttributes(
    RawModel& raw,
    FbxScene* pScene,
    FbxNode* pNode,
    const std::map<const FbxTexture*, FbxString>& textureLocations) {
  if (!pNode->GetVisibility()) {
    return;
  }

  // Only support non-animated user defined properties for now
  FbxProperty objectProperty = pNode->GetFirstProperty();
  while (objectProperty.IsValid()) {
    if (objectProperty.GetFlag(FbxPropertyFlags::eUserDefined)) {
      ReadNodeProperty(raw, pNode, objectProperty);
    }

    objectProperty = pNode->GetNextProperty(objectProperty);
  }

  FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
  if (pNodeAttribute != nullptr) {
    const FbxNodeAttribute::EType attributeType = pNodeAttribute->GetAttributeType();
    switch (attributeType) {
      case FbxNodeAttribute::eMesh:
      case FbxNodeAttribute::eNurbs:
      case FbxNodeAttribute::eNurbsSurface:
      case FbxNodeAttribute::eTrimNurbsSurface:
      case FbxNodeAttribute::ePatch: {
        ReadMesh(raw, pScene, pNode, textureLocations);
        break;
      }
      case FbxNodeAttribute::eCamera: {
        ReadCamera(raw, pScene, pNode);
        break;
      }
      case FbxNodeAttribute::eLight:
        ReadLight(raw, pScene, pNode);
        break;
      case FbxNodeAttribute::eUnknown:
      case FbxNodeAttribute::eNull:
      case FbxNodeAttribute::eMarker:
      case FbxNodeAttribute::eSkeleton:
      case FbxNodeAttribute::eCameraStereo:
      case FbxNodeAttribute::eCameraSwitcher:
      case FbxNodeAttribute::eOpticalReference:
      case FbxNodeAttribute::eOpticalMarker:
      case FbxNodeAttribute::eNurbsCurve:
      case FbxNodeAttribute::eBoundary:
      case FbxNodeAttribute::eShape:
      case FbxNodeAttribute::eLODGroup:
      case FbxNodeAttribute::eSubDiv:
      case FbxNodeAttribute::eCachedEffect:
      case FbxNodeAttribute::eLine: {
        break;
      }
    }
  }

  for (int child = 0; child < pNode->GetChildCount(); child++) {
    ReadNodeAttributes(raw, pScene, pNode->GetChild(child), textureLocations);
  }
}

/**
 * Compute the local scale vector to use for a given node. This is an imperfect hack to cope with
 * the FBX node transform's eInheritRrs inheritance type, in which ancestral scale is ignored
 */
static FbxVector4 computeLocalScale(FbxNode* pNode, FbxTime pTime = FBXSDK_TIME_INFINITE) {
  const FbxVector4 lScale = pNode->EvaluateLocalTransform(pTime).GetS();

  if (pNode->GetParent() == nullptr ||
      pNode->GetTransform().GetInheritType() != FbxTransform::eInheritRrs) {
    return lScale;
  }
  // This is a very partial fix that is only correct for models that use identity scale in their
  // rig's joints. We could write better support that compares local scale to parent's global scale
  // and apply the ratio to our local translation. We'll always want to return scale 1, though --
  // that's the only way to encode the missing 'S' (parent scale) in the transform chain.
  return FbxVector4(1, 1, 1, 1);
}

static void ReadNodeHierarchy(
    RawModel& raw,
    FbxScene* pScene,
    FbxNode* pNode,
    const long parentId,
    const std::string& path) {
  const FbxUInt64 nodeId = pNode->GetUniqueID();
  const char* nodeName = pNode->GetName();
  const int nodeIndex = raw.AddNode(nodeId, nodeName, parentId);
  RawNode& node = raw.GetNode(nodeIndex);

  FbxTransform::EInheritType lInheritType;
  pNode->GetTransformationInheritType(lInheritType);

  std::string newPath = path + "/" + nodeName;
  if (verboseOutput) {
    fmt::printf("node %d: %s\n", nodeIndex, newPath.c_str());
  }

  static int warnRrSsCount = 0;
  static int warnRrsCount = 0;
  if (lInheritType == FbxTransform::eInheritRrSs && parentId) {
    if (++warnRrSsCount == 1) {
      fmt::printf(
          "Warning: node %s uses unsupported transform inheritance type 'eInheritRrSs'.\n",
          newPath);
      fmt::printf("         (Further warnings of this type squelched.)\n");
    }

  } else if (lInheritType == FbxTransform::eInheritRrs) {
    if (++warnRrsCount == 1) {
      fmt::printf(
          "Warning: node %s uses unsupported transform inheritance type 'eInheritRrs'\n"
          "     This tool will attempt to partially compensate, but glTF cannot truly express this mode.\n"
          "     If this was a Maya export, consider turning off 'Segment Scale Compensate' on all joints.\n"
          "     (Further warnings of this type squelched.)\n",
          newPath);
    }
  }

  // Set the initial node transform.
  const FbxAMatrix localTransform = pNode->EvaluateLocalTransform();
  const FbxVector4 localTranslation = localTransform.GetT();
  const FbxQuaternion localRotation = localTransform.GetQ();
  const FbxVector4 localScaling = computeLocalScale(pNode);

  node.translation = toVec3f(localTranslation) * scaleFactor;
  node.rotation = toQuatf(localRotation);
  node.scale = toVec3f(localScaling);

  if (parentId) {
    RawNode& parentNode = raw.GetNode(raw.GetNodeById(parentId));
    // Add unique child name to the parent node.
    if (std::find(parentNode.childIds.begin(), parentNode.childIds.end(), nodeId) ==
        parentNode.childIds.end()) {
      parentNode.childIds.push_back(nodeId);
    }
  } else {
    // If there is no parent then this is the root node.
    raw.SetRootNode(nodeId);
  }

  for (int child = 0; child < pNode->GetChildCount(); child++) {
    ReadNodeHierarchy(raw, pScene, pNode->GetChild(child), nodeId, newPath);
  }
}

static void ReadAnimations(RawModel& raw, FbxScene* pScene, const GltfOptions& options) {
  FbxTime::EMode eMode = FbxTime::eFrames24;
  switch (options.animationFramerate) {
    case AnimationFramerateOptions::BAKE24:
      eMode = FbxTime::eFrames24;
      break;
    case AnimationFramerateOptions::BAKE30:
      eMode = FbxTime::eFrames30;
      break;
    case AnimationFramerateOptions::BAKE60:
      eMode = FbxTime::eFrames60;
      break;
  }
  const double epsilon = 1e-5f;

  const int animationCount = pScene->GetSrcObjectCount<FbxAnimStack>();
  for (size_t animIx = 0; animIx < animationCount; animIx++) {
    FbxAnimStack* pAnimStack = pScene->GetSrcObject<FbxAnimStack>(animIx);
    FbxString animStackName = pAnimStack->GetName();

    pScene->SetCurrentAnimationStack(pAnimStack);

    /**
     * Individual animations are often concatenated on the timeline, and the
     * only certain way to identify precisely what interval they occupy is to
     * depth-traverse the entire animation stack, and examine the actual keys.
     *
     * There is a deprecated concept of an "animation take" which is meant to
     * provide precisely this time interval information, but the data is not
     * actually derived by the SDK from source-of-truth data structures, but
     * rather provided directly by the FBX exporter, and not sanity checked.
     *
     * Some exporters calculate it correctly. Others do not. In any case, we
     * now ignore it completely.
     */
    FbxLongLong firstFrameIndex = -1;
    FbxLongLong lastFrameIndex = -1;
    for (int layerIx = 0; layerIx < pAnimStack->GetMemberCount(); layerIx++) {
      FbxAnimLayer* layer = pAnimStack->GetMember<FbxAnimLayer>(layerIx);
      for (int nodeIx = 0; nodeIx < layer->GetMemberCount(); nodeIx++) {
        auto* node = layer->GetMember<FbxAnimCurveNode>(nodeIx);
        FbxTimeSpan nodeTimeSpan;
        // Multiple curves per curve node is not even supported by the SDK.
        for (int curveIx = 0; curveIx < node->GetCurveCount(0); curveIx++) {
          FbxAnimCurve* curve = node->GetCurve(0U, curveIx);
          if (curve == nullptr) {
            continue;
          }
          // simply take the interval as first key to last key
          int firstKeyIndex = 0;
          int lastKeyIndex = std::max(firstKeyIndex, curve->KeyGetCount() - 1);
          FbxLongLong firstCurveFrame = curve->KeyGetTime(firstKeyIndex).GetFrameCount(eMode);
          FbxLongLong lastCurveFrame = curve->KeyGetTime(lastKeyIndex).GetFrameCount(eMode);

          // the final interval is the union of all node curve intervals
          if (firstFrameIndex == -1 || firstCurveFrame < firstFrameIndex) {
            firstFrameIndex = firstCurveFrame;
          }
          if (lastFrameIndex == -1 || lastCurveFrame > lastFrameIndex) {
            lastFrameIndex = lastCurveFrame;
          }
        }
      }
    }
    RawAnimation animation;
    animation.name = animStackName;

    fmt::printf(
        "Animation %s: [%lu - %lu]\n", std::string(animStackName), firstFrameIndex, lastFrameIndex);

    if (verboseOutput) {
      fmt::printf("animation %zu: %s (%d%%)", animIx, (const char*)animStackName, 0);
    }

    for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
      FbxTime pTime;
      // first frame is always at t = 0.0
      pTime.SetFrame(frameIndex - firstFrameIndex, eMode);
      animation.times.emplace_back((float)pTime.GetSecondDouble());
    }

    size_t totalSizeInBytes = 0;

    const int nodeCount = pScene->GetNodeCount();
    for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
      FbxNode* pNode = pScene->GetNode(nodeIndex);
      const FbxAMatrix baseTransform = pNode->EvaluateLocalTransform();
      const FbxVector4 baseTranslation = baseTransform.GetT();
      const FbxQuaternion baseRotation = baseTransform.GetQ();
      const FbxVector4 baseScaling = computeLocalScale(pNode);
      bool hasTranslation = false;
      bool hasRotation = false;
      bool hasScale = false;
      bool hasMorphs = false;

      RawChannel channel;
      channel.nodeIndex = raw.GetNodeById(pNode->GetUniqueID());

      for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
        FbxTime pTime;
        pTime.SetFrame(frameIndex, eMode);

        const FbxAMatrix localTransform = pNode->EvaluateLocalTransform(pTime);
        const FbxVector4 localTranslation = localTransform.GetT();
        const FbxQuaternion localRotation = localTransform.GetQ();
        const FbxVector4 localScale = computeLocalScale(pNode, pTime);

        hasTranslation |=
            (fabs(localTranslation[0] - baseTranslation[0]) > epsilon ||
             fabs(localTranslation[1] - baseTranslation[1]) > epsilon ||
             fabs(localTranslation[2] - baseTranslation[2]) > epsilon);
        hasRotation |=
            (fabs(localRotation[0] - baseRotation[0]) > epsilon ||
             fabs(localRotation[1] - baseRotation[1]) > epsilon ||
             fabs(localRotation[2] - baseRotation[2]) > epsilon ||
             fabs(localRotation[3] - baseRotation[3]) > epsilon);
        hasScale |=
            (fabs(localScale[0] - baseScaling[0]) > epsilon ||
             fabs(localScale[1] - baseScaling[1]) > epsilon ||
             fabs(localScale[2] - baseScaling[2]) > epsilon);

        channel.translations.push_back(toVec3f(localTranslation) * scaleFactor);
        channel.rotations.push_back(toQuatf(localRotation));
        channel.scales.push_back(toVec3f(localScale));
      }

      std::vector<FbxAnimCurve*> shapeAnimCurves;
      FbxNodeAttribute* nodeAttr = pNode->GetNodeAttribute();
      if (nodeAttr != nullptr && nodeAttr->GetAttributeType() == FbxNodeAttribute::EType::eMesh) {
        // it's inelegant to recreate this same access class multiple times, but it's also dirt
        // cheap...
        FbxBlendShapesAccess blendShapes(static_cast<FbxMesh*>(nodeAttr));

        for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
          FbxTime pTime;
          pTime.SetFrame(frameIndex, eMode);

          for (size_t channelIx = 0; channelIx < blendShapes.GetChannelCount(); channelIx++) {
            FbxAnimCurve* curve = blendShapes.GetAnimation(channelIx, animIx);
            float influence = (curve != nullptr) ? curve->Evaluate(pTime) : 0; // 0-100

            int targetCount = static_cast<int>(blendShapes.GetTargetShapeCount(channelIx));

            // the target shape 'fullWeight' values are a strictly ascending list of floats (between
            // 0 and 100), forming a sequence of intervals -- this convenience function figures out
            // if 'p' lays between some certain target fullWeights, and if so where (from 0 to 1).
            auto findInInterval = [&](const double p, const int n) {
              if (n >= targetCount) {
                // p is certainly completely left of this interval
                return NAN;
              }
              double leftWeight = 0;
              if (n >= 0) {
                leftWeight = blendShapes.GetTargetShape(channelIx, n).fullWeight;
                if (p < leftWeight) {
                  return NAN;
                }
                // the first interval implicitly includes all lesser influence values
              }
              double rightWeight = blendShapes.GetTargetShape(channelIx, n + 1).fullWeight;
              if (p > rightWeight && n + 1 < targetCount - 1) {
                return NAN;
                // the last interval implicitly includes all greater influence values
              }
              // transform p linearly such that [leftWeight, rightWeight] => [0, 1]
              return static_cast<float>((p - leftWeight) / (rightWeight - leftWeight));
            };

            for (int targetIx = 0; targetIx < targetCount; targetIx++) {
              if (curve) {
                float result = findInInterval(influence, targetIx - 1);
                if (!std::isnan(result)) {
                  // we're transitioning into targetIx
                  channel.weights.push_back(result);
                  hasMorphs = true;
                  continue;
                }
                if (targetIx != targetCount - 1) {
                  result = findInInterval(influence, targetIx);
                  if (!std::isnan(result)) {
                    // we're transitioning AWAY from targetIx
                    channel.weights.push_back(1.0f - result);
                    hasMorphs = true;
                    continue;
                  }
                }
              }

              // this is here because we have to fill in a weight for every channelIx/targetIx
              // permutation, regardless of whether or not they participate in this animation.
              channel.weights.push_back(0.0f);
            }
          }
        }
      }

      if (hasTranslation || hasRotation || hasScale || hasMorphs) {
        if (!hasTranslation) {
          channel.translations.clear();
        }
        if (!hasRotation) {
          channel.rotations.clear();
        }
        if (!hasScale) {
          channel.scales.clear();
        }
        if (!hasMorphs) {
          channel.weights.clear();
        }

        animation.channels.emplace_back(channel);

        totalSizeInBytes += channel.translations.size() * sizeof(channel.translations[0]) +
            channel.rotations.size() * sizeof(channel.rotations[0]) +
            channel.scales.size() * sizeof(channel.scales[0]) +
            channel.weights.size() * sizeof(channel.weights[0]);
      }

      if (verboseOutput) {
        fmt::printf(
            "\ranimation %d: %s (%d%%)",
            animIx,
            (const char*)animStackName,
            nodeIndex * 100 / nodeCount);
      }
    }

    raw.AddAnimation(animation);

    if (verboseOutput) {
      fmt::printf(
          "\ranimation %d: %s (%d channels, %3.1f MB)\n",
          animIx,
          (const char*)animStackName,
          (int)animation.channels.size(),
          (float)totalSizeInBytes * 1e-6f);
    }
  }
}

static std::string FindFileLoosely(
    const std::string& fbxFileName,
    const std::string& directory,
    const std::vector<std::string>& directoryFileList) {
  if (FileUtils::FileExists(fbxFileName)) {
    return fbxFileName;
  }

  // From e.g. C:/Assets/Texture.jpg, extract 'Texture.jpg'
  const std::string fileName = FileUtils::GetFileName(fbxFileName);

  // Try to find a match with extension.
  for (const auto& file : directoryFileList) {
    if (StringUtils::CompareNoCase(fileName, FileUtils::GetFileName(file)) == 0) {
      return directory + "/" + file;
    }
  }

  // Get the file name without file extension.
  const std::string fileBase = FileUtils::GetFileBase(fileName);

  // Try to find a match that ignores file extension
  for (const auto& file : directoryFileList) {
    if (StringUtils::CompareNoCase(fileBase, FileUtils::GetFileBase(file)) == 0) {
      return directory + "/" + file;
    }
  }

  return "";
}

/**
 * Try to locate the best match to the given texture filename, as provided in the FBX,
 * possibly searching through the provided folders for a reasonable-looking match.
 *
 * Returns empty string if no match can be found, else the absolute path of the file.
 **/
static std::string FindFbxTexture(
    const std::string& textureFileName,
    const std::vector<std::string>& folders,
    const std::vector<std::vector<std::string>>& folderContents) {
  // it might exist exactly as-is on the running machine's filesystem
  if (FileUtils::FileExists(textureFileName)) {
    return textureFileName;
  }
  // else look in other designated folders
  for (int ii = 0; ii < folders.size(); ii++) {
    const auto& fileLocation = FindFileLoosely(textureFileName, folders[ii], folderContents[ii]);
    if (!fileLocation.empty()) {
      return FileUtils::GetAbsolutePath(fileLocation);
    }
  }
  //Replace slashes with alternative platform version (e.g. '/' instead of '\\')
  std::string textureFileNameAltSlash = textureFileName;
  std::replace( textureFileNameAltSlash.begin(), textureFileNameAltSlash.end(), ALTERNATIVE_SLASH_CHAR, SLASH_CHAR);
  // finally look with alternative slashes
  for (int ii = 0; ii < folders.size(); ii++) {
    const auto& fileLocation = FindFileLoosely(textureFileNameAltSlash, folders[ii], folderContents[ii]);
    if (!fileLocation.empty()) {
      return FileUtils::GetAbsolutePath(fileLocation);
    }
  }
  return "";
}

/*
    The texture file names inside of the FBX often contain some long author-specific
    path with the wrong extensions. For instance, all of the art assets may be PSD
    files in the FBX metadata, but in practice they are delivered as TGA or PNG files.

    This function takes a texture file name stored in the FBX, which may be an absolute
    path on the author's computer such as "C:\MyProject\TextureName.psd", and matches
    it to a list of existing texture files in the same directory as the FBX file.
*/
static void FindFbxTextures(
    FbxScene* pScene,
    const std::string& fbxFileName,
    const std::set<std::string>& extensions,
    std::map<const FbxTexture*, FbxString>& textureLocations) {
  // figure out what folder the FBX file is in,
  const auto& fbxFolder = FileUtils::getFolder(fbxFileName);
  std::vector<std::string> folders{
      // first search filename.fbm folder which the SDK itself expands embedded textures into,
      fbxFolder + "/" + FileUtils::GetFileBase(fbxFileName) + ".fbm", // filename.fbm
      // then the FBX folder itself,
      fbxFolder,
      // then finally our working directory
      FileUtils::GetCurrentFolder(),
  };

  // List the contents of each of these folders (if they exist)
  std::vector<std::vector<std::string>> folderContents;
  for (const auto& folder : folders) {
    if (FileUtils::FolderExists(folder)) {
      folderContents.push_back(FileUtils::ListFolderFiles(folder, extensions));
    } else {
      folderContents.push_back({});
    }
  }

  // Try to match the FBX texture names with the actual files on disk.
  for (int i = 0; i < pScene->GetTextureCount(); i++) {
    const FbxFileTexture* pFileTexture = FbxCast<FbxFileTexture>(pScene->GetTexture(i));
    if (pFileTexture != nullptr) {
      const std::string fileLocation =
          FindFbxTexture(pFileTexture->GetFileName(), folders, folderContents);
      // always extend the mapping (even for files we didn't find)
      textureLocations.emplace(pFileTexture, fileLocation.c_str());
      if (fileLocation.empty()) {
        fmt::printf(
            "Warning: could not find a image file for texture: %s.\n", pFileTexture->GetName());
      } else if (verboseOutput) {
        fmt::printf("Found texture '%s' at: %s\n", pFileTexture->GetName(), fileLocation);
      }
    }
  }
}

bool LoadFBXFile(
    RawModel& raw,
    const std::string fbxFileName,
    const std::set<std::string>& textureExtensions,
    const GltfOptions& options) {
  std::string fbxFileNameU8 = NativeToUTF8(fbxFileName);
  FbxManager* pManager = FbxManager::Create();

  if (!options.fbxTempDir.empty()) {
    pManager->GetXRefManager().AddXRefProject("embeddedFileProject", options.fbxTempDir.c_str());
    FbxXRefManager::sEmbeddedFileProject = "embeddedFileProject";
    pManager->GetXRefManager().AddXRefProject("configurationProject", options.fbxTempDir.c_str());
    FbxXRefManager::sConfigurationProject = "configurationProject";
    pManager->GetXRefManager().AddXRefProject("localizationProject", options.fbxTempDir.c_str());
    FbxXRefManager::sLocalizationProject = "localizationProject";
    pManager->GetXRefManager().AddXRefProject("temporaryFileProject", options.fbxTempDir.c_str());
    FbxXRefManager::sTemporaryFileProject = "temporaryFileProject";
  }

  FbxIOSettings* pIoSettings = FbxIOSettings::Create(pManager, IOSROOT);
  pManager->SetIOSettings(pIoSettings);

  FbxImporter* pImporter = FbxImporter::Create(pManager, "");

  if (!pImporter->Initialize(fbxFileNameU8.c_str(), -1, pManager->GetIOSettings())) {
    if (verboseOutput) {
      fmt::printf("%s\n", pImporter->GetStatus().GetErrorString());
    }
    pImporter->Destroy();
    pManager->Destroy();
    return false;
  }

  FbxScene* pScene = FbxScene::Create(pManager, "fbxScene");
  pImporter->Import(pScene);
  pImporter->Destroy();

  if (pScene == nullptr) {
    pImporter->Destroy();
    pManager->Destroy();
    return false;
  }

  std::map<const FbxTexture*, FbxString> textureLocations;
  FindFbxTextures(pScene, fbxFileName, textureExtensions, textureLocations);

  // Use Y up for glTF
  FbxAxisSystem::MayaYUp.ConvertScene(pScene);

  // FBX's internal unscaled unit is centimetres, and if you choose not to work in that unit,
  // you will find scaling transforms on all the children of the root node. Those transforms are
  // superfluous and cause a lot of people a lot of trouble. Luckily we can get rid of them by
  // converting to CM here (which just gets rid of the scaling), and then we pre-multiply the
  // scale factor into every vertex position (and related attributes) instead.
  FbxSystemUnit sceneSystemUnit = pScene->GetGlobalSettings().GetSystemUnit();
  if (sceneSystemUnit != FbxSystemUnit::cm) {
    FbxSystemUnit::cm.ConvertScene(pScene);
  }
  // this is always 0.01, but let's opt for clarity.
  scaleFactor = FbxSystemUnit::m.GetConversionFactorFrom(FbxSystemUnit::cm);

  ReadNodeHierarchy(raw, pScene, pScene->GetRootNode(), 0, "");
  ReadNodeAttributes(raw, pScene, pScene->GetRootNode(), textureLocations);
  ReadAnimations(raw, pScene, options);

  pScene->Destroy();
  pManager->Destroy();

  return true;
}

// convenience method for describing a property in JSON
json TranscribeProperty(FbxProperty& prop) {
  using fbxsdk::EFbxType;
  std::string ename;

  // Convert property type
  switch (prop.GetPropertyDataType().GetType()) {
    case eFbxBool:
      ename = "eFbxBool";
      break;
    case eFbxChar:
      ename = "eFbxChar";
      break;
    case eFbxUChar:
      ename = "eFbxUChar";
      break;
    case eFbxShort:
      ename = "eFbxShort";
      break;
    case eFbxUShort:
      ename = "eFbxUShort";
      break;
    case eFbxInt:
      ename = "eFbxInt";
      break;
    case eFbxUInt:
      ename = "eFbxUint";
      break;
    case eFbxLongLong:
      ename = "eFbxLongLong";
      break;
    case eFbxULongLong:
      ename = "eFbxULongLong";
      break;
    case eFbxFloat:
      ename = "eFbxFloat";
      break;
    case eFbxHalfFloat:
      ename = "eFbxHalfFloat";
      break;
    case eFbxDouble:
      ename = "eFbxDouble";
      break;
    case eFbxDouble2:
      ename = "eFbxDouble2";
      break;
    case eFbxDouble3:
      ename = "eFbxDouble3";
      break;
    case eFbxDouble4:
      ename = "eFbxDouble4";
      break;
    case eFbxString:
      ename = "eFbxString";
      break;

      // Use this as fallback because it does not give very descriptive names
    default:
      ename = prop.GetPropertyDataType().GetName();
      break;
  }

  json p = {{"type", ename}};

  // Convert property value
  switch (prop.GetPropertyDataType().GetType()) {
    case eFbxBool:
    case eFbxChar:
    case eFbxUChar:
    case eFbxShort:
    case eFbxUShort:
    case eFbxInt:
    case eFbxUInt:
    case eFbxLongLong: {
      p["value"] = prop.EvaluateValue<long long>(FBXSDK_TIME_INFINITE);
      break;
    }
    case eFbxULongLong: {
      p["value"] = prop.EvaluateValue<unsigned long long>(FBXSDK_TIME_INFINITE);
      break;
    }
    case eFbxFloat:
    case eFbxHalfFloat:
    case eFbxDouble: {
      p["value"] = prop.EvaluateValue<double>(FBXSDK_TIME_INFINITE);
      break;
    }
    case eFbxDouble2: {
      auto v = prop.EvaluateValue<FbxDouble2>(FBXSDK_TIME_INFINITE);
      p["value"] = {v[0], v[1]};
      break;
    }
    case eFbxDouble3: {
      auto v = prop.EvaluateValue<FbxDouble3>(FBXSDK_TIME_INFINITE);
      p["value"] = {v[0], v[1], v[2]};
      break;
    }
    case eFbxDouble4: {
      auto v = prop.EvaluateValue<FbxDouble4>(FBXSDK_TIME_INFINITE);
      p["value"] = {v[0], v[1], v[2], v[3]};
      break;
    }
    case eFbxString: {
      p["value"] = std::string{prop.Get<FbxString>()};
      break;
    }
    default: {
      p["value"] = "UNSUPPORTED_VALUE_TYPE";
      break;
    }
  }

  return {{prop.GetNameAsCStr(), p}};
}
