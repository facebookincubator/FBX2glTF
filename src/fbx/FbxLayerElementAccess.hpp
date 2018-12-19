/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
#pragma once
#include "FBX2glTF.h"

template <typename _type_>
class FbxLayerElementAccess {
 public:
  FbxLayerElementAccess(const FbxLayerElementTemplate<_type_>* layer, int count);

  bool LayerPresent() const {
    return (mappingMode != FbxLayerElement::eNone);
  }

  _type_ GetElement(
      const int polygonIndex,
      const int polygonVertexIndex,
      const int controlPointIndex,
      const _type_ defaultValue) const;
  _type_ GetElement(
      const int polygonIndex,
      const int polygonVertexIndex,
      const int controlPointIndex,
      const _type_ defaultValue,
      const FbxMatrix& transform,
      const bool normalize) const;

 private:
  FbxLayerElement::EMappingMode mappingMode;
  const FbxLayerElementArrayTemplate<_type_>* elements;
  const FbxLayerElementArrayTemplate<int>* indices;
};

template <typename _type_>
FbxLayerElementAccess<_type_>::FbxLayerElementAccess(
    const FbxLayerElementTemplate<_type_>* layer,
    int count)
    : mappingMode(FbxLayerElement::eNone), elements(nullptr), indices(nullptr) {
  if (count <= 0 || layer == nullptr) {
    return;
  }
  const FbxLayerElement::EMappingMode newMappingMode = layer->GetMappingMode();
  if (newMappingMode == FbxLayerElement::eByControlPoint ||
      newMappingMode == FbxLayerElement::eByPolygonVertex ||
      newMappingMode == FbxLayerElement::eByPolygon) {
    mappingMode = newMappingMode;
    elements = &layer->GetDirectArray();
    indices = (layer->GetReferenceMode() == FbxLayerElement::eIndexToDirect ||
               layer->GetReferenceMode() == FbxLayerElement::eIndex)
        ? &layer->GetIndexArray()
        : nullptr;
  }
}

template <typename _type_>
_type_ FbxLayerElementAccess<_type_>::GetElement(
    const int polygonIndex,
    const int polygonVertexIndex,
    const int controlPointIndex,
    const _type_ defaultValue) const {
  if (mappingMode != FbxLayerElement::eNone) {
    int index = (mappingMode == FbxLayerElement::eByControlPoint)
        ? controlPointIndex
        : ((mappingMode == FbxLayerElement::eByPolygonVertex) ? polygonVertexIndex : polygonIndex);
    index = (indices != nullptr) ? (*indices)[index] : index;
    _type_ element = elements->GetAt(index);
    return element;
  }
  return defaultValue;
}

template <typename _type_>
_type_ FbxLayerElementAccess<_type_>::GetElement(
    const int polygonIndex,
    const int polygonVertexIndex,
    const int controlPointIndex,
    const _type_ defaultValue,
    const FbxMatrix& transform,
    const bool normalize) const {
  if (mappingMode != FbxLayerElement::eNone) {
    _type_ element = transform.MultNormalize(
        GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, defaultValue));
    if (normalize) {
      element.Normalize();
    }
    return element;
  }
  return defaultValue;
}
