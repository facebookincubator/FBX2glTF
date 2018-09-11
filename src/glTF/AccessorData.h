/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef FBX2GLTF_ACCESSORDATA_H
#define FBX2GLTF_ACCESSORDATA_H

#include "Raw2Gltf.h"

struct AccessorData : Holdable
{
    AccessorData(const BufferViewData &bufferView, GLType type, std::string name);
    explicit AccessorData(GLType type);

    json serialize() const override;

    template<class T>
    void appendAsBinaryArray(const std::vector<T> &in, std::vector<uint8_t> &out)
    {
        const unsigned int stride = type.byteStride();
        const size_t       offset = out.size();
        const size_t       count  = in.size();

        this->count = (unsigned int) count;

        out.resize(offset + count * stride);
        for (int ii = 0; ii < count; ii ++) {
            type.write(&out[offset + ii * stride], in[ii]);
        }
    }

    unsigned int byteLength() const { return type.byteStride() * count; }

    const int          bufferView;
    const GLType       type;

    unsigned int       byteOffset;
    unsigned int       count;
    std::vector<float> min;
    std::vector<float> max;
    std::string        name;
};

#endif //FBX2GLTF_ACCESSORDATA_H
