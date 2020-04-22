/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RawModel.hpp"

#include <cmath>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(__unix__)
#include <algorithm>
#endif

#include "utils/Image_Utils.hpp"
#include "utils/String_Utils.hpp"

size_t VertexHasher::operator()(const RawVertex& v) const {
  size_t seed = 5381;
  const auto hasher = std::hash<float>{};
  seed ^= hasher(v.position[0]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  seed ^= hasher(v.position[1]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  seed ^= hasher(v.position[2]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  return seed;
}

bool RawVertex::operator==(const RawVertex& other) const {
  return (position == other.position) && (normal == other.normal) && (tangent == other.tangent) &&
      (binormal == other.binormal) && (color == other.color) && (uv0 == other.uv0) &&
      (uv1 == other.uv1) &&
      (jointWeights == other.jointWeights) && (jointIndices == other.jointIndices) &&
      (polarityUv0 == other.polarityUv0) &&
      (blendSurfaceIx == other.blendSurfaceIx) && (blends == other.blends);
}

size_t RawVertex::Difference(const RawVertex& other) const {
  size_t attributes = 0;
  if (position != other.position) {
    attributes |= RAW_VERTEX_ATTRIBUTE_POSITION;
  }
  if (normal != other.normal) {
    attributes |= RAW_VERTEX_ATTRIBUTE_NORMAL;
  }
  if (tangent != other.tangent) {
    attributes |= RAW_VERTEX_ATTRIBUTE_TANGENT;
  }
  if (binormal != other.binormal) {
    attributes |= RAW_VERTEX_ATTRIBUTE_BINORMAL;
  }
  if (color != other.color) {
    attributes |= RAW_VERTEX_ATTRIBUTE_COLOR;
  }
  if (uv0 != other.uv0) {
    attributes |= RAW_VERTEX_ATTRIBUTE_UV0;
  }
  if (uv1 != other.uv1) {
    attributes |= RAW_VERTEX_ATTRIBUTE_UV1;
  }
  // Always need both or neither.
  if (jointIndices != other.jointIndices || jointWeights != other.jointWeights) {
    attributes |= RAW_VERTEX_ATTRIBUTE_JOINT_INDICES | RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS;
  }
  return attributes;
}

RawModel::RawModel() : vertexAttributes(0){}

void RawModel::AddVertexAttribute(const RawVertexAttribute attrib) {
  vertexAttributes |= attrib;
}

int RawModel::AddVertex(const RawVertex& vertex) {
  auto it = vertexHash.find(vertex);
  if (it != vertexHash.end()) {
    return it->second;
  }
  vertexHash.emplace(vertex, (int)vertices.size());
  vertices.push_back(vertex);
  return (int)vertices.size() - 1;
}

int RawModel::AddTriangle(
    const int v0,
    const int v1,
    const int v2,
    const int materialIndex,
    const int surfaceIndex) {
  const RawTriangle triangle = {{v0, v1, v2}, materialIndex, surfaceIndex};
  triangles.push_back(triangle);
  return (int)triangles.size() - 1;
}

int RawModel::AddTexture(
    const std::string& name,
    const std::string& fileName,
    const std::string& fileLocation,
    RawTextureUsage usage) {
  if (name.empty()) {
    return -1;
  }
  for (size_t i = 0; i < textures.size(); i++) {
    // we allocate the struct even if the implementing image file is missing
    if (textures[i].usage == usage &&
        StringUtils::CompareNoCase(textures[i].fileLocation, fileLocation) == 0 &&
        StringUtils::CompareNoCase(textures[i].name, name) == 0) {
      return (int)i;
    }
  }

  const ImageUtils::ImageProperties properties = ImageUtils::GetImageProperties(
      !fileLocation.empty() ? fileLocation.c_str() : fileName.c_str());

  RawTexture texture;
  texture.name = name;
  texture.width = properties.width;
  texture.height = properties.height;
  texture.mipLevels =
      (int)ceilf(log2f(std::max((float)properties.width, (float)properties.height)));
  texture.usage = usage;
  texture.occlusion = (properties.occlusion == ImageUtils::IMAGE_TRANSPARENT)
      ? RAW_TEXTURE_OCCLUSION_TRANSPARENT
      : RAW_TEXTURE_OCCLUSION_OPAQUE;
  texture.fileName = fileName;
  texture.fileLocation = fileLocation;
  textures.emplace_back(texture);
  return (int)textures.size() - 1;
}

int RawModel::AddMaterial(const RawMaterial& material) {
  return AddMaterial(
      material.id,
      material.name.c_str(),
      material.type,
      material.textures,
      material.info,
      material.userProperties);
}

int RawModel::AddMaterial(
    const long id,
    const char* name,
    const RawMaterialType materialType,
    const int textures[RAW_TEXTURE_USAGE_MAX],
    std::shared_ptr<RawMatProps> materialInfo,
    const std::vector<std::string>& userProperties) {
  for (size_t i = 0; i < materials.size(); i++) {
    if (materials[i].name != name) {
      continue;
    }
    if (materials[i].type != materialType) {
      continue;
    }
    if (*(materials[i].info) != *materialInfo) {
      continue;
    }
    bool match = true;
    for (int j = 0; match && j < RAW_TEXTURE_USAGE_MAX; j++) {
      match = match && (materials[i].textures[j] == textures[j]);
    }
    if (materials[i].userProperties.size() != userProperties.size()) {
      match = false;
    } else {
      for (int j = 0; match && j < userProperties.size(); j++) {
        match = match && (materials[i].userProperties[j] == userProperties[j]);
      }
    }
    if (match) {
      return (int)i;
    }
  }

  RawMaterial material;
  material.id = id;
  material.name = name;
  material.type = materialType;
  material.info = materialInfo;
  material.userProperties = userProperties;

  for (int i = 0; i < RAW_TEXTURE_USAGE_MAX; i++) {
    material.textures[i] = textures[i];
  }

  materials.emplace_back(material);

  return (int)materials.size() - 1;
}

int RawModel::AddLight(
    const char* name,
    const RawLightType lightType,
    const Vec3f color,
    const float intensity,
    const float innerConeAngle,
    const float outerConeAngle) {
  for (size_t i = 0; i < lights.size(); i++) {
    if (lights[i].name != name || lights[i].type != lightType) {
      continue;
    }
    // only care about cone angles for spot
    if (lights[i].type == RAW_LIGHT_TYPE_SPOT) {
      if (lights[i].innerConeAngle != innerConeAngle ||
          lights[i].outerConeAngle != outerConeAngle) {
        continue;
      }
    }
    return (int)i;
  }
  RawLight light{
      name,
      lightType,
      color,
      intensity,
      innerConeAngle,
      outerConeAngle,
  };
  lights.push_back(light);
  return (int)lights.size() - 1;
}

int RawModel::AddSurface(const RawSurface& surface) {
  for (size_t i = 0; i < surfaces.size(); i++) {
    if (StringUtils::CompareNoCase(surfaces[i].name, surface.name) == 0) {
      return (int)i;
    }
  }

  surfaces.emplace_back(surface);
  return (int)(surfaces.size() - 1);
}

int RawModel::AddSurface(const char* name, const long surfaceId) {
  assert(name[0] != '\0');

  for (size_t i = 0; i < surfaces.size(); i++) {
    if (surfaces[i].id == surfaceId) {
      return (int)i;
    }
  }
  RawSurface surface;
  surface.id = surfaceId;
  surface.name = name;
  surface.bounds.Clear();
  surface.discrete = false;

  surfaces.emplace_back(surface);
  return (int)(surfaces.size() - 1);
}

int RawModel::AddAnimation(const RawAnimation& animation) {
  animations.emplace_back(animation);
  return (int)(animations.size() - 1);
}

int RawModel::AddNode(const RawNode& node) {
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].id == node.id) {
      return (int)i;
    }
  }

  nodes.emplace_back(node);
  return (int)nodes.size() - 1;
}

