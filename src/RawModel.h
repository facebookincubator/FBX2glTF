/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __RAWMODEL_H__
#define __RAWMODEL_H__

#include <unordered_map>
#include <functional>

enum RawVertexAttribute
{
    RAW_VERTEX_ATTRIBUTE_POSITION      = 1 << 0,
    RAW_VERTEX_ATTRIBUTE_NORMAL        = 1 << 1,
    RAW_VERTEX_ATTRIBUTE_TANGENT       = 1 << 2,
    RAW_VERTEX_ATTRIBUTE_BINORMAL      = 1 << 3,
    RAW_VERTEX_ATTRIBUTE_COLOR         = 1 << 4,
    RAW_VERTEX_ATTRIBUTE_UV0           = 1 << 5,
    RAW_VERTEX_ATTRIBUTE_UV1           = 1 << 6,
    RAW_VERTEX_ATTRIBUTE_JOINT_INDICES = 1 << 7,
    RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS = 1 << 8,

    RAW_VERTEX_ATTRIBUTE_AUTO = 1 << 31
};

struct RawBlendVertex
{
    Vec3f position {};
    Vec3f normal {};
    Vec4f tangent {};

    bool operator==(const RawBlendVertex &other) const {
        return position == other.position &&
               normal == other.normal &&
               tangent == other.tangent;
    }
};

struct RawVertex
{
    RawVertex() :
        polarityUv0(false),
        pad1(false),
        pad2(false),
        pad3(false) {}

    Vec3f position { 0.0f };
    Vec3f normal { 0.0f };
    Vec3f binormal { 0.0f };
    Vec4f tangent { 0.0f };
    Vec4f color { 0.0f };
    Vec2f uv0 { 0.0f };
    Vec2f uv1 { 0.0f };
    Vec4i jointIndices { 0, 0, 0, 0 };
    Vec4f jointWeights { 0.0f };
    // end of members that directly correspond to vertex attributes

    // if this vertex participates in a blend shape setup, the surfaceIx of its dedicated mesh; otherwise, -1
    int blendSurfaceIx = -1;
    // the size of this vector is always identical to the size of the corresponding RawSurface.blendChannels
    std::vector<RawBlendVertex> blends { };

    bool polarityUv0;
    bool pad1;
    bool pad2;
    bool pad3;

    bool operator==(const RawVertex &other) const;
    size_t Difference(const RawVertex &other) const;
};

class VertexHasher
{
public:
    size_t operator()(const RawVertex &v) const
    {
        size_t seed = 5381;
        const auto hasher = std::hash<float>{};
        seed ^= hasher(v.position[0]) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= hasher(v.position[1]) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= hasher(v.position[2]) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        return seed;
    }
};

struct RawTriangle
{
    int verts[3];
    int materialIndex;
    int surfaceIndex;
};

enum RawShadingModel
{
    RAW_SHADING_MODEL_UNKNOWN = -1,
    RAW_SHADING_MODEL_CONSTANT,
    RAW_SHADING_MODEL_LAMBERT,
    RAW_SHADING_MODEL_BLINN,
    RAW_SHADING_MODEL_PHONG,
    RAW_SHADING_MODEL_PBR_MET_ROUGH,
    RAW_SHADING_MODEL_MAX
};

static inline std::string Describe(RawShadingModel model) {
    switch(model) {
        case RAW_SHADING_MODEL_UNKNOWN:         return "<unknown>";
        case RAW_SHADING_MODEL_CONSTANT:        return "Constant";
        case RAW_SHADING_MODEL_LAMBERT:         return "Lambert";
        case RAW_SHADING_MODEL_BLINN:           return "Blinn";
        case RAW_SHADING_MODEL_PHONG:           return "Phong";
        case RAW_SHADING_MODEL_PBR_MET_ROUGH:   return "Metallic/Roughness";
        case RAW_SHADING_MODEL_MAX: default:    return "<unknown>";
    }
}

