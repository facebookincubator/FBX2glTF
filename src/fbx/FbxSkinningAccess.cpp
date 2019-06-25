/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FbxSkinningAccess.hpp"

FbxSkinningAccess::FbxSkinningAccess(const FbxMesh* pMesh, FbxScene* pScene, FbxNode* pNode)
    : rootIndex(-1) {
  for (int deformerIndex = 0; deformerIndex < pMesh->GetDeformerCount(); deformerIndex++) {
    FbxSkin* skin =
        reinterpret_cast<FbxSkin*>(pMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
    if (skin != nullptr) {
      const int clusterCount = skin->GetClusterCount();
      if (clusterCount == 0) {
        continue;
      }
      int controlPointCount = pMesh->GetControlPointsCount();
      vertexSkinning.resize(controlPointCount);

      for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
        FbxCluster* cluster = skin->GetCluster(clusterIndex);
        const int indexCount = cluster->GetControlPointIndicesCount();
        const int* clusterIndices = cluster->GetControlPointIndices();
        const double* clusterWeights = cluster->GetControlPointWeights();

        assert(
            cluster->GetLinkMode() == FbxCluster::eNormalize ||
            cluster->GetLinkMode() == FbxCluster::eTotalOne);

        // Transform link matrix.
        FbxAMatrix transformLinkMatrix;
        cluster->GetTransformLinkMatrix(transformLinkMatrix);

        // The transformation of the mesh at binding time
        FbxAMatrix transformMatrix;
        cluster->GetTransformMatrix(transformMatrix);

        // Inverse bind matrix.
        FbxAMatrix globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix;
        inverseBindMatrices.emplace_back(globalBindposeInverseMatrix);

        jointNodes.push_back(cluster->GetLink());
        jointIds.push_back(cluster->GetLink()->GetUniqueID());

        const FbxAMatrix globalNodeTransform = cluster->GetLink()->EvaluateGlobalTransform();
        jointSkinningTransforms.push_back(
            FbxMatrix(globalNodeTransform * globalBindposeInverseMatrix));
        jointInverseGlobalTransforms.push_back(FbxMatrix(globalNodeTransform.Inverse()));

        for (int i = 0; i < indexCount; i++) {
          if (clusterIndices[i] < 0 || clusterIndices[i] >= controlPointCount) {
            continue;
          }
          if (clusterWeights[i] <= 0.0) {
            continue;
          }

          vertexSkinning[clusterIndices[i]].push_back(FbxVertexSkinningInfo{(int) clusterIndex, (float)clusterWeights[i]});

        }
      }
      
      for (int i = 0; i < vertexSkinning.size(); i++)
        maxBoneInfluences = std::max((int) vertexSkinning[i].size(), maxBoneInfluences);

    }
  }

  rootIndex = -1;
  for (size_t i = 0; i < jointNodes.size() && rootIndex == -1; i++) {
    rootIndex = (int)i;
    FbxNode* parent = jointNodes[i]->GetParent();
    if (parent == nullptr) {
      break;
    }
    for (size_t j = 0; j < jointNodes.size(); j++) {
      if (jointNodes[j] == parent) {
        rootIndex = -1;
        break;
      }
    }
  }
}