int RawModel::AddCameraPerspective(
    const char* name,
    const long nodeId,
    const float aspectRatio,
    const float fovDegreesX,
    const float fovDegreesY,
    const float nearZ,
    const float farZ) {
  RawCamera camera;
  camera.name = name;
  camera.nodeId = nodeId;
  camera.mode = RawCamera::CAMERA_MODE_PERSPECTIVE;
  camera.perspective.aspectRatio = aspectRatio;
  camera.perspective.fovDegreesX = fovDegreesX;
  camera.perspective.fovDegreesY = fovDegreesY;
  camera.perspective.nearZ = nearZ;
  camera.perspective.farZ = farZ;
  cameras.emplace_back(camera);
  return (int)cameras.size() - 1;
}

int RawModel::AddCameraOrthographic(
    const char* name,
    const long nodeId,
    const float magX,
    const float magY,
    const float nearZ,
    const float farZ) {
  RawCamera camera;
  camera.name = name;
  camera.nodeId = nodeId;
  camera.mode = RawCamera::CAMERA_MODE_ORTHOGRAPHIC;
  camera.orthographic.magX = magX;
  camera.orthographic.magY = magY;
  camera.orthographic.nearZ = nearZ;
  camera.orthographic.farZ = farZ;
  cameras.emplace_back(camera);
  return (int)cameras.size() - 1;
}

