/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>

// This can be a macro under Windows, confusing Draco
#undef ERROR
#include <draco/compression/encode.h>

#include "FBX2glTF.h"
#include "raw/RawModel.hpp"
#include "mathfu.hpp" // ensure Vec2f/Vec3f/Mat4f/Quatf typedefs are available

const std::string KHR_DRACO_MESH_COMPRESSION = "KHR_draco_mesh_compression";
const std::string KHR_MATERIALS_CMN_UNLIT = "KHR_materials_unlit";
const std::string KHR_LIGHTS_PUNCTUAL = "KHR_lights_punctual";

const std::string extBufferFilename = "buffer.bin";

struct ComponentType {
  // OpenGL Datatype enums
  enum GL_DataType {
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

const ComponentType CT_USHORT = {ComponentType::GL_UNSIGNED_SHORT, 2};
const ComponentType CT_UINT = {ComponentType::GL_UNSIGNED_INT, 4};
const ComponentType CT_FLOAT = {ComponentType::GL_FLOAT, 4};

// Map our low-level data types for glTF output
struct GLType {
  GLType(const ComponentType& componentType, unsigned int count, const std::string dataType)
      : componentType(componentType), count(count), dataType(dataType) {}

  unsigned int byteStride() const {
    return componentType.size * count;
  }

  void write(uint8_t* buf, const float scalar) const {
    *((float*)buf) = scalar;
  }
  void write(uint8_t* buf, const uint32_t scalar) const {
    switch (componentType.size) {
      case 1:
        *buf = (uint8_t)scalar;
        break;
      case 2:
        *((uint16_t*)buf) = (uint16_t)scalar;
        break;
      case 4:
        *((uint32_t*)buf) = scalar;
        break;
    }
  }

  // vector overloads (Vec2f/Vec3f/Vec4f are typedefs to glm types via mathfu.hpp shim)
  void write(uint8_t* buf, const Vec2f& v) const {
    for (int i = 0; i < 2; ++i) {
      if (componentType.size == 1)
        ((uint8_t*)buf)[i] = (uint8_t)v[i];
      else if (componentType.size == 2)
        ((uint16_t*)buf)[i] = (uint16_t)v[i];
      else
        ((float*)buf)[i] = (float)v[i];
    }
  }
  void write(uint8_t* buf, const Vec3f& v) const {
    for (int i = 0; i < 3; ++i) {
      if (componentType.size == 1)
        ((uint8_t*)buf)[i] = (uint8_t)v[i];
      else if (componentType.size == 2)
        ((uint16_t*)buf)[i] = (uint16_t)v[i];
      else
        ((float*)buf)[i] = (float)v[i];
    }
  }
  void write(uint8_t* buf, const Vec4f& v) const {
    for (int i = 0; i < 4; ++i) {
      if (componentType.size == 1)
        ((uint8_t*)buf)[i] = (uint8_t)v[i];
      else if (componentType.size == 2)
        ((uint16_t*)buf)[i] = (uint16_t)v[i];
      else
        ((float*)buf)[i] = (float)v[i];
    }
  }
  void write(uint8_t* buf, const Vec4i& v) const {
    for (int i = 0; i < 4; ++i) {
      if (componentType.size == 1)
        ((uint8_t*)buf)[i] = (uint8_t)v[i];
      else if (componentType.size == 2)
        ((uint16_t*)buf)[i] = (uint16_t)v[i];
      else
        ((uint32_t*)buf)[i] = (uint32_t)v[i];
    }
  }
  // matrix overloads: write column-major as glTF expects, converting element type by componentType.size
  void write(uint8_t* buf, const Mat2f& m) const {
    const int d = 2;
    for (int col = 0; col < d; ++col) {
      for (int row = 0; row < d; ++row) {
        const float val = m[col][row];
        const int idx = col * d + row;
        if (componentType.size == 1)
          ((uint8_t*)buf)[idx] = (uint8_t)val;
        else if (componentType.size == 2)
          ((uint16_t*)buf)[idx] = (uint16_t)val;
        else
          ((float*)buf)[idx] = (float)val;
      }
    }
  }
  void write(uint8_t* buf, const Mat3f& m) const {
    const int d = 3;
    for (int col = 0; col < d; ++col) {
      for (int row = 0; row < d; ++row) {
        const float val = m[col][row];
        const int idx = col * d + row;
        if (componentType.size == 1)
          ((uint8_t*)buf)[idx] = (uint8_t)val;
        else if (componentType.size == 2)
          ((uint16_t*)buf)[idx] = (uint16_t)val;
        else
          ((float*)buf)[idx] = (float)val;
      }
    }
  }
  void write(uint8_t* buf, const Mat4f& m) const {
    const int d = 4;
    for (int col = 0; col < d; ++col) {
      for (int row = 0; row < d; ++row) {
        const float val = m[col][row];
        const int idx = col * d + row;
        if (componentType.size == 1)
          ((uint8_t*)buf)[idx] = (uint8_t)val;
        else if (componentType.size == 2)
          ((uint16_t*)buf)[idx] = (uint16_t)val;
        else
          ((float*)buf)[idx] = (float)val;
      }
    }
  }
  void write(uint8_t* buf, const Quatf& q) const {
    // write x, y, z, w
    const float vals[4] = {q.x, q.y, q.z, q.w};
    for (int i = 0; i < 4; ++i) {
      if (componentType.size == 1)
        ((uint8_t*)buf)[i] = (uint8_t)vals[i];
      else if (componentType.size == 2)
        ((uint16_t*)buf)[i] = (uint16_t)vals[i];
      else
        ((float*)buf)[i] = vals[i];
    }
  }

