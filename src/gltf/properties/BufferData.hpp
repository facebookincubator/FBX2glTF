/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct BufferData : Holdable {
  explicit BufferData(const std::shared_ptr<const std::vector<uint8_t>>& binData);

  BufferData(
      std::string uri,
      const std::shared_ptr<const std::vector<uint8_t>>& binData,
      bool isEmbedded = false);

  json serialize() const override;

  const bool isGlb;
  const std::string uri;
  const std::shared_ptr<const std::vector<uint8_t>> binData; // TODO this is just weird
};
