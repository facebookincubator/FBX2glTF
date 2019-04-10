/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "FBX2glTF.h"

class FbxSkinningAccess {
 public:
  static const int MAX_WEIGHTS = 8;

  FbxSkinningAccess(const FbxMesh* pMesh, FbxScene* pScene, FbxNode* pNode);

  bool IsSkinned() const {
    return (vertexJointWeights.size() > 0);
  }

  int MaxWeights() const {
    return MAX_WEIGHTS;
  }

  int GetNodeCount() const {
    return (int)jointNodes.size();
  }

  FbxNode* GetJointNode(const int jointIndex) const {
    return jointNodes[jointIndex];
  }

  const long GetJointId(const int jointIndex) const {
    return jointIds[jointIndex];
  }

  const FbxMatrix& GetJointSkinningTransform(const int jointIndex) const {
    return jointSkinningTransforms[jointIndex];
  }

  const FbxMatrix& GetJointInverseGlobalTransforms(const int jointIndex) const {
    return jointInverseGlobalTransforms[jointIndex];
  }

  const long GetRootNode() const {
    assert(rootIndex != -1);
    return jointIds[rootIndex];
  }

  const FbxAMatrix& GetInverseBindMatrix(const int jointIndex) const {
    return inverseBindMatrices[jointIndex];
  }

  const Vec4i GetVertexIndices(const int controlPointIndex, const int subset) const {
    if (vertexJointIndices.empty())
      return Vec4i(0, 0, 0, 0);
    Vec4i indices(0, 0, 0, 0);
    for (int k = subset * 4; k < subset * 4 + 4 && k < vertexJointIndices[controlPointIndex].size(); k++)
      indices[k - subset * 4] = vertexJointIndices[controlPointIndex][k];
    return indices;
  }

  const Vec4f GetVertexWeights(const int controlPointIndex, const int subset) const {
    if (vertexJointWeights.empty())
      return Vec4f(0.0f);
    Vec4f weights(0.0f);
    for (int k = subset * 4; k < subset * 4 + 4 && k < vertexJointWeights[controlPointIndex].size(); k++)
      weights[k - subset * 4] = vertexJointWeights[controlPointIndex][k];
    return weights;
  }

 private:
  int rootIndex;
  std::vector<long> jointIds;
  std::vector<FbxNode*> jointNodes;
  std::vector<FbxMatrix> jointSkinningTransforms;
  std::vector<FbxMatrix> jointInverseGlobalTransforms;
  std::vector<FbxAMatrix> inverseBindMatrices;
  std::vector<std::vector<uint16_t>> vertexJointIndices;
  std::vector<std::vector<float>> vertexJointWeights;
};
