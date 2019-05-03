/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NodeData.hpp"

NodeData::NodeData(
    std::string name,
    const Vec3f& translation,
    const Quatf& rotation,
    const Vec3f& scale,
    bool isJoint)
    : Holdable(),
      name(std::move(name)),
      isJoint(isJoint),
      translation(translation),
      rotation(rotation),
      scale(scale),
      children(),
      mesh(-1),
      camera(-1),
      light(-1),
      skin(-1) {}

void NodeData::AddChildNode(uint32_t childIx) {
  children.push_back(childIx);
}

void NodeData::SetMesh(uint32_t meshIx) {
  assert(mesh < 0);
  assert(!isJoint);
  mesh = meshIx;
}

void NodeData::SetSkin(uint32_t skinIx) {
  assert(skin < 0);
  assert(!isJoint);
  skin = skinIx;
}

void NodeData::SetCamera(uint32_t cameraIndex) {
  assert(!isJoint);
  camera = cameraIndex;
}

void NodeData::SetLight(uint32_t lightIndex) {
  assert(!isJoint);
  light = lightIndex;
}

json NodeData::serialize() const {
  json result = {{"name", name}};

  // if any of the T/R/S have NaN components, just leave them out of the glTF
  auto maybeAdd = [&](std::string key, std::vector<float> vec) -> void {
    if (std::none_of(vec.begin(), vec.end(), [&](float n) { return std::isnan(n); })) {
      result[key] = vec;
    }
  };
  maybeAdd("translation", toStdVec(translation));
  maybeAdd("rotation", toStdVec(rotation));
  maybeAdd("scale", toStdVec(scale));

  if (!children.empty()) {
    result["children"] = children;
  }
  if (isJoint) {
    // sanity-check joint node
    assert(mesh < 0 && skin < 0);
  } else {
    // non-joint node
    if (mesh >= 0) {
      result["mesh"] = mesh;
    }
    if (!skeletons.empty()) {
      result["skeletons"] = skeletons;
    }
    if (skin >= 0) {
      result["skin"] = skin;
    }
    if (camera >= 0) {
      result["camera"] = camera;
    }
    if (light >= 0) {
      result["extensions"][KHR_LIGHTS_PUNCTUAL]["light"] = light;
    }
  }

  for (const auto& i : userProperties) {
    auto& prop_map = result["extras"]["fromFBX"]["userProperties"];

    json j = json::parse(i);
    for (const auto& k : json::iterator_wrapper(j)) {
      prop_map[k.key()] = k.value();
    }
  }

  return result;
}
