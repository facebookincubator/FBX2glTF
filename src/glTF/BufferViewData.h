/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef FBX2GLTF_BUFFERVIEW_H
#define FBX2GLTF_BUFFERVIEW_H

#include "Raw2Gltf.h"

struct BufferViewData : Holdable
{
    enum GL_ArrayType
    {
        GL_ARRAY_NONE           = 0, // no GL buffer is being set
        GL_ARRAY_BUFFER         = 34962,
        GL_ELEMENT_ARRAY_BUFFER = 34963
    };

    BufferViewData(const BufferData &_buffer, const size_t _byteOffset, const GL_ArrayType _target);

    json serialize() const override;

    const unsigned int buffer;
    const unsigned int byteOffset;
    const GL_ArrayType target;

    unsigned int byteLength = 0;
};

#endif //FBX2GLTF_BUFFERVIEW_H
