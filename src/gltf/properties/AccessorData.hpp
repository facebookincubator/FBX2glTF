/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct AccessorData : Holdable {
    AccessorData(const BufferViewData& bufferView, GLType type, std::string name);
    explicit AccessorData(GLType type);
    AccessorData(const AccessorData& baseAccessor, const BufferViewData& sparseIdxBufferView, const BufferViewData& sparseDataBufferView, GLType type, std::string name);

    json serialize() const override;

    unsigned int byteLength() const {
        return type.byteStride() * count;
    }

    const int bufferView;
    const GLType type;

    unsigned int byteOffset;
    unsigned int count;
    std::vector<float> min;
    std::vector<float> max;
    std::string name;

    bool sparse;
    int sparseIdxCount;
    int sparseIdxBufferView;
    int sparseIdxBufferViewOffset;
    int sparseIdxBufferViewType;
    int sparseDataBufferView;
    int sparseDataBufferViewOffset;
};