  const ComponentType componentType;
  const uint8_t count;
  const std::string dataType;
};

// Inline definitions for GLT_* constants used by Raw2Gltf.cpp
// Using inline to avoid multiple-definition issues when included in multiple TUs (C++17)
inline const GLType GLT_FLOAT  = GLType(CT_FLOAT, 1, "SCALAR");
inline const GLType GLT_VEC2F  = GLType(CT_FLOAT, 2, "VEC2");
inline const GLType GLT_VEC3F  = GLType(CT_FLOAT, 3, "VEC3");
inline const GLType GLT_VEC4F  = GLType(CT_FLOAT, 4, "VEC4");
inline const GLType GLT_QUATF  = GLType(CT_FLOAT, 4, "VEC4");

inline const GLType GLT_VEC4I  = GLType(CT_UINT,  4, "VEC4"); // integer vector (e.g. joint indices)
inline const GLType GLT_USHORT = GLType(CT_USHORT, 1, "SCALAR");
inline const GLType GLT_UINT   = GLType(CT_UINT,   1, "SCALAR");

// added: 4x4 matrix type for inverse bind matrices, etc.
inline const GLType GLT_MAT4F  = GLType(CT_FLOAT, 16, "MAT4");

/**
 * The base of any indexed glTF entity.
 */
struct Holdable {
  uint32_t ix = UINT_MAX;

  virtual json serialize() const = 0;
};

template <class T>
struct AttributeDefinition {
  const std::string gltfName;
  const T RawVertex::*rawAttributeIx;
  const GLType glType;
  const draco::GeometryAttribute::Type dracoAttribute;
  const draco::DataType dracoComponentType;

  AttributeDefinition(
      const std::string gltfName,
      const T RawVertex::*rawAttributeIx,
      const GLType& _glType,
      const draco::GeometryAttribute::Type dracoAttribute,
      const draco::DataType dracoComponentType)
      : gltfName(gltfName),
        rawAttributeIx(rawAttributeIx),
        glType(_glType),
        dracoAttribute(dracoAttribute),
        dracoComponentType(dracoComponentType) {}

  AttributeDefinition(
      const std::string gltfName,
      const T RawVertex::*rawAttributeIx,
      const GLType& _glType)
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

struct ModelData {
  explicit ModelData(std::shared_ptr<const std::vector<uint8_t>> const& _binary)
      : binary(_binary) {}

  std::shared_ptr<const std::vector<uint8_t>> const binary;
};

ModelData* Raw2Gltf(
    std::ofstream& gltfOutStream,
    const std::string& outputFolder,
    const RawModel& raw,
    const GltfOptions& options);
