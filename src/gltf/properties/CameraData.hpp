/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

// TODO: this class needs some work
struct CameraData : Holdable {
  CameraData();
  json serialize() const override;

  std::string name;
  std::string type;
  float aspectRatio;
  float yfov;
  float xmag;
  float ymag;
  float znear;
  float zfar;
};
