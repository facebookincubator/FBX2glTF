/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AccessorData.hpp"
#include "BufferViewData.hpp"

AccessorData::AccessorData(const BufferViewData& bufferView, GLType type, std::string name)
    : Holdable(),
      bufferView(bufferView.ix),
      type(std::move(type)),
      byteOffset(0),
      count(0),
      name(name),
      sparse(false) {}

AccessorData::AccessorData(const AccessorData& baseAccessor, const BufferViewData& sparseIdxBufferView, const BufferViewData& sparseDataBufferView, GLType type, std::string name)
    : Holdable(),
      bufferView(baseAccessor.bufferView),
      type(std::move(type)),
      byteOffset(baseAccessor.byteOffset),
      count(baseAccessor.count),
      name(name),
      sparse(true),
      sparseIdxCount(sparseIdxBufferView.count),
      sparseIdxBufferView(sparseIdxBufferView.ix),
      sparseIdxBufferViewOffset(0),
      sparseIdxBufferViewType(0),
      sparseDataBufferView(sparseDataBufferView.ix),
      sparseDataBufferViewOffset(0) {}

AccessorData::AccessorData(GLType type)
    : Holdable(), bufferView(-1), type(std::move(type)), byteOffset(0), count(0) {}

json AccessorData::serialize() const {
  json result{
      {"componentType", type.componentType.glType}, {"type", type.dataType}, {"count", count}};
  if (bufferView >= 0 && !sparse) {
    result["bufferView"] = bufferView;
    result["byteOffset"] = byteOffset;
  }
  if (!min.empty()) {
    result["min"] = min;
  }
  if (!max.empty()) {
    result["max"] = max;
  }
  if (sparse) {
    json sparseData = {{"count", sparseIdxCount}};
    sparseData["indices"] = { {"bufferView", sparseIdxBufferView},
                              {"byteOffset", sparseIdxBufferViewOffset},
                              {"componentType", sparseIdxBufferViewType}};

    sparseData["values"]  = { {"bufferView", sparseDataBufferView},
                              {"byteOffset", sparseDataBufferViewOffset}};

    result["sparse"] = sparseData;
  }
  if (name.length() > 0) {
    result["name"] = name;
  }
  return result;
}
