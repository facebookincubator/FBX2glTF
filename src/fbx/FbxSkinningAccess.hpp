/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
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

struct FbxVertexSkinningInfo {
  int jointId;
  float weight;
};


class FbxSkinningAccess {
 public:

  FbxSkinningAccess(const FbxMesh* pMesh, FbxScene* pScene, FbxNode* pNode);

  bool IsSkinned() const {
    return (vertexSkinning.size() > 0);
  }


  int GetNodeCount() const {
    return (int)jointNodes.size();
  }

  FbxNode* GetJointNode(const int jointIndex) const {
    return jointNodes[jointIndex];
  }

  const uint64_t GetJointId(const int jointIndex) const {
    return jointIds[jointIndex];
  }

  const FbxMatrix& GetJointSkinningTransform(const int jointIndex) const {
    return jointSkinningTransforms[jointIndex];
  }

  const FbxMatrix& GetJointInverseGlobalTransforms(const int jointIndex) const {
    return jointInverseGlobalTransforms[jointIndex];
  }

  const uint64_t GetRootNode() const {
    assert(rootIndex != -1);
    return jointIds[rootIndex];
  }

  const FbxAMatrix& GetInverseBindMatrix(const int jointIndex) const {
    return inverseBindMatrices[jointIndex];
  }

  const std::vector<FbxVertexSkinningInfo> GetVertexSkinningInfo(const int controlPointIndex) const {
    return vertexSkinning[controlPointIndex];
  }

 private:
  int rootIndex;
  int maxBoneInfluences;
  std::vector<long> jointIds;
  std::vector<FbxNode*> jointNodes;
  std::vector<FbxMatrix> jointSkinningTransforms;
  std::vector<FbxMatrix> jointInverseGlobalTransforms;
  std::vector<FbxAMatrix> inverseBindMatrices;
  std::vector<std::vector<FbxVertexSkinningInfo>> vertexSkinning;
};
