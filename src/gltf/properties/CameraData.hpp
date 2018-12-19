/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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
