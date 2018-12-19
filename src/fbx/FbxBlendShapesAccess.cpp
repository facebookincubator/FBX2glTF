/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "FbxBlendShapesAccess.hpp"

FbxBlendShapesAccess::TargetShape::TargetShape(const FbxShape* shape, FbxDouble fullWeight)
    : shape(shape),
      fullWeight(fullWeight),
      count(shape->GetControlPointsCount()),
      positions(shape->GetControlPoints()),
      normals(FbxLayerElementAccess<FbxVector4>(
          shape->GetElementNormal(),
          shape->GetElementNormalCount())),
      tangents(FbxLayerElementAccess<FbxVector4>(
          shape->GetElementTangent(),
          shape->GetElementTangentCount())) {}

FbxAnimCurve* FbxBlendShapesAccess::BlendChannel::ExtractAnimation(unsigned int animIx) const {
  FbxAnimStack* stack = mesh->GetScene()->GetSrcObject<FbxAnimStack>(animIx);
  FbxAnimLayer* layer = stack->GetMember<FbxAnimLayer>(0);
  return mesh->GetShapeChannel(blendShapeIx, channelIx, layer, true);
}

FbxBlendShapesAccess::BlendChannel::BlendChannel(
    FbxMesh* mesh,
    const unsigned int blendShapeIx,
    const unsigned int channelIx,
    const FbxDouble deformPercent,
    const std::vector<FbxBlendShapesAccess::TargetShape>& targetShapes,
    std::string name)
    : mesh(mesh),
      blendShapeIx(blendShapeIx),
      channelIx(channelIx),
      deformPercent(deformPercent),
      targetShapes(targetShapes),
      name(name) {}

std::vector<FbxBlendShapesAccess::BlendChannel> FbxBlendShapesAccess::extractChannels(
    FbxMesh* mesh) const {
  std::vector<BlendChannel> channels;
  for (int shapeIx = 0; shapeIx < mesh->GetDeformerCount(FbxDeformer::eBlendShape); shapeIx++) {
    auto* fbxBlendShape =
        static_cast<FbxBlendShape*>(mesh->GetDeformer(shapeIx, FbxDeformer::eBlendShape));

    for (int channelIx = 0; channelIx < fbxBlendShape->GetBlendShapeChannelCount(); ++channelIx) {
      FbxBlendShapeChannel* fbxChannel = fbxBlendShape->GetBlendShapeChannel(channelIx);

      if (fbxChannel->GetTargetShapeCount() > 0) {
        std::vector<TargetShape> targetShapes;
        const double* fullWeights = fbxChannel->GetTargetShapeFullWeights();
        std::string name = std::string(fbxChannel->GetName());

        if (verboseOutput) {
          fmt::printf("\rblendshape channel: %s\n", name);
        }

        for (int targetIx = 0; targetIx < fbxChannel->GetTargetShapeCount(); targetIx++) {
          FbxShape* fbxShape = fbxChannel->GetTargetShape(targetIx);
          targetShapes.emplace_back(fbxShape, fullWeights[targetIx]);
        }
        channels.emplace_back(
            mesh, shapeIx, channelIx, fbxChannel->DeformPercent * 0.01, targetShapes, name);
      }
    }
  }
  return channels;
}
