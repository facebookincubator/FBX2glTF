/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "BufferViewData.h"
#include "BufferData.h"

BufferViewData::BufferViewData(const BufferData &_buffer, const size_t _byteOffset, const GL_ArrayType _target)
    : Holdable(),
      buffer(_buffer.ix),
      byteOffset((unsigned int) _byteOffset),
      target(_target)
{
}

json BufferViewData::serialize() const
{
    json result {
        { "buffer", buffer },
        { "byteLength", byteLength },
        { "byteOffset", byteOffset }
    };
    if (target != GL_ARRAY_NONE) {
        result["target"] = target;
    }
    return result;
}
