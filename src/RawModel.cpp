/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <map>

#if defined( __unix__ )
#include <algorithm>
#endif

#include "FBX2glTF.h"
#include "utils/String_Utils.h"
#include "utils/Image_Utils.h"
#include "RawModel.h"

bool RawVertex::operator==(const RawVertex &other) const
{
    return (position == other.position) &&
           (normal == other.normal) &&
           (tangent == other.tangent) &&
           (binormal == other.binormal) &&
           (color == other.color) &&
           (uv0 == other.uv0) &&
           (uv1 == other.uv1) &&
           (jointIndices == other.jointIndices) &&
           (jointWeights == other.jointWeights) &&
           (polarityUv0 == other.polarityUv0) &&
           (blendSurfaceIx == other.blendSurfaceIx) &&
           (blends == other.blends);
}

size_t RawVertex::Difference(const RawVertex &other) const
{
    size_t attributes = 0;
    if (position != other.position) { attributes |= RAW_VERTEX_ATTRIBUTE_POSITION; }
    if (normal != other.normal) { attributes |= RAW_VERTEX_ATTRIBUTE_NORMAL; }
    if (tangent != other.tangent) { attributes |= RAW_VERTEX_ATTRIBUTE_TANGENT; }
    if (binormal != other.binormal) { attributes |= RAW_VERTEX_ATTRIBUTE_BINORMAL; }
    if (color != other.color) { attributes |= RAW_VERTEX_ATTRIBUTE_COLOR; }
    if (uv0 != other.uv0) { attributes |= RAW_VERTEX_ATTRIBUTE_UV0; }
    if (uv1 != other.uv1) { attributes |= RAW_VERTEX_ATTRIBUTE_UV1; }
    // Always need both or neither.
    if (jointIndices != other.jointIndices) { attributes |= RAW_VERTEX_ATTRIBUTE_JOINT_INDICES | RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS; }
    if (jointWeights != other.jointWeights) { attributes |= RAW_VERTEX_ATTRIBUTE_JOINT_INDICES | RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS; }
    return attributes;
}

RawModel::RawModel()
    : vertexAttributes(0)
{
}

void RawModel::AddVertexAttribute(const RawVertexAttribute attrib)
{
    vertexAttributes |= attrib;
}

int RawModel::AddVertex(const RawVertex &vertex)
{
    auto it = vertexHash.find(vertex);
    if (it != vertexHash.end()) {
        return it->second;
    }
    vertexHash.emplace(vertex, (int) vertices.size());
    vertices.push_back(vertex);
    return (int) vertices.size() - 1;
}

int RawModel::AddTriangle(const int v0, const int v1, const int v2, const int materialIndex, const int surfaceIndex)
{
    const RawTriangle triangle = {{v0, v1, v2}, materialIndex, surfaceIndex};
    triangles.push_back(triangle);
    return (int) triangles.size() - 1;
}

int RawModel::AddTexture(const std::string &name, const std::string &fileName, const std::string &fileLocation, RawTextureUsage usage)
{
    if (name.empty()) {
        return -1;
    }
    for (size_t i = 0; i < textures.size(); i++) {
        if (Gltf::StringUtils::CompareNoCase(textures[i].name, name) == 0 && textures[i].usage == usage) {
            return (int) i;
        }
    }

    const ImageProperties properties = GetImageProperties(!fileLocation.empty() ? fileLocation.c_str() : fileName.c_str());

    RawTexture texture;
    texture.name         = name;
    texture.width        = properties.width;
    texture.height       = properties.height;
    texture.mipLevels    = (int) ceilf(log2f(std::max((float) properties.width, (float) properties.height)));
    texture.usage        = usage;
    texture.occlusion    = (properties.occlusion == IMAGE_TRANSPARENT) ?
                           RAW_TEXTURE_OCCLUSION_TRANSPARENT : RAW_TEXTURE_OCCLUSION_OPAQUE;
    texture.fileName     = fileName;
    texture.fileLocation = fileLocation;
    textures.emplace_back(texture);
    return (int) textures.size() - 1;
}

int RawModel::AddMaterial(const RawMaterial &material)
{
    return AddMaterial(
        material.name.c_str(), material.shadingModel.c_str(), material.type, material.textures, material.ambientFactor,
        material.diffuseFactor, material.specularFactor, material.emissiveFactor, material.shininess);
}

