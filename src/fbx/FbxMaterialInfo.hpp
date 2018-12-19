/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "FBX2glTF.h"

class FbxMaterialInfo {
 public:
  FbxMaterialInfo(const FbxString& name, const FbxString& shadingModel)
      : name(name), shadingModel(shadingModel) {}

  const FbxString name;
  const FbxString shadingModel;
};
