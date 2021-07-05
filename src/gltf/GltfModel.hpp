/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fstream>

#include "FBX2glTF.h"

#include "gltf/properties/AccessorData.hpp"
#include "gltf/properties/AnimationData.hpp"
#include "gltf/properties/BufferData.hpp"
#include "gltf/properties/BufferViewData.hpp"
#include "gltf/properties/CameraData.hpp"
#include "gltf/properties/ImageData.hpp"
#include "gltf/properties/LightData.hpp"
#include "gltf/properties/MaterialData.hpp"
#include "gltf/properties/MeshData.hpp"
#include "gltf/properties/NodeData.hpp"
#include "gltf/properties/PrimitiveData.hpp"
#include "gltf/properties/SamplerData.hpp"
#include "gltf/properties/SceneData.hpp"
#include "gltf/properties/SkinData.hpp"
#include "gltf/properties/TextureData.hpp"

/**
 * glTF 2.0 is based on the idea that data structs within a file are referenced by index; an
 * accessor will point to the n:th buffer view, and so on. The Holder class takes a freshly
 * instantiated class, and then creates, stored, and returns a shared_ptr<T> for it.
 *
 * The idea is that every glTF resource in the file will live as long as the Holder does, and the
 * Holders are all kept in the GLTFData struct. Clients may certainly cnhoose to perpetuate the full
 * shared_ptr<T> reference counting type, but generally speaking we pass around simple T& and T*
 * types because the GLTFData struct will, by design, outlive all other activity that takes place
 * during in a single conversion run.
 */
template <typename T>
class Holder {
 public:
  std::shared_ptr<T> hold(T* ptr) {
    ptr->ix = to_uint32(ptrs.size());
    ptrs.emplace_back(ptr);
    return ptrs.back();
  }
  std::vector<std::shared_ptr<T>> ptrs;
};

class GltfModel {
 public:
  explicit GltfModel(const GltfOptions& options)
      : binary(new std::vector<uint8_t>),
        isGlb(options.outputBinary),
        defaultSampler(nullptr),
        defaultBuffer(buffers.hold(buildDefaultBuffer(options))) {
    defaultSampler = samplers.hold(buildDefaultSampler());
  }

  std::shared_ptr<BufferViewData> GetAlignedBufferView(
      BufferData& buffer,
      const BufferViewData::GL_ArrayType target);
  std::shared_ptr<BufferViewData>
  AddRawBufferView(BufferData& buffer, const char* source, uint32_t bytes);
  std::shared_ptr<BufferViewData> AddBufferViewForFile(
      BufferData& buffer,
      const std::string& filename);

  template <class T>
  void
  CopyToBufferView(BufferViewData& bufferView, const std::vector<T>& source, const GLType& type) {
    bufferView.appendAsBinaryArray(source, *binary, type);
  }

  template <class T>
  std::shared_ptr<AccessorData> AddAccessorWithView(
      BufferViewData& bufferView,
      const GLType& type,
      const std::vector<T>& source,
      std::string name) {
    auto accessor = accessors.hold(new AccessorData(bufferView, type, name));
    bufferView.appendAsBinaryArray(source, *binary, type);
    accessor->count = bufferView.count;
    return accessor;
  }

  template <class T>
  std::shared_ptr<AccessorData> AddSparseAccessorWithView(
      AccessorData& baseAccessor,
      BufferViewData& indexBufferView,
      const GLType& indexBufferViewType,
      BufferViewData& bufferView,
      const GLType& type,
      const std::vector<T>& source,
      std::string name) {
    auto accessor =
        accessors.hold(new AccessorData(baseAccessor, indexBufferView, bufferView, type, name));
    bufferView.appendAsBinaryArray(source, *binary, type);
    accessor->count = baseAccessor.count;
    accessor->sparseIdxBufferViewType = indexBufferViewType.componentType.glType;
    return accessor;
  }

  //  template <class T>
  std::shared_ptr<AccessorData> AddSparseAccessor(
      AccessorData& baseAccessor,
      BufferViewData& indexBufferView,
      const GLType& indexBufferViewType,
      BufferViewData& bufferView,
      const GLType& type,
      //      const std::vector<T>& source,
      std::string name) {
    auto accessor =
        accessors.hold(new AccessorData(baseAccessor, indexBufferView, bufferView, type, name));
    // bufferView.appendAsBinaryArray(source, *binary, type);
    accessor->count = baseAccessor.count;
    accessor->sparseIdxBufferViewType = indexBufferViewType.componentType.glType;
    return accessor;
  }

