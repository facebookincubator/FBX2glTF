/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct PrimitiveData {
  enum MeshMode {
    POINTS = 0,
    LINES,
    LINE_LOOP,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
  };

  PrimitiveData(
      const AccessorData& indices,
      const MaterialData& material,
      std::shared_ptr<draco::Mesh> dracoMesh);

  PrimitiveData(const AccessorData& indices, const MaterialData& material);

  void AddAttrib(std::string name, const AccessorData& accessor);

  void AddTarget(
      const AccessorData* positions,
      const AccessorData* normals,
      const AccessorData* tangents);

  template <class T>
  void AddDracoAttrib(const AttributeDefinition<T> attribute, const std::vector<T>& attribArr) {
    draco::PointAttribute att;
    int8_t componentCount = attribute.glType.count;
    att.Init(
        attribute.dracoAttribute,
        componentCount,
        attribute.dracoComponentType,
        false,
        componentCount * draco::DataTypeLength(attribute.dracoComponentType));

    const int dracoAttId = dracoMesh->AddAttribute(att, true, to_uint32(attribArr.size()));
    draco::PointAttribute* attPtr = dracoMesh->attribute(dracoAttId);

    std::vector<uint8_t> buf(sizeof(T));
    for (uint32_t ii = 0; ii < attribArr.size(); ii++) {
      uint8_t* ptr = &buf[0];
      attribute.glType.write(ptr, attribArr[ii]);
      attPtr->SetAttributeValue(attPtr->mapped_index(draco::PointIndex(ii)), ptr);
    }

    dracoAttributes[attribute.gltfName] = dracoAttId;
  }

  template <class T>
  void AddDracoArrayAttrib(
      const AttributeArrayDefinition<T> attribute,
      const std::vector<T>& attribArr) {
    draco::PointAttribute att;
    int8_t componentCount = attribute.glType.count;
    att.Init(
        attribute.dracoAttribute,
        componentCount,
        attribute.dracoComponentType,
        false,
        componentCount * draco::DataTypeLength(attribute.dracoComponentType));

    const int dracoAttId = dracoMesh->AddAttribute(att, true, attribArr.size());
    draco::PointAttribute* attPtr = dracoMesh->attribute(dracoAttId);

    std::vector<uint8_t> buf(sizeof(T));
    for (uint32_t ii = 0; ii < attribArr.size(); ii++) {
      uint8_t* ptr = &buf[0];
      attribute.glType.write(ptr, attribArr[ii]);
      attPtr->SetAttributeValue(attPtr->mapped_index(draco::PointIndex(ii)), ptr);
    }

    dracoAttributes[attribute.gltfName] = dracoAttId;
  }

  void NoteDracoBuffer(const BufferViewData& data);

  const int indices;
  const unsigned int material;
  const MeshMode mode;

  std::vector<std::tuple<int, int, int>> targetAccessors{};
  std::vector<std::string> targetNames{};

  std::map<std::string, int> attributes;
  std::map<std::string, int> dracoAttributes;

  std::shared_ptr<draco::Mesh> dracoMesh;
  int dracoBufferView;
};

void to_json(json& j, const PrimitiveData& d);