enum RawTextureUsage
{
    RAW_TEXTURE_USAGE_NONE = -1,
    RAW_TEXTURE_USAGE_AMBIENT,
    RAW_TEXTURE_USAGE_DIFFUSE,
    RAW_TEXTURE_USAGE_NORMAL,
    RAW_TEXTURE_USAGE_SPECULAR,
    RAW_TEXTURE_USAGE_SHININESS,
    RAW_TEXTURE_USAGE_EMISSIVE,
    RAW_TEXTURE_USAGE_REFLECTION,
    RAW_TEXTURE_USAGE_ALBEDO,
    RAW_TEXTURE_USAGE_OCCLUSION,
    RAW_TEXTURE_USAGE_ROUGHNESS,
    RAW_TEXTURE_USAGE_METALLIC,
    RAW_TEXTURE_USAGE_MAX
};

static inline std::string Describe(RawTextureUsage usage)
{
    switch (usage) {
        case RAW_TEXTURE_USAGE_NONE:        return "<none>";
        case RAW_TEXTURE_USAGE_AMBIENT:     return "ambient";
        case RAW_TEXTURE_USAGE_DIFFUSE:     return "diffuse";
        case RAW_TEXTURE_USAGE_NORMAL:      return "normal";
        case RAW_TEXTURE_USAGE_SPECULAR:    return "specuar";
        case RAW_TEXTURE_USAGE_SHININESS:   return "shininess";
        case RAW_TEXTURE_USAGE_EMISSIVE:    return "emissive";
        case RAW_TEXTURE_USAGE_REFLECTION:  return "reflection";
        case RAW_TEXTURE_USAGE_OCCLUSION:   return "occlusion";
        case RAW_TEXTURE_USAGE_ROUGHNESS:   return "roughness";
        case RAW_TEXTURE_USAGE_METALLIC:    return "metallic";
        case RAW_TEXTURE_USAGE_MAX:default: return "unknown";
    }
};

enum RawTextureOcclusion
{
    RAW_TEXTURE_OCCLUSION_OPAQUE,
    RAW_TEXTURE_OCCLUSION_TRANSPARENT
};

struct RawTexture
{
    std::string         name;           // logical name in FBX file
    int                 width;
    int                 height;
    int                 mipLevels;
    RawTextureUsage     usage;
    RawTextureOcclusion occlusion;
    std::string         fileName;       // original filename in FBX file
    std::string         fileLocation;   // inferred path in local filesystem, or ""
};

enum RawMaterialType
{
    RAW_MATERIAL_TYPE_OPAQUE,
    RAW_MATERIAL_TYPE_TRANSPARENT,
    RAW_MATERIAL_TYPE_SKINNED_OPAQUE,
    RAW_MATERIAL_TYPE_SKINNED_TRANSPARENT,
};

struct RawMatProps {
    explicit RawMatProps(RawShadingModel shadingModel)
        : shadingModel(shadingModel)
    {}
    const RawShadingModel shadingModel;

    virtual bool operator!=(const RawMatProps &other) const { return !(*this == other); }
    virtual bool operator==(const RawMatProps &other) const { return shadingModel == other.shadingModel; };
};

struct RawTraditionalMatProps : RawMatProps {
    RawTraditionalMatProps(
        RawShadingModel shadingModel,
        const Vec3f &&ambientFactor,
        const Vec4f &&diffuseFactor,
        const Vec3f &&emissiveFactor,
        const Vec3f &&specularFactor,
        const float shininess
    ) : RawMatProps(shadingModel),
          ambientFactor(ambientFactor),
          diffuseFactor(diffuseFactor),
          emissiveFactor(emissiveFactor),
          specularFactor(specularFactor),
          shininess(shininess)
    {}

    const Vec3f ambientFactor;
    const Vec4f diffuseFactor;
    const Vec3f emissiveFactor;
    const Vec3f specularFactor;
    const float shininess;

