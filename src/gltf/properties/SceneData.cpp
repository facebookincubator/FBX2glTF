/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "SceneData.hpp"

#include "NodeData.hpp"

SceneData::SceneData(std::string name, const NodeData& rootNode)
    : Holdable(), name(std::move(name)), nodes({rootNode.ix}) {}

json SceneData::serialize() const {
  assert(nodes.size() <= 1);
  return {{"name", name}, {"nodes", nodes}};
}