  template <class T>
  std::shared_ptr<AccessorData>
  AddAccessorAndView(BufferData& buffer, const GLType& type, const std::vector<T>& source) {
    auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_NONE);
    return AddAccessorWithView(*bufferView, type, source, std::string(""));
  }

  template <class T>
  std::shared_ptr<AccessorData> AddAccessorAndView(
      BufferData& buffer,
      const GLType& type,
      const std::vector<T>& source,
      std::string name) {
    auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_NONE);
    return AddAccessorWithView(*bufferView, type, source, name);
  }

  template <class T>
  std::shared_ptr<AccessorData> AddAttributeToPrimitive(
      BufferData& buffer,
      const RawModel& surfaceModel,
      PrimitiveData& primitive,
      const AttributeDefinition<T>& attrDef) {
    // copy attribute data into vector
    std::vector<T> attribArr;
    surfaceModel.GetAttributeArray<T>(attribArr, attrDef.rawAttributeIx);

    std::shared_ptr<AccessorData> accessor;
    if (attrDef.dracoComponentType != draco::DT_INVALID && primitive.dracoMesh != nullptr) {
      primitive.AddDracoAttrib(attrDef, attribArr);

      accessor = accessors.hold(new AccessorData(attrDef.glType));
      accessor->count = to_uint32(attribArr.size());
    } else {
      auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_BUFFER);
      accessor = AddAccessorWithView(*bufferView, attrDef.glType, attribArr, std::string(""));
    }
    primitive.AddAttrib(attrDef.gltfName, *accessor);
    return accessor;
  };

  template <class T>
  std::shared_ptr<AccessorData> AddAttributeArrayToPrimitive(
      BufferData& buffer,
      const RawModel& surfaceModel,
      PrimitiveData& primitive,
      const AttributeArrayDefinition<T>& attrDef) {
    // copy attribute data into vector
    std::vector<T> attribArr;
    surfaceModel.GetArrayAttributeArray<T>(attribArr, attrDef.rawAttributeIx, attrDef.arrayOffset);

    std::shared_ptr<AccessorData> accessor;
    if (attrDef.dracoComponentType != draco::DT_INVALID && primitive.dracoMesh != nullptr) {
      primitive.AddDracoArrayAttrib(attrDef, attribArr);

      accessor = accessors.hold(new AccessorData(attrDef.glType));
      accessor->count = attribArr.size();
    } else {
      auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_BUFFER);
      accessor = AddAccessorWithView(*bufferView, attrDef.glType, attribArr, std::string(""));
    }
    primitive.AddAttrib(attrDef.gltfName, *accessor);
    return accessor;
  };

  template <class T>
  void serializeHolder(json& glTFJson, std::string key, const Holder<T> holder) {
    if (!holder.ptrs.empty()) {
      std::vector<json> bits;
      for (const auto& ptr : holder.ptrs) {
        bits.push_back(ptr->serialize());
      }
      glTFJson[key] = bits;
    }
  }

  void serializeHolders(json& glTFJson);

  const bool isGlb;

  // cache BufferViewData instances that've already been created from a given filename
  std::map<std::string, std::shared_ptr<BufferViewData>> filenameToBufferView;

  std::shared_ptr<std::vector<uint8_t>> binary;

  Holder<BufferData> buffers;
  Holder<BufferViewData> bufferViews;
  Holder<AccessorData> accessors;
  Holder<ImageData> images;
  Holder<SamplerData> samplers;
  Holder<TextureData> textures;
  Holder<MaterialData> materials;
  Holder<MeshData> meshes;
  Holder<SkinData> skins;
  Holder<AnimationData> animations;
  Holder<CameraData> cameras;
  Holder<NodeData> nodes;
  Holder<SceneData> scenes;
  Holder<LightData> lights;

  std::shared_ptr<SamplerData> defaultSampler;
  std::shared_ptr<BufferData> defaultBuffer;

 private:
  SamplerData* buildDefaultSampler() {
    return new SamplerData();
  }
  BufferData* buildDefaultBuffer(const GltfOptions& options) {
    return options.outputBinary ? new BufferData(binary)
                                : new BufferData(extBufferFilename, binary, options.embedResources);
  }
};