int RawModel::AddNode(const long id, const char* name, const long parentId) {
  assert(name[0] != '\0');

  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].id == id) {
      return (int)i;
    }
  }

  RawNode joint;
  joint.isJoint = false;
  joint.id = id;
  joint.name = name;
  joint.parentId = parentId;
  joint.surfaceId = 0;
  joint.lightIx = -1;
  joint.translation = Vec3f(0, 0, 0);
  joint.rotation = Quatf(0, 0, 0, 1);
  joint.scale = Vec3f(1, 1, 1);

  nodes.emplace_back(joint);
  return (int)nodes.size() - 1;
}

void RawModel::Condense(const int maxSkinningWeights, const bool normalizeWeights) {
  // Only keep surfaces that are referenced by one or more triangles.
  {
    std::vector<RawSurface> oldSurfaces = surfaces;

    surfaces.clear();

    std::set<int> survivingSurfaceIds;
    for (auto& triangle : triangles) {
      const RawSurface& surface = oldSurfaces[triangle.surfaceIndex];
      const int surfaceIndex = AddSurface(surface.name.c_str(), surface.id);
      surfaces[surfaceIndex] = surface;
      triangle.surfaceIndex = surfaceIndex;
      survivingSurfaceIds.emplace(surface.id);
    }
    // clear out references to meshes that no longer exist
    for (auto& node : nodes) {
      if (node.surfaceId != 0 &&
          survivingSurfaceIds.find(node.surfaceId) == survivingSurfaceIds.end()) {
        node.surfaceId = 0;
      }
    }
  }

  // Only keep materials that are referenced by one or more triangles.
  {
    std::vector<RawMaterial> oldMaterials = materials;

    materials.clear();

    for (auto& triangle : triangles) {
      const RawMaterial& material = oldMaterials[triangle.materialIndex];
      const int materialIndex = AddMaterial(material);
      materials[materialIndex] = material;
      triangle.materialIndex = materialIndex;
    }
  }

  // Only keep textures that are referenced by one or more materials.
  {
    std::vector<RawTexture> oldTextures = textures;

    textures.clear();

    for (auto& material : materials) {
      for (int j = 0; j < RAW_TEXTURE_USAGE_MAX; j++) {
        if (material.textures[j] >= 0) {
          const RawTexture& texture = oldTextures[material.textures[j]];
          const int textureIndex =
              AddTexture(texture.name, texture.fileName, texture.fileLocation, texture.usage);
          textures[textureIndex] = texture;
          material.textures[j] = textureIndex;
        }
      }
    }
  }

  // Only keep vertices that are referenced by one or more triangles.
  {
    std::vector<RawVertex> oldVertices = vertices;

    vertexHash.clear();
    vertices.clear();

    for (auto& triangle : triangles) {
      for (int j = 0; j < 3; j++) {
        triangle.verts[j] = AddVertex(oldVertices[triangle.verts[j]]);
      }
    }
  }

  {
    globalMaxWeights = 0;
    for (auto& vertex: vertices) {

      // Sort from largest to smallest weight.
      std::sort(vertex.skinningInfo.begin(), vertex.skinningInfo.end(), std::greater<RawVertexSkinningInfo>());
      
      // Reduce to fit the requirements.
      if (maxSkinningWeights < vertex.skinningInfo.size())
        vertex.skinningInfo.resize(maxSkinningWeights);
      globalMaxWeights = std::max(globalMaxWeights, (int) vertex.skinningInfo.size());

      // Normalize weights if requested.
      if (normalizeWeights) {
        float weightSum = 0;
        for (auto& jointWeight : vertex.skinningInfo)
          weightSum += jointWeight.jointWeight;
        const float weightSumRcp = 1.0 / weightSum;
        for (auto& jointWeight : vertex.skinningInfo)
          jointWeight.jointWeight *= weightSumRcp;
      }
    }

    if (globalMaxWeights > 0) {
      AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_JOINT_INDICES);
      AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS);
    }

    
    assert(globalMaxWeights >= 0);
    // Copy to gltf friendly structure
    for (auto& vertex : vertices) {
      vertex.jointIndices.reserve(globalMaxWeights);
      vertex.jointWeights.reserve(globalMaxWeights);
      for (int i = 0; i < globalMaxWeights; i += 4) { // ensure every vertex has the same amount of weights
        Vec4f weights{0.0};
        Vec4i jointIds{0,0,0,0};
        for (int j = i; j < i + 4 && j < vertex.skinningInfo.size(); j++) {
          weights[j - i] = vertex.skinningInfo[j].jointWeight;
          jointIds[j - i] = vertex.skinningInfo[j].jointIndex;
        }
        vertex.jointIndices.push_back(jointIds);
        vertex.jointWeights.push_back(weights);
      }
    }
  }
}

