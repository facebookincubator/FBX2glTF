/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AnimationData.hpp"

#include <utility>

#include "AccessorData.hpp"
#include "NodeData.hpp"

AnimationData::AnimationData(std::string name, const AccessorData& timeAccessor)
    : Holdable(), name(std::move(name)), timeAccessor(timeAccessor.ix) {}

// assumption: 1-to-1 relationship between channels and samplers; this is a simplification on what
// glTF can express, but it means we can rely on samplerIx == channelIx throughout an animation
void AnimationData::AddNodeChannel(
    const NodeData& node,
    const AccessorData& accessor,
    std::string path) {
  assert(channels.size() == samplers.size());
  uint32_t ix = to_uint32(channels.size());
  channels.emplace_back(channel_t(ix, node, std::move(path)));
  samplers.emplace_back(sampler_t(timeAccessor, accessor.ix));
}

json AnimationData::serialize() const {
  return {{"name", name}, {"channels", channels}, {"samplers", samplers}};
}

AnimationData::channel_t::channel_t(uint32_t ix, const NodeData& node, std::string path)
    : ix(ix), node(node.ix), path(std::move(path)) {}

AnimationData::sampler_t::sampler_t(uint32_t time, uint32_t output) : time(time), output(output) {}

void to_json(json& j, const AnimationData::channel_t& data) {
  j = json{
      {"sampler", data.ix},
      {
          "target",
          {{"node", data.node}, {"path", data.path}},
      }};
}

void to_json(json& j, const AnimationData::sampler_t& data) {
  j = json{
      {"input", data.time},
      {"interpolation", "LINEAR"},
      {"output", data.output},
  };
}
