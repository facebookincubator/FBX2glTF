/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct BufferViewData : Holdable {
  enum GL_ArrayType {
    GL_ARRAY_NONE = 0, // no GL buffer is being set
    GL_ARRAY_BUFFER = 34962,
    GL_ELEMENT_ARRAY_BUFFER = 34963
  };

  BufferViewData(const BufferData& _buffer, const size_t _byteOffset, const GL_ArrayType _target);

  json serialize() const override;

  template <class T>
  void appendAsBinaryArray(const std::vector<T>& in, std::vector<uint8_t>& out, GLType type) {
    const unsigned int stride = type.byteStride();
    const size_t offset = out.size();
    const size_t count = in.size();

    this->byteLength = stride * count;
    this->count = count;

    out.resize(offset + count * stride);
    for (int ii = 0; ii < count; ii++) {
      type.write(&out[offset + ii * stride], in[ii]);
    }
  }

  const unsigned int buffer;
  const unsigned int byteOffset;
  const GL_ArrayType target;

  unsigned int count = 0;
  unsigned int byteLength = 0;
};
