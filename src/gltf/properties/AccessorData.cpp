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
      name(name) {}

AccessorData::AccessorData(GLType type)
    : Holdable(), bufferView(-1), type(std::move(type)), byteOffset(0), count(0) {}

json AccessorData::serialize() const {
  json result{
      {"componentType", type.componentType.glType}, {"type", type.dataType}, {"count", count}};
  if (bufferView >= 0) {
    result["bufferView"] = bufferView;
  }
	if (byteOffset >= 0) {
		result["byteOffset"] = byteOffset;
	}
  if (!min.empty()) {
    result["min"] = min;
  }
  if (!max.empty()) {
    result["max"] = max;
  }
  if (name.length() > 0) {
    result["name"] = name;
  }
  return result;
}
