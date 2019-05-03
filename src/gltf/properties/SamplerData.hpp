/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct SamplerData : Holdable {
  // this is where magFilter, minFilter, wrapS and wrapT would go, should we want it
  SamplerData() : Holdable() {}

  json serialize() const override {
    return json::object();
  }
};
