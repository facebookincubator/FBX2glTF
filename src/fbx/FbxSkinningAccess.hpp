/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <fstream>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "FBX2glTF.h"

class FbxSkinningAccess
{
public:

    static const int MAX_WEIGHTS = 4;

    FbxSkinningAccess(const FbxMesh *pMesh, FbxScene *pScene, FbxNode *pNode);

    bool IsSkinned() const
    {
        return (vertexJointWeights.size() > 0);
    }

    int GetNodeCount() const
    {
        return (int) jointNodes.size();
    }

    FbxNode *GetJointNode(const int jointIndex) const
    {
        return jointNodes[jointIndex];
    }

    const long GetJointId(const int jointIndex) const
    {
        return jointIds[jointIndex];
    }

    const FbxMatrix &GetJointSkinningTransform(const int jointIndex) const
    {
        return jointSkinningTransforms[jointIndex];
    }

    const FbxMatrix &GetJointInverseGlobalTransforms(const int jointIndex) const
    {
        return jointInverseGlobalTransforms[jointIndex];
    }

    const long GetRootNode() const
    {
        assert(rootIndex != -1);
        return jointIds[rootIndex];
    }

    const FbxAMatrix &GetInverseBindMatrix(const int jointIndex) const
    {
        return inverseBindMatrices[jointIndex];
    }

    const Vec4i GetVertexIndices(const int controlPointIndex) const
    {
        return (!vertexJointIndices.empty()) ?
               vertexJointIndices[controlPointIndex] : Vec4i(0, 0, 0, 0);
    }

    const Vec4f GetVertexWeights(const int controlPointIndex) const
    {
        return (!vertexJointWeights.empty()) ?
               vertexJointWeights[controlPointIndex] : Vec4f(0, 0, 0, 0);
    }

private:
    int                     rootIndex;
    std::vector<long>       jointIds;
    std::vector<FbxNode *>  jointNodes;
    std::vector<FbxMatrix>  jointSkinningTransforms;
    std::vector<FbxMatrix>  jointInverseGlobalTransforms;
    std::vector<FbxAMatrix> inverseBindMatrices;
    std::vector<Vec4i>      vertexJointIndices;
    std::vector<Vec4f>      vertexJointWeights;
};
