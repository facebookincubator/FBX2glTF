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
#include <vector>
#include <algorithm>

#include "FBX2glTF.h"
#include "FbxLayerElementAccess.hpp"

/**
 * At the FBX level, each Mesh can have a set of FbxBlendShape deformers; organisational units that contain no data
 * of their own. The actual deformation is determined by one or more FbxBlendShapeChannels, whose influences are all
 * additively applied to the mesh. In a simpler world, each such channel would extend each base vertex with alternate
 * position, and optionally normal and tangent.
 *
 * It's not quite so simple, though. We also have progressive morphing, where one logical morph actually consists of
 * several concrete ones, each applied in sequence. For us, this means each channel contains a sequence of FbxShapes
 * (aka target shape); these are the actual data-holding entities that provide the alternate vertex attributes. As such
 * a channel is given more weight, it moves from one target shape to another.
 *
 * The total number of alternate sets of attributes, then, is the total number of target shapes across all the channels
 * of all the blend shapes of the mesh.
 *
 * Each animation in the scene stack can yield one or zero FbxAnimCurves per channel (not target shape). We evaluate
 * these curves to get the weight of the channel: this weight is further introspected on to figure out which target
 * shapes we're currently interpolation between.
 */
class FbxBlendShapesAccess
{
public:
    /**
     * A target shape is on a 1:1 basis with the eventual glTF morph target, and is the object which contains the
     * actual morphed vertex data.
     */
    struct TargetShape
    {
        explicit TargetShape(const FbxShape *shape, FbxDouble fullWeight);

        const FbxShape                          *shape;
        const FbxDouble                         fullWeight;
        const unsigned int                      count;
        const FbxVector4                        *positions;
        const FbxLayerElementAccess<FbxVector4> normals;
        const FbxLayerElementAccess<FbxVector4> tangents;
    };

    /**
     * A channel collects a sequence (often of length 1) of target shapes.
     */
    struct BlendChannel
    {
        BlendChannel(
            FbxMesh *mesh,
            const unsigned int blendShapeIx,
            const unsigned int channelIx,
            const FbxDouble deformPercent,
            const std::vector<TargetShape> &targetShapes,
            const std::string name
        );

        FbxAnimCurve *ExtractAnimation(unsigned int animIx) const;

        FbxMesh *const mesh;

        const unsigned int                  blendShapeIx;
        const unsigned int                  channelIx;
        const std::vector<TargetShape> targetShapes;
        const std::string name;

        const FbxDouble deformPercent;
    };

    explicit FbxBlendShapesAccess(FbxMesh *mesh) :
        channels(extractChannels(mesh))
    { }

    size_t GetChannelCount() const { return channels.size(); }
    const BlendChannel &GetBlendChannel(size_t channelIx) const {
        return channels.at(channelIx);
    }

    size_t GetTargetShapeCount(size_t channelIx) const { return channels[channelIx].targetShapes.size(); }
    const TargetShape &GetTargetShape(size_t channelIx, size_t targetShapeIx) const {
        return channels.at(channelIx).targetShapes[targetShapeIx];
    }

    FbxAnimCurve * GetAnimation(size_t channelIx, size_t animIx) const {
        return channels.at(channelIx).ExtractAnimation(animIx);
    }

private:
    std::vector<BlendChannel> extractChannels(FbxMesh *mesh) const;

    const std::vector<BlendChannel> channels;
};