void RawModel::TransformGeometry(ComputeNormalsOption normals) {
  switch (normals) {
    case ComputeNormalsOption::NEVER:
      break;
    case ComputeNormalsOption::MISSING:
      if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_NORMAL) != 0) {
        break;
      }
      // otherwise fall through
    case ComputeNormalsOption::BROKEN:
    case ComputeNormalsOption::ALWAYS:
      size_t computedNormalsCount = this->CalculateNormals(normals == ComputeNormalsOption::BROKEN);
      vertexAttributes |= RAW_VERTEX_ATTRIBUTE_NORMAL;

      if (verboseOutput) {
        if (normals == ComputeNormalsOption::BROKEN) {
          if (computedNormalsCount > 0) {
            fmt::printf("Repaired %lu empty normals.\n", computedNormalsCount);
          }
        } else {
          fmt::printf("Computed %lu normals.\n", computedNormalsCount);
        }
      }
      break;
  }
}

void RawModel::TransformTextures(const std::vector<std::function<Vec2f(Vec2f)>>& transforms) {
  for (auto& vertice : vertices) {
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
      for (const auto& fun : transforms) {
        vertice.uv0 = fun(vertice.uv0);
      }
    }
    if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV1) != 0) {
      for (const auto& fun : transforms) {
        vertice.uv1 = fun(vertice.uv1);
      }
    }
  }
}

struct TriangleModelSortPos {
  static bool Compare(const RawTriangle& a, const RawTriangle& b) {
    if (a.materialIndex != b.materialIndex) {
      return a.materialIndex < b.materialIndex;
    }
    if (a.surfaceIndex != b.surfaceIndex) {
      return a.surfaceIndex < b.surfaceIndex;
    }
    return a.verts[0] < b.verts[0];
  }
};

struct TriangleModelSortNeg {
  static bool Compare(const RawTriangle& a, const RawTriangle& b) {
    if (a.materialIndex != b.materialIndex) {
      return a.materialIndex < b.materialIndex;
    }
    if (a.surfaceIndex != b.surfaceIndex) {
      return a.surfaceIndex < b.surfaceIndex;
    }
    return a.verts[0] > b.verts[0];
  }
};