int RawModel::AddMaterial(
    const char *name, const char *shadingModel, const RawMaterialType materialType,
    const int textures[RAW_TEXTURE_USAGE_MAX], const Vec3f ambientFactor,
    const Vec4f diffuseFactor, const Vec3f specularFactor,
    const Vec3f emissiveFactor, float shinineness)
{
    for (size_t i = 0; i < materials.size(); i++) {
        if (materials[i].name != name) {
            continue;
        }
        if (materials[i].shadingModel != shadingModel) {
            continue;
        }
        if (materials[i].type != materialType) {
            continue;
        }
        if (materials[i].ambientFactor != ambientFactor ||
            materials[i].diffuseFactor != diffuseFactor ||
            materials[i].specularFactor != specularFactor ||
            materials[i].emissiveFactor != emissiveFactor ||
            materials[i].shininess != shinineness) {
            continue;
        }

        bool match = true;
        for (int j = 0; match && j < RAW_TEXTURE_USAGE_MAX; j++) {
            match = match && (materials[i].textures[j] == textures[j]);
        }
        if (match) {
            return (int) i;
        }
    }

    RawMaterial material;
    material.name           = name;
    material.shadingModel   = shadingModel;
    material.type           = materialType;
    material.ambientFactor  = ambientFactor;
    material.diffuseFactor  = diffuseFactor;
    material.specularFactor = specularFactor;
    material.emissiveFactor = emissiveFactor;
    material.shininess      = shinineness;

    for (int i = 0; i < RAW_TEXTURE_USAGE_MAX; i++) {
        material.textures[i] = textures[i];
    }

    materials.emplace_back(material);

    return (int) materials.size() - 1;
}

int RawModel::AddSurface(const RawSurface &surface)
{
    for (size_t i = 0; i < surfaces.size(); i++) {
        if (Gltf::StringUtils::CompareNoCase(surfaces[i].name, surface.name) == 0) {
            return (int) i;
        }
    }

    surfaces.emplace_back(surface);
    return (int) (surfaces.size() - 1);
}

int RawModel::AddSurface(const char *name, const long surfaceId)
{
    assert(name[0] != '\0');

    for (size_t i = 0; i < surfaces.size(); i++) {
        if (surfaces[i].id == surfaceId) {
            return (int) i;
        }
    }
    RawSurface  surface;
    surface.id = surfaceId;
    surface.name     = name;
    surface.bounds.Clear();
    surface.discrete  = false;

    surfaces.emplace_back(surface);
    return (int) (surfaces.size() - 1);
}

int RawModel::AddAnimation(const RawAnimation &animation)
{
    animations.emplace_back(animation);
    return (int) (animations.size() - 1);
}

int RawModel::AddNode(const RawNode &node)
{
    for (size_t i = 0; i < nodes.size(); i++) {
        if (Gltf::StringUtils::CompareNoCase(nodes[i].name.c_str(), node.name) == 0) {
            return (int) i;
        }
    }

    nodes.emplace_back(node);
    return (int) nodes.size() - 1;
}

int RawModel::AddCameraPerspective(
    const char *name, const char *nodeName, const float aspectRatio, const float fovDegreesX, const float fovDegreesY, const float nearZ,
    const float farZ)
{
    RawCamera camera;
    camera.name                    = name;
    camera.nodeName                = nodeName;
    camera.mode                    = RawCamera::CAMERA_MODE_PERSPECTIVE;
    camera.perspective.aspectRatio = aspectRatio;
    camera.perspective.fovDegreesX = fovDegreesX;
    camera.perspective.fovDegreesY = fovDegreesY;
    camera.perspective.nearZ       = nearZ;
    camera.perspective.farZ        = farZ;
    cameras.emplace_back(camera);
    return (int) cameras.size() - 1;
}

int RawModel::AddCameraOrthographic(
    const char *name, const char *nodeName, const float magX, const float magY, const float nearZ, const float farZ)
{
    RawCamera camera;
    camera.name               = name;
    camera.nodeName           = nodeName;
    camera.mode               = RawCamera::CAMERA_MODE_ORTHOGRAPHIC;
    camera.orthographic.magX  = magX;
    camera.orthographic.magY  = magY;
    camera.orthographic.nearZ = nearZ;
    camera.orthographic.farZ  = farZ;
    cameras.emplace_back(camera);
    return (int) cameras.size() - 1;
}

int RawModel::AddNode(const char *name, const char *parentName)
{
    assert(name[0] != '\0');

    for (size_t i = 0; i < nodes.size(); i++) {
        if (Gltf::StringUtils::CompareNoCase(nodes[i].name, name) == 0) {
            return (int) i;
        }
    }

    RawNode joint;
    joint.isJoint     = false;
    joint.name        = name;
    joint.parentName  = parentName;
    joint.surfaceId   = 0;
    joint.translation = Vec3f(0, 0, 0);
    joint.rotation    = Quatf(0, 0, 0, 1);
    joint.scale       = Vec3f(1, 1, 1);

    nodes.emplace_back(joint);
    return (int) nodes.size() - 1;
}

