/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __RAW2GLTF_H__
#define __RAW2GLTF_H__

#include <memory>
#include <string>

// This can be a macro under Windows, confusing Draco
#undef ERROR
#include <draco/compression/encode.h>

#include <json.hpp>
#include <fifo_map.hpp>

template<class K, class V, class ignore, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;

using json = nlohmann::basic_json<workaround_fifo_map>;

#include "FBX2glTF.h"
#include "RawModel.h"

static const std::string KHR_DRACO_MESH_COMPRESSION            = "KHR_draco_mesh_compression";
static const std::string KHR_MATERIALS_COMMON                  = "KHR_materials_common";
static const std::string KHR_MATERIALS_CMN_UNLIT               = "KHR_materials_unlit";
static const std::string KHR_MATERIALS_PBR_SPECULAR_GLOSSINESS = "KHR_materials_pbrSpecularGlossiness";

static const std::string extBufferFilename = "buffer.bin";

/**
 * User-supplied options that dictate the nature of the glTF being generated.
 */
struct GltfOptions
{
    /**
     * If negative, disabled. Otherwise, a bitfield of RawVertexAttributes that
     * specify the largest set of attributes that'll ever be kept for a vertex.
     * The special bit RAW_VERTEX_ATTRIBUTE_AUTO triggers smart mode, where the
     * attributes to keep are inferred from which textures are supplied.
     */
    int  keepAttribs;
    /** Whether to output a .glb file, the binary format of glTF. */
    bool outputBinary;
    /** If non-binary, whether to inline all resources, for a single (large) .glTF file. */
    bool embedResources;
    /** Whether to use KHR_draco_mesh_compression to minimize static geometry size. */
    bool useDraco;
    /** Whether to use KHR_materials_common to extend materials definitions. */
    bool useKHRMatCom;
    /** Whether to use KHR_materials_unlit to extend materials definitions. */
    bool useKHRMatUnlit;
    /** Whether to populate the pbrMetallicRoughness substruct in materials. */
    bool usePBRMetRough;
    /** Whether to use KHR_materials_pbrSpecularGlossiness to extend material definitions. */
    bool usePBRSpecGloss;
    /** Whether to include blend shape normals, if present according to the SDK. */
    bool useBlendShapeNormals;
    /** Whether to include blend shape tangents, if present according to the SDK. */
    bool useBlendShapeTangents;
};

struct ComponentType {
    // OpenGL Datatype enums
    enum GL_DataType
    {
        GL_BYTE = 5120,
        GL_UNSIGNED_BYTE,
        GL_SHORT,
        GL_UNSIGNED_SHORT,
        GL_INT,
        GL_UNSIGNED_INT,
        GL_FLOAT
    };

    const GL_DataType glType;
    const unsigned int size;
};

static const ComponentType CT_USHORT = {ComponentType::GL_UNSIGNED_SHORT, 2};
static const ComponentType CT_FLOAT  = {ComponentType::GL_FLOAT, 4};

// Map our low-level data types for glTF output
struct GLType {
    GLType(const ComponentType &componentType, unsigned int count, const std::string dataType)
        : componentType(componentType),
          count(count),
          dataType(dataType)
    {}

    unsigned int byteStride() const { return componentType.size * count; }

    void write(uint8_t *buf, const float scalar) const    { *((float *) buf)    = scalar; }
    void write(uint8_t *buf, const uint16_t scalar) const { *((uint16_t *) buf) = scalar; }

    template<class T, int d>
    void write(uint8_t *buf, const mathfu::Vector<T, d> &vector) const {
        for (int ii = 0; ii < d; ii ++) {
            ((T *)buf)[ii] = vector(ii);
        }
    }
    template<class T, int d>
    void write(uint8_t *buf, const mathfu::Matrix<T, d> &matrix) const {
        // three matrix types require special alignment considerations that we don't handle
        // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#data-alignment
        assert(!(sizeof(T) == 1 && d == 2));
        assert(!(sizeof(T) == 1 && d == 3));
        assert(!(sizeof(T) == 2 && d == 2));
        for (int col = 0; col < d; col ++) {
            for (int row = 0; row < d; row ++) {
                // glTF matrices are column-major
                ((T *)buf)[col * d + row] = matrix(row, col);
            }
        }
    }
    template<class T>
    void write(uint8_t *buf, const mathfu::Quaternion<T> &quaternion) const {
        for (int ii = 0; ii < 3; ii++) {
            ((T *)buf)[ii] = quaternion.vector()(ii);
        }
        ((T *)buf)[3] = quaternion.scalar();
    }

    const ComponentType componentType;
    const uint8_t       count;
    const std::string   dataType;
};

static const GLType GLT_FLOAT  = {CT_FLOAT, 1, "SCALAR"};
static const GLType GLT_USHORT = {CT_USHORT, 1, "SCALAR"};
static const GLType GLT_VEC2F  = {CT_FLOAT, 2, "VEC2"};
static const GLType GLT_VEC3F  = {CT_FLOAT, 3, "VEC3"};
static const GLType GLT_VEC4F  = {CT_FLOAT, 4, "VEC4"};
static const GLType GLT_VEC4I  = {CT_USHORT, 4, "VEC4"};
static const GLType GLT_MAT2F  = {CT_USHORT, 4, "MAT2"};
static const GLType GLT_MAT3F  = {CT_USHORT, 9, "MAT3"};
static const GLType GLT_MAT4F  = {CT_FLOAT, 16, "MAT4"};
static const GLType GLT_QUATF  = {CT_FLOAT, 4, "VEC4"};

/**
 * The base of any indexed glTF entity.
 */
struct Holdable
{
    uint32_t ix;

    virtual json serialize() const = 0;
};

template<class T>
struct AttributeDefinition
{
    const std::string                    gltfName;
    const T RawVertex::*                 rawAttributeIx;
    const GLType                         glType;
    const draco::GeometryAttribute::Type dracoAttribute;
    const draco::DataType                dracoComponentType;

    AttributeDefinition(
        const std::string gltfName, const T RawVertex::*rawAttributeIx, const GLType &_glType,
        const draco::GeometryAttribute::Type dracoAttribute, const draco::DataType dracoComponentType)
        : gltfName(gltfName),
          rawAttributeIx(rawAttributeIx),
          glType(_glType),
          dracoAttribute(dracoAttribute),
          dracoComponentType(dracoComponentType) {}

    AttributeDefinition(
        const std::string gltfName, const T RawVertex::*rawAttributeIx, const GLType &_glType)
        : gltfName(gltfName),
          rawAttributeIx(rawAttributeIx),
          glType(_glType),
          dracoAttribute(draco::GeometryAttribute::INVALID),
          dracoComponentType(draco::DataType::DT_INVALID) {}
};

struct AccessorData;
struct AnimationData;
struct BufferData;
struct BufferViewData;
struct CameraData;
struct GLTFData;
struct ImageData;
struct MaterialData;
struct MeshData;
struct NodeData;
struct PrimitiveData;
struct SamplerData;
struct SceneData;
struct SkinData;
struct TextureData;

struct ModelData
{
    explicit ModelData(std::shared_ptr<const std::vector<uint8_t> > const &_binary)
        : binary(_binary)
    {
    }

    std::shared_ptr<const std::vector<uint8_t> > const binary;
};

ModelData *Raw2Gltf(
    std::ofstream &gltfOutStream,
    const std::string &outputFolder,
    const RawModel &raw,
    const GltfOptions &options
);

#endif // !__RAW2GLTF_H__