void RawModel::CreateMaterialModels(
    std::vector<RawModel>& materialModels,
    bool shortIndices,
    const int keepAttribs,
    const bool forceDiscrete) const {
  // Sort all triangles based on material first, then surface, then first vertex index.
  std::vector<RawTriangle> sortedTriangles;

  bool invertedTransparencySort = true;
  if (invertedTransparencySort) {
    // Split the triangles into opaque and transparent triangles.
    std::vector<RawTriangle> opaqueTriangles;
    std::vector<RawTriangle> transparentTriangles;
    for (const auto& triangle : triangles) {
      const int materialIndex = triangle.materialIndex;
      if (materialIndex < 0) {
        opaqueTriangles.push_back(triangle);
        continue;
      }
      const int textureIndex = materials[materialIndex].textures[RAW_TEXTURE_USAGE_DIFFUSE];
      if (textureIndex < 0) {
        if (vertices[triangle.verts[0]].color.w < 1.0f ||
            vertices[triangle.verts[1]].color.w < 1.0f ||
            vertices[triangle.verts[2]].color.w < 1.0f) {
          transparentTriangles.push_back(triangle);
          continue;
        }
        opaqueTriangles.push_back(triangle);
        continue;
      }
      if (textures[textureIndex].occlusion == RAW_TEXTURE_OCCLUSION_TRANSPARENT) {
        transparentTriangles.push_back(triangle);
      } else {
        opaqueTriangles.push_back(triangle);
      }
    }

    // Sort the opaque triangles.
    std::sort(opaqueTriangles.begin(), opaqueTriangles.end(), TriangleModelSortPos::Compare);

    // Sort the transparent triangles in the reverse direction.
    std::sort(
        transparentTriangles.begin(), transparentTriangles.end(), TriangleModelSortNeg::Compare);

    // Add the triangles to the sorted list.
    for (const auto& opaqueTriangle : opaqueTriangles) {
      sortedTriangles.push_back(opaqueTriangle);
    }
    for (const auto& transparentTriangle : transparentTriangles) {
      sortedTriangles.push_back(transparentTriangle);
    }
  } else {
    sortedTriangles = triangles;
    std::sort(sortedTriangles.begin(), sortedTriangles.end(), TriangleModelSortPos::Compare);
  }

  // Overestimate the number of models that will be created to avoid massive reallocation.
  int discreteCount = 0;
  for (const auto& surface : surfaces) {
    discreteCount += surface.discrete ? 1 : 0;
  }

  materialModels.clear();
  materialModels.reserve(materials.size() + discreteCount);

  const RawVertex defaultVertex;

  // Create a separate model for each material.
  RawModel* model;
  for (size_t i = 0; i < sortedTriangles.size(); i++) {
    if (sortedTriangles[i].materialIndex < 0 || sortedTriangles[i].surfaceIndex < 0) {
      continue;
    }

    if (i == 0 || (shortIndices && model->GetVertexCount() >= 0xFFFE) ||
        sortedTriangles[i].materialIndex != sortedTriangles[i - 1].materialIndex ||
        (sortedTriangles[i].surfaceIndex != sortedTriangles[i - 1].surfaceIndex &&
         (forceDiscrete || surfaces[sortedTriangles[i].surfaceIndex].discrete ||
          surfaces[sortedTriangles[i - 1].surfaceIndex].discrete))) {
      materialModels.resize(materialModels.size() + 1);
      model = &materialModels[materialModels.size() - 1];
      model->globalMaxWeights = globalMaxWeights;
    }

    // FIXME: will have to unlink from the nodes, transform both surfaces into a
    // common space, and reparent to a new node with appropriate transform.

    const int prevSurfaceCount = model->GetSurfaceCount();
    const int materialIndex = model->AddMaterial(materials[sortedTriangles[i].materialIndex]);
    const int surfaceIndex = model->AddSurface(surfaces[sortedTriangles[i].surfaceIndex]);
    RawSurface& rawSurface = model->GetSurface(surfaceIndex);

    if (model->GetSurfaceCount() > prevSurfaceCount) {
      const std::vector<long>& jointIds = surfaces[sortedTriangles[i].surfaceIndex].jointIds;
      for (const auto& jointId : jointIds) {
        const int nodeIndex = GetNodeById(jointId);
        assert(nodeIndex != -1);
        model->AddNode(GetNode(nodeIndex));
      }
      rawSurface.bounds.Clear();
    }

    int verts[3];
    for (int j = 0; j < 3; j++) {
      RawVertex vertex = vertices[sortedTriangles[i].verts[j]];

      if (keepAttribs != -1) {
        int keep = keepAttribs;
        if ((keepAttribs & RAW_VERTEX_ATTRIBUTE_POSITION) != 0) {
          keep |= RAW_VERTEX_ATTRIBUTE_JOINT_INDICES | RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS;
        }
        if ((keepAttribs & RAW_VERTEX_ATTRIBUTE_AUTO) != 0) {
          keep |= RAW_VERTEX_ATTRIBUTE_POSITION;

          const RawMaterial& mat = model->GetMaterial(materialIndex);
          if (mat.textures[RAW_TEXTURE_USAGE_DIFFUSE] != -1) {
            keep |= RAW_VERTEX_ATTRIBUTE_UV0;
          }
          if (mat.textures[RAW_TEXTURE_USAGE_NORMAL] != -1) {
            keep |= RAW_VERTEX_ATTRIBUTE_NORMAL | RAW_VERTEX_ATTRIBUTE_TANGENT |
                RAW_VERTEX_ATTRIBUTE_BINORMAL | RAW_VERTEX_ATTRIBUTE_UV0;
          }
          if (mat.textures[RAW_TEXTURE_USAGE_SPECULAR] != -1) {
            keep |= RAW_VERTEX_ATTRIBUTE_NORMAL | RAW_VERTEX_ATTRIBUTE_UV0;
          }
          if (mat.textures[RAW_TEXTURE_USAGE_EMISSIVE] != -1) {
            keep |= RAW_VERTEX_ATTRIBUTE_UV1;
          }
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_POSITION) == 0) {
          vertex.position = defaultVertex.position;
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_NORMAL) == 0) {
          vertex.normal = defaultVertex.normal;
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_TANGENT) == 0) {
          vertex.tangent = defaultVertex.tangent;
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_BINORMAL) == 0) {
          vertex.binormal = defaultVertex.binormal;
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_COLOR) == 0) {
          vertex.color = defaultVertex.color;
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_UV0) == 0) {
          vertex.uv0 = defaultVertex.uv0;
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_UV1) == 0) {
          vertex.uv1 = defaultVertex.uv1;
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_JOINT_INDICES) == 0) {
          vertex.jointIndices = defaultVertex.jointIndices;
        }
        if ((keep & RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS) == 0) {
          vertex.jointWeights = defaultVertex.jointWeights;
        }
      }

      verts[j] = model->AddVertex(vertex);
      model->vertexAttributes |= vertex.Difference(defaultVertex);

      rawSurface.bounds.AddPoint(vertex.position);
    }

    model->AddTriangle(verts[0], verts[1], verts[2], materialIndex, surfaceIndex);
  }
}