    bool operator==(const RawMatProps &other) const override {
        if (RawMatProps::operator==(other)) {
            const auto &typed = (RawTraditionalMatProps &) other;
            return ambientFactor == typed.ambientFactor &&
                diffuseFactor == typed.diffuseFactor &&
                specularFactor == typed.specularFactor &&
                emissiveFactor == typed.emissiveFactor &&
                shininess == typed.shininess;
        }
        return false;
    }
};

struct RawMetRoughMatProps : RawMatProps {
    RawMetRoughMatProps(
        RawShadingModel shadingModel,
        const Vec4f &&diffuseFactor,
        const Vec3f &&emissiveFactor,
        float emissiveIntensity,
        float metallic,
        float roughness
    ) : RawMatProps(shadingModel),
      diffuseFactor(diffuseFactor),
      emissiveFactor(emissiveFactor),
      emissiveIntensity(emissiveIntensity),
      metallic(metallic),
      roughness(roughness)
    {}
    const Vec4f diffuseFactor;
    const Vec3f emissiveFactor;
    const float emissiveIntensity;
    const float metallic;
    const float roughness;

    bool operator==(const RawMatProps &other) const override {
        if (RawMatProps::operator==(other)) {
            const auto &typed = (RawMetRoughMatProps &) other;
            return diffuseFactor == typed.diffuseFactor &&
            emissiveFactor == typed.emissiveFactor &&
            emissiveIntensity == typed.emissiveIntensity &&
            metallic == typed.metallic &&
            roughness == typed.roughness;
        }
        return false;
    }
};

struct RawMaterial
{
    std::string                  name;
    RawMaterialType              type;
    std::shared_ptr<RawMatProps> info;
    int                          textures[RAW_TEXTURE_USAGE_MAX];
};

struct RawBlendChannel
{
    float defaultDeform;
    bool hasNormals;
    bool hasTangents;
};

struct RawSurface
{
    long                         id;
    std::string                  name;                            // The name of this surface
    std::string                  skeletonRootName;                // The name of the root of the skeleton.
    Bounds<float, 3>             bounds;
    std::vector<std::string>     jointNames;
    std::vector<Vec3f>           jointGeometryMins;
    std::vector<Vec3f>           jointGeometryMaxs;
    std::vector<Mat4f>           inverseBindMatrices;
    std::vector<RawBlendChannel> blendChannels;
    bool                         discrete;
};

struct RawChannel
{
    int                nodeIndex;
    std::vector<Vec3f> translations;
    std::vector<Quatf> rotations;
    std::vector<Vec3f> scales;
    std::vector<float> weights;
};

struct RawAnimation
{
    std::string             name;
    std::vector<float>      times;
    std::vector<RawChannel> channels;
};

struct RawCamera
{
    std::string name;
    std::string nodeName;

    enum
    {
        CAMERA_MODE_PERSPECTIVE,
        CAMERA_MODE_ORTHOGRAPHIC
    } mode;

    struct
    {
        float aspectRatio;
        float fovDegreesX;
        float fovDegreesY;
        float nearZ;
        float farZ;
    } perspective;

    struct
    {
        float magX;
        float magY;
        float nearZ;
        float farZ;
    } orthographic;
};

struct RawNode
{
    bool                     isJoint;
    std::string              name;
    std::string              parentName;
    std::vector<std::string> childNames;
    Vec3f                    translation;
    Quatf                    rotation;
    Vec3f                    scale;
    long                     surfaceId;
};

class RawModel
{
public:
    RawModel();

    // Add geometry.
    void AddVertexAttribute(const RawVertexAttribute attrib);
    int AddVertex(const RawVertex &vertex);
    int AddTriangle(const int v0, const int v1, const int v2, const int materialIndex, const int surfaceIndex);
    int AddTexture(const std::string &name, const std::string &fileName, const std::string &fileLocation, RawTextureUsage usage);
    int AddMaterial(const RawMaterial &material);
    int AddMaterial(
        const char *name, const RawMaterialType materialType, const int textures[RAW_TEXTURE_USAGE_MAX],
        std::shared_ptr<RawMatProps> materialInfo);
    int AddSurface(const RawSurface &suface);
    int AddSurface(const char *name, long surfaceId);
    int AddAnimation(const RawAnimation &animation);
    int AddCameraPerspective(
        const char *name, const char *nodeName, const float aspectRatio, const float fovDegreesX, const float fovDegreesY,
        const float nearZ, const float farZ);
    int
    AddCameraOrthographic(const char *name, const char *nodeName, const float magX, const float magY, const float nearZ, const float farZ);
    int AddNode(const RawNode &node);
    int AddNode(const char *name, const char *parentName);
    void SetRootNode(const char *name) { rootNodeName = name; }
    const char *GetRootNode() const { return rootNodeName.c_str(); }

