/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef FBX2GLTF_SAMPLERDATA_H
#define FBX2GLTF_SAMPLERDATA_H

#include "Raw2Gltf.h"

struct SamplerData : Holdable
{
    // this is where magFilter, minFilter, wrapS and wrapT would go, should we want it
    SamplerData()
        : Holdable()
    {
    }

    json serialize() const {
        return json::object();
    }
};

#endif //FBX2GLTF_SAMPLERDATA_H