int RawModel::GetNodeById(const long nodeId) const {
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].id == nodeId) {
      return (int)i;
    }
  }
  return -1;
}

int RawModel::GetSurfaceById(const long surfaceId) const {
  for (size_t i = 0; i < surfaces.size(); i++) {
    if (surfaces[i].id == surfaceId) {
      return (int)i;
    }
  }
  return -1;
}

Vec3f RawModel::getFaceNormal(int verts[3]) const {
  const float l0 = (vertices[verts[1]].position - vertices[verts[0]].position).LengthSquared();
  const float l1 = (vertices[verts[2]].position - vertices[verts[1]].position).LengthSquared();
  const float l2 = (vertices[verts[0]].position - vertices[verts[2]].position).LengthSquared();
  const int index = (l0 > l1) ? (l0 > l2 ? 2 : 1) : (l1 > l2 ? 0 : 1);

  const Vec3f e0 = vertices[verts[(index + 1) % 3]].position - vertices[verts[index]].position;
  const Vec3f e1 = vertices[verts[(index + 2) % 3]].position - vertices[verts[index]].position;
  if (e0.LengthSquared() < FLT_MIN || e1.LengthSquared() < FLT_MIN) {
    return Vec3f{0.0f};
  }
  auto result = Vec3f::CrossProduct(e0, e1);
  auto resultLengthSquared = result.LengthSquared();
  if (resultLengthSquared < FLT_MIN) {
    return Vec3f{0.0f};
  }
  float edgeDot = std::max(-1.0f, std::min(1.0f, Vec3f::DotProduct(e0, e1)));
  float angle = acos(edgeDot);
  float area = resultLengthSquared / 2.0f;
  return result.Normalized() * angle * area;
}

size_t RawModel::CalculateNormals(bool onlyBroken) {
  Vec3f averagePos = Vec3f{0.0f};
  std::set<int> brokenVerts;
  for (int vertIx = 0; vertIx < vertices.size(); vertIx++) {
    RawVertex& vertex = vertices[vertIx];
    averagePos += (vertex.position / (float)vertices.size());
    if (onlyBroken && (vertex.normal.LengthSquared() >= FLT_MIN)) {
      continue;
    }
    vertex.normal = Vec3f{0.0f};
    if (onlyBroken) {
      brokenVerts.emplace(vertIx);
    }
  }

  for (auto& triangle : triangles) {
    bool relevant = false;
    for (int vertIx : triangle.verts) {
      relevant |= (brokenVerts.count(vertIx) > 0);
    }
    if (!relevant) {
      continue;
    }
    Vec3f faceNormal = this->getFaceNormal(triangle.verts);
    for (int vertIx : triangle.verts) {
      if (!onlyBroken || brokenVerts.count(vertIx) > 0) {
        vertices[vertIx].normal += faceNormal;
      }
    }
  }

  for (int vertIx = 0; vertIx < vertices.size(); vertIx++) {
    if (onlyBroken && brokenVerts.count(vertIx) == 0) {
      continue;
    }
    RawVertex& vertex = vertices[vertIx];
    if (vertex.normal.LengthSquared() < FLT_MIN) {
      vertex.normal = vertex.position - averagePos;
      if (vertex.normal.LengthSquared() < FLT_MIN) {
        vertex.normal = Vec3f{0.0f, 1.0f, 0.0f};
        continue;
      }
    }
    vertex.normal.Normalize();
  }
  return onlyBroken ? brokenVerts.size() : vertices.size();
}