void RawModel::Condense()
{
    // Only keep surfaces that are referenced by one or more triangles.
    {
        std::vector<RawSurface> oldSurfaces = surfaces;

        surfaces.clear();

        for (auto &triangle : triangles) {
            const RawSurface &surface     = oldSurfaces[triangle.surfaceIndex];
            const int        surfaceIndex = AddSurface(surface.name.c_str(), surface.id);
            surfaces[surfaceIndex] = surface;
            triangle.surfaceIndex = surfaceIndex;
        }
    }

    // Only keep materials that are referenced by one or more triangles.
    {
        std::vector<RawMaterial> oldMaterials = materials;

        materials.clear();

        for (auto &triangle : triangles) {
            const RawMaterial &material     = oldMaterials[triangle.materialIndex];
            const int         materialIndex = AddMaterial(material);
            materials[materialIndex] = material;
            triangle.materialIndex = materialIndex;
        }
    }

    // Only keep textures that are referenced by one or more materials.
    {
        std::vector<RawTexture> oldTextures = textures;

        textures.clear();

        for (auto &material : materials) {
            for (int j = 0; j < RAW_TEXTURE_USAGE_MAX; j++) {
                if (material.textures[j] >= 0) {
                    const RawTexture &texture = oldTextures[material.textures[j]];
                    const int textureIndex = AddTexture(texture.name, texture.fileName, texture.fileLocation, texture.usage);
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

        for (auto &triangle : triangles) {
            for (int j = 0; j < 3; j++) {
                triangle.verts[j] = AddVertex(oldVertices[triangle.verts[j]]);
            }
        }
    }
}

void RawModel::TransformTextures(const std::vector<std::function<Vec2f(Vec2f)>> &transforms)
{
    for (auto &vertice : vertices) {
        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
            for (const auto &fun : transforms) {
                vertice.uv0 = fun(vertice.uv0);
            }
        }
        if ((vertexAttributes & RAW_VERTEX_ATTRIBUTE_UV1) != 0) {
            for (const auto &fun : transforms) {
                vertice.uv1 = fun(vertice.uv1);
            }
        }
    }
}

struct TriangleModelSortPos
{
    static bool Compare(const RawTriangle &a, const RawTriangle &b)
    {
        if (a.materialIndex != b.materialIndex) {
            return a.materialIndex < b.materialIndex;
        }
        if (a.surfaceIndex != b.surfaceIndex) {
            return a.surfaceIndex < b.surfaceIndex;
        }
        return a.verts[0] < b.verts[0];
    }
};

struct TriangleModelSortNeg
{
    static bool Compare(const RawTriangle &a, const RawTriangle &b)
    {
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
    std::vector<RawModel> &materialModels, const int maxModelVertices, const int keepAttribs, const bool forceDiscrete) const
{
    // Sort all triangles based on material first, then surface, then first vertex index.
    std::vector<RawTriangle> sortedTriangles;

    bool invertedTransparencySort = true;
    if (invertedTransparencySort) {
        // Split the triangles into opaque and transparent triangles.
        std::vector<RawTriangle> opaqueTriangles;
        std::vector<RawTriangle> transparentTriangles;
        for (const auto &triangle : triangles) {
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
        std::sort(transparentTriangles.begin(), transparentTriangles.end(), TriangleModelSortNeg::Compare);

        // Add the triangles to the sorted list.
        for (const auto &opaqueTriangle : opaqueTriangles) {
            sortedTriangles.push_back(opaqueTriangle);
        }
        for (const auto &transparentTriangle : transparentTriangles) {
            sortedTriangles.push_back(transparentTriangle);
        }
    } else {
        sortedTriangles = triangles;
        std::sort(sortedTriangles.begin(), sortedTriangles.end(), TriangleModelSortPos::Compare);
    }

    // Overestimate the number of models that will be created to avoid massive reallocation.
    int discreteCount = 0;
    for (const auto &surface : surfaces) {
        discreteCount += (surface.discrete != false);
    }

    materialModels.clear();
    materialModels.reserve(materials.size() + discreteCount);

    const RawVertex defaultVertex;

    // Create a separate model for each material.
    RawModel *model;
    for (size_t i = 0; i < sortedTriangles.size(); i++) {

        if (sortedTriangles[i].materialIndex < 0 || sortedTriangles[i].surfaceIndex < 0) {
            continue;
        }

        if (i == 0 ||
            model->GetVertexCount() > maxModelVertices - 3 ||
            sortedTriangles[i].materialIndex != sortedTriangles[i - 1].materialIndex ||
            (sortedTriangles[i].surfaceIndex != sortedTriangles[i - 1].surfaceIndex &&
                (forceDiscrete || surfaces[sortedTriangles[i].surfaceIndex].discrete ||
                    surfaces[sortedTriangles[i - 1].surfaceIndex].discrete))) {
            materialModels.resize(materialModels.size() + 1);
            model = &materialModels[materialModels.size() - 1];
        }

        // FIXME: will have to unlink from the nodes, transform both surfaces into a
        // common space, and reparent to a new node with appropriate transform.

        const int prevSurfaceCount = model->GetSurfaceCount();
        const int materialIndex    = model->AddMaterial(materials[sortedTriangles[i].materialIndex]);
        const int surfaceIndex     = model->AddSurface(surfaces[sortedTriangles[i].surfaceIndex]);
        RawSurface &rawSurface = model->GetSurface(surfaceIndex);

        if (model->GetSurfaceCount() > prevSurfaceCount) {
            const std::vector<std::string> &jointNames = surfaces[sortedTriangles[i].surfaceIndex].jointNames;
            for (const auto &jointName : jointNames) {
                const int nodeIndex = GetNodeByName(jointName.c_str());
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

                    const RawMaterial &mat = model->GetMaterial(materialIndex);
                    if (mat.textures[RAW_TEXTURE_USAGE_DIFFUSE] != -1) {
                        keep |= RAW_VERTEX_ATTRIBUTE_UV0;
                    }
                    if (mat.textures[RAW_TEXTURE_USAGE_NORMAL] != -1) {
                        keep |= RAW_VERTEX_ATTRIBUTE_NORMAL |
                                RAW_VERTEX_ATTRIBUTE_TANGENT |
                                RAW_VERTEX_ATTRIBUTE_BINORMAL |
                                RAW_VERTEX_ATTRIBUTE_UV0;
                    }
                    if (mat.textures[RAW_TEXTURE_USAGE_SPECULAR] != -1) {
                        keep |= RAW_VERTEX_ATTRIBUTE_NORMAL |
                                RAW_VERTEX_ATTRIBUTE_UV0;
                    }
                    if (mat.textures[RAW_TEXTURE_USAGE_EMISSIVE] != -1) {
                        keep |= RAW_VERTEX_ATTRIBUTE_UV1;
                    }
                }
                if ((keep & RAW_VERTEX_ATTRIBUTE_POSITION) == 0) { vertex.position = defaultVertex.position; }
                if ((keep & RAW_VERTEX_ATTRIBUTE_NORMAL) == 0) { vertex.normal = defaultVertex.normal; }
                if ((keep & RAW_VERTEX_ATTRIBUTE_TANGENT) == 0) { vertex.tangent = defaultVertex.tangent; }
                if ((keep & RAW_VERTEX_ATTRIBUTE_BINORMAL) == 0) { vertex.binormal = defaultVertex.binormal; }
                if ((keep & RAW_VERTEX_ATTRIBUTE_COLOR) == 0) { vertex.color = defaultVertex.color; }
                if ((keep & RAW_VERTEX_ATTRIBUTE_UV0) == 0) { vertex.uv0 = defaultVertex.uv0; }
                if ((keep & RAW_VERTEX_ATTRIBUTE_UV1) == 0) { vertex.uv1 = defaultVertex.uv1; }
                if ((keep & RAW_VERTEX_ATTRIBUTE_JOINT_INDICES) == 0) { vertex.jointIndices = defaultVertex.jointIndices; }
                if ((keep & RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS) == 0) { vertex.jointWeights = defaultVertex.jointWeights; }
            }

            verts[j] = model->AddVertex(vertex);
            model->vertexAttributes |= vertex.Difference(defaultVertex);

            rawSurface.bounds.AddPoint(vertex.position);
        }

        model->AddTriangle(verts[0], verts[1], verts[2], materialIndex, surfaceIndex);
    }
}

int RawModel::GetNodeByName(const char *name) const
{
    for (size_t i = 0; i < nodes.size(); i++) {
        if (nodes[i].name == name) {
            return (int) i;
        }
    }
    return -1;
}

int RawModel::GetSurfaceById(const long surfaceId) const
{
    for (size_t i = 0; i < surfaces.size(); i++) {
        if (surfaces[i].id == surfaceId) {
            return (int)i;
        }
    }
    return -1;
}
