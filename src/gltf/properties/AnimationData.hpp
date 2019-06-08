/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "gltf/Raw2Gltf.hpp"
#include "AccessorData.hpp"

struct AnimationData : Holdable {
  AnimationData(std::string name);

  AnimationData(std::string name, std::shared_ptr<draco::KeyframeAnimation> dracoKeyframeAnimation);

  void AddTimestamps(const AccessorData& timeAccessor);

	template <class T>
	void AddDracoTimestamps(const AccessorData& timeAccessor, const std::vector<T>& timestamps) {
		this->timeAccessor = timeAccessor.ix;

		std::vector<draco::KeyframeAnimation::TimestampType> dracoTimestamps(timestamps.begin(), timestamps.end());
		dracoKeyframeAnimation->SetTimestamps(dracoTimestamps);
	}

  // assumption: 1-to-1 relationship between channels and samplers; this is a simplification on what
  // glTF can express, but it means we can rely on samplerIx == channelIx throughout an animation
  void AddNodeChannel(const NodeData& node, const AccessorData& accessor, std::string path);

	template <class T>
	void AddDracoNodeChannel(
		const NodeData& node,
		const AccessorData& accessor,
		const std::string& path,
		const ChannelDefinition<T>& keyframe) {
		assert(channels.size() == samplers.size());
		uint32_t ix = to_uint32(channels.size());
		channels.emplace_back(channel_t(ix, node, std::move(path)));
		samplers.emplace_back(sampler_t(timeAccessor, accessor.ix));

		dracoKeyframeAnimation->AddKeyframes(
			keyframe.dracoComponentType, 
			keyframe.glType.count, 
			keyframe.glType.toStdVec(keyframe.channelData));
	}

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
  uint32_t timeAccessor;
  std::vector<channel_t> channels;
  std::vector<sampler_t> samplers;
  std::shared_ptr<draco::KeyframeAnimation> dracoKeyframeAnimation;
};

void to_json(json& j, const AnimationData::channel_t& data);
void to_json(json& j, const AnimationData::sampler_t& data);