    // Remove unused vertices, textures or materials after removing vertex attributes, textures, materials or surfaces.
    void Condense();

    void TransformTextures(const std::vector<std::function<Vec2f(Vec2f)>> &transforms);

    // Get the attributes stored per vertex.
    int GetVertexAttributes() const { return vertexAttributes; }

    // Iterate over the vertices.
    int GetVertexCount() const { return (int) vertices.size(); }
    const RawVertex &GetVertex(const int index) const { return vertices[index]; }

    // Iterate over the triangles.
    int GetTriangleCount() const { return (int) triangles.size(); }
    const RawTriangle &GetTriangle(const int index) const { return triangles[index]; }

    // Iterate over the textures.
    int GetTextureCount() const { return (int) textures.size(); }
    const RawTexture &GetTexture(const int index) const { return textures[index]; }

    // Iterate over the materials.
    int GetMaterialCount() const { return (int) materials.size(); }
    const RawMaterial &GetMaterial(const int index) const { return materials[index]; }

    // Iterate over the surfaces.
    int GetSurfaceCount() const { return (int) surfaces.size(); }
    const RawSurface &GetSurface(const int index) const { return surfaces[index]; }
    RawSurface &GetSurface(const int index) { return surfaces[index]; }
    int GetSurfaceById(const long id) const;

    // Iterate over the animations.
    int GetAnimationCount() const { return (int) animations.size(); }
    const RawAnimation &GetAnimation(const int index) const { return animations[index]; }

    // Iterate over the cameras.
    int GetCameraCount() const { return (int) cameras.size(); }
    const RawCamera &GetCamera(const int index) const { return cameras[index]; }

    // Iterate over the nodes.
    int GetNodeCount() const { return (int) nodes.size(); }
    const RawNode &GetNode(const int index) const { return nodes[index]; }
    RawNode &GetNode(const int index) { return nodes[index]; }
    int GetNodeByName(const char *name) const;

    // Create individual attribute arrays.
    // Returns true if the vertices store the particular attribute.
    template<typename _attrib_type_>
    void GetAttributeArray(std::vector<_attrib_type_> &out, const _attrib_type_ RawVertex::* ptr) const;

    // Create an array with a raw model for each material.
    // Multiple surfaces with the same material will turn into a single model.
    // However, surfaces that are marked as 'discrete' will turn into separate models.
    void CreateMaterialModels(
        std::vector<RawModel> &materialModels, const int maxModelVertices, const int keepAttribs, const bool forceDiscrete) const;

private:
    std::string                                      rootNodeName;
    int                                              vertexAttributes;
    std::unordered_map<RawVertex, int, VertexHasher> vertexHash;
    std::vector<RawVertex>                           vertices;
    std::vector<RawTriangle>                         triangles;
    std::vector<RawTexture>                          textures;
    std::vector<RawMaterial>                         materials;
    std::vector<RawSurface>                          surfaces;
    std::vector<RawAnimation>                        animations;
    std::vector<RawCamera>                           cameras;
    std::vector<RawNode>                             nodes;
};

template<typename _attrib_type_>
void RawModel::GetAttributeArray(std::vector<_attrib_type_> &out, const _attrib_type_ RawVertex::* ptr) const
{
    out.resize(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++) {
        out[i] = vertices[i].*ptr;
    }
}

#endif // !__RAWMODEL_H__
