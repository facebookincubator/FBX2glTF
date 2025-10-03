/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LightData.hpp"
#include <cmath>

// Define M_PI_4 for MSVC compatibility
#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif

LightData::LightData(
    std::string name,
    Type type,
    Vec3f color,
    float intensity,
    float innerConeAngle,
    float outerConeAngle)
    : Holdable(),
      type(type),
      color(color),
      intensity(intensity),
      innerConeAngle(innerConeAngle),
      outerConeAngle(outerConeAngle) {}

json LightData::serialize() const {
  json result{{"name", name}, {"color", toStdVec(color)}, {"intensity", intensity}};
  switch (type) {
    case Directional:
      result["type"] = "directional";
      break;
    case Point:
      result["type"] = "point";
      break;
    case Spot:
      result["type"] = "spot";
      json spotJson;
      if (innerConeAngle != 0) {
        spotJson["innerConeAngle"] = innerConeAngle;
      }
      if (std::abs(outerConeAngle - static_cast<float>(M_PI_4)) > 1e-6f) {
        spotJson["outerConeAngle"] = outerConeAngle;
      }
      result["spot"] = spotJson;
      break;
  }
  return result;
}
