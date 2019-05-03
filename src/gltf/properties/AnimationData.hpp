/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"

struct AnimationData : Holdable {
  AnimationData(std::string name, const AccessorData& timeAccessor);

  // assumption: 1-to-1 relationship between channels and samplers; this is a simplification on what
  // glTF can express, but it means we can rely on samplerIx == channelIx throughout an animation
  void AddNodeChannel(const NodeData& node, const AccessorData& accessor, std::string path);

  json serialize() const override;

  struct channel_t {
    channel_t(uint32_t _ix, const NodeData& node, std::string path);

    const uint32_t ix;
    const uint32_t node;
    const std::string path;
  };

  struct sampler_t {
    sampler_t(uint32_t time, uint32_t output);

    const uint32_t time;
    const uint32_t output;
  };

  const std::string name;
  const uint32_t timeAccessor;
  std::vector<channel_t> channels;
  std::vector<sampler_t> samplers;
};

void to_json(json& j, const AnimationData::channel_t& data);
void to_json(json& j, const AnimationData::sampler_t& data);
