/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct LightData : Holdable {
  enum Type {
    Directional,
    Point,
    Spot,
  };

  LightData(
      std::string name,
      Type type,
      Vec3f color,
      float intensity,
      float innerConeAngle,
      float outerConeAngle);

  json serialize() const override;

  const std::string name;
  const Type type;
  const Vec3f color;
  const float intensity;
  const float innerConeAngle;
  const float outerConeAngle;
};
