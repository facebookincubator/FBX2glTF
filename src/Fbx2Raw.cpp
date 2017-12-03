/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <cassert>

#include "FBX2glTF.h"
#include "utils/File_Utils.h"
#include "utils/String_Utils.h"
#include "RawModel.h"
#include "Fbx2Raw.h"

extern bool verboseOutput;

template<typename _type_>
class FbxLayerElementAccess
{
public:

    FbxLayerElementAccess(const FbxLayerElementTemplate<_type_> *layer, int count) :
        mappingMode(FbxGeometryElement::eNone),
        elements(nullptr),
        indices(nullptr)
    {
        if (count <= 0 || layer == nullptr) {
            return;
        }
        const FbxGeometryElement::EMappingMode newMappingMode = layer->GetMappingMode();
        if (newMappingMode == FbxGeometryElement::eByControlPoint ||
            newMappingMode == FbxGeometryElement::eByPolygonVertex ||
            newMappingMode == FbxGeometryElement::eByPolygon) {
            mappingMode = newMappingMode;
            elements    = &layer->GetDirectArray();
            indices     = (
                layer->GetReferenceMode() == FbxGeometryElement::eIndexToDirect ||
                layer->GetReferenceMode() == FbxGeometryElement::eIndex) ? &layer->GetIndexArray() : nullptr;
        }
    }

    bool LayerPresent() const
    {
        return (mappingMode != FbxGeometryElement::eNone);
    }

    _type_ GetElement(const int polygonIndex, const int polygonVertexIndex, const int controlPointIndex, const _type_ defaultValue) const
    {
        if (mappingMode != FbxGeometryElement::eNone) {
            int index = (mappingMode == FbxGeometryElement::eByControlPoint) ? controlPointIndex :
                        ((mappingMode == FbxGeometryElement::eByPolygonVertex) ? polygonVertexIndex : polygonIndex);
            index = (indices != nullptr) ? (*indices)[index] : index;
            _type_ element = elements->GetAt(index);
            return element;
        }
        return defaultValue;
    }

    _type_ GetElement(
        const int polygonIndex, const int polygonVertexIndex, const int controlPointIndex, const _type_ defaultValue,
        const FbxMatrix &transform, const bool normalize) const
    {
        if (mappingMode != FbxGeometryElement::eNone) {
            _type_ element = transform.MultNormalize(GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, defaultValue));
            if (normalize) {
                element.Normalize();
            }
            return element;
        }
        return defaultValue;
    }

private:
    FbxGeometryElement::EMappingMode           mappingMode;
    const FbxLayerElementArrayTemplate<_type_> *elements;
    const FbxLayerElementArrayTemplate<int>    *indices;
};

struct FbxMaterialInfo {
    FbxMaterialInfo(const FbxString &name, const FbxString &shadingModel)
        : name(name),
          shadingModel(shadingModel)
    {}
    const FbxString name;
    const FbxString shadingModel;
};

struct FbxRoughMetMaterialInfo : FbxMaterialInfo {
    static constexpr const char *FBX_SHADER_METROUGH = "MetallicRoughness";

    FbxRoughMetMaterialInfo(const FbxString &name, const FbxString &shadingModel)
        : FbxMaterialInfo(name, shadingModel)
    {}
    const FbxFileTexture *texColor {};
    FbxVector4           colBase {};
    const FbxFileTexture *texNormal {};
    const FbxFileTexture *texMetallic {};
    FbxDouble            metallic {};
    const FbxFileTexture *texRoughness {};
    FbxDouble            roughness {};
    const FbxFileTexture *texEmissive {};
    FbxVector4           colEmissive {};
    FbxDouble            emissiveIntensity {};
    const FbxFileTexture *texAmbientOcclusion {};

    static std::unique_ptr<FbxRoughMetMaterialInfo> From(
        FbxSurfaceMaterial *fbxMaterial,
        const std::map<const FbxTexture *, FbxString> &textureLocations)
    {
        std::unique_ptr<FbxRoughMetMaterialInfo> res(new FbxRoughMetMaterialInfo(fbxMaterial->GetName(), FBX_SHADER_METROUGH));

        const FbxProperty mayaProp = fbxMaterial->FindProperty("Maya");
        if (mayaProp.GetPropertyDataType() != FbxCompoundDT) {
            return nullptr;
        }
        if (!fbxMaterial->ShadingModel.Get().IsEmpty()) {
            fmt::printf("Warning: Material %s has surprising shading model: %s\n",
                fbxMaterial->GetName(), fbxMaterial->ShadingModel.Get());
        }

        auto getTex = [&](std::string propName) {
            const FbxFileTexture *ptr = nullptr;

            const FbxProperty useProp = mayaProp.FindHierarchical(("use_" + propName + "_map").c_str());
            if (useProp.IsValid() && useProp.Get<bool>()) {
                const FbxProperty texProp = mayaProp.FindHierarchical(("TEX_" + propName + "_map").c_str());
                if (texProp.IsValid()) {
                    ptr = texProp.GetSrcObject<FbxFileTexture>();
                    if (ptr != nullptr && textureLocations.find(ptr) == textureLocations.end()) {
                        ptr = nullptr;
                    }
                }
            } else if (verboseOutput && useProp.IsValid()) {
                fmt::printf("Note: Property '%s' of material '%s' exists, but is flagged as 'do not use'.\n",
                    propName, fbxMaterial->GetName());
            }
            return ptr;
        };

        auto getVec = [&](std::string propName) -> FbxDouble3 {
            const FbxProperty vecProp = mayaProp.FindHierarchical(propName.c_str());
            return vecProp.IsValid() ? vecProp.Get<FbxDouble3>() : FbxDouble3(1, 1, 1);
        };

        auto getVal = [&](std::string propName) -> FbxDouble {
            const FbxProperty vecProp = mayaProp.FindHierarchical(propName .c_str());
            return vecProp.IsValid() ? vecProp.Get<FbxDouble>() : 0;
        };

        res->texNormal = getTex("normal");
        res->texColor = getTex("color");
        res->colBase = getVec("base_color");
        res->texAmbientOcclusion = getTex("ao");
        res->texEmissive = getTex("emissive");
        res->colEmissive = getVec("emissive");
        res->emissiveIntensity = getVal("emissive_intensity");
        res->texMetallic = getTex("metallic");
        res->metallic = getVal("metallic");
        res->texRoughness = getTex("roughness");
        res->roughness = getVal("roughness");

        return res;
    }
};

struct FbxTraditionalMaterialInfo : FbxMaterialInfo {
    static constexpr const char *FBX_SHADER_LAMBERT = "Lambert";
    static constexpr const char *FBX_SHADER_BLINN   = "Blinn";
    static constexpr const char *FBX_SHADER_PHONG   = "Phong";

    FbxTraditionalMaterialInfo(const FbxString &name, const FbxString &shadingModel)
        : FbxMaterialInfo(name, shadingModel)
    {}

    FbxFileTexture *texAmbient {};
    FbxVector4     colAmbient {};
    FbxFileTexture *texSpecular {};
    FbxVector4     colSpecular {};
    FbxFileTexture *texDiffuse {};
    FbxVector4     colDiffuse {};
    FbxFileTexture *texEmissive {};
    FbxVector4     colEmissive {};
    FbxFileTexture *texNormal {};
    FbxFileTexture *texShininess {};
    FbxDouble      shininess {};

    static std::unique_ptr<FbxTraditionalMaterialInfo> From(
        FbxSurfaceMaterial *fbxMaterial,
        const std::map<const FbxTexture *, FbxString> &textureLocations)
    {
        auto getSurfaceScalar = [&](const char *propName) -> std::tuple<FbxDouble, FbxFileTexture *> {
            const FbxProperty prop = fbxMaterial->FindProperty(propName);

            FbxDouble val(0);
            FbxFileTexture *tex = prop.GetSrcObject<FbxFileTexture>();
            if (tex != nullptr && textureLocations.find(tex) == textureLocations.end()) {
                tex = nullptr;
            }
            if (tex == nullptr && prop.IsValid()) {
                val = prop.Get<FbxDouble>();
            }
            return std::make_tuple(val, tex);
        };

        auto getSurfaceVector = [&](const char *propName) -> std::tuple<FbxDouble3, FbxFileTexture *> {
            const FbxProperty prop = fbxMaterial->FindProperty(propName);

            FbxDouble3 val(1, 1, 1);
            FbxFileTexture *tex = prop.GetSrcObject<FbxFileTexture>();
            if (tex != nullptr && textureLocations.find(tex) == textureLocations.end()) {
                tex = nullptr;
            }
            if (tex == nullptr && prop.IsValid()) {
                val = prop.Get<FbxDouble3>();
            }
            return std::make_tuple(val, tex);
        };

        auto getSurfaceValues = [&](const char *colName, const char *facName) -> std::tuple<FbxVector4, FbxFileTexture *, FbxFileTexture *> {
            const FbxProperty colProp = fbxMaterial->FindProperty(colName);
            const FbxProperty facProp = fbxMaterial->FindProperty(facName);

            FbxDouble3 colorVal(1, 1, 1);
            FbxDouble  factorVal(1);

            FbxFileTexture *colTex = colProp.GetSrcObject<FbxFileTexture>();
            if (colTex != nullptr && textureLocations.find(colTex) == textureLocations.end()) {
                colTex = nullptr;
            }
            if (colTex == nullptr && colProp.IsValid()) {
                colorVal = colProp.Get<FbxDouble3>();
            }
            FbxFileTexture *facTex = facProp.GetSrcObject<FbxFileTexture>();
            if (facTex != nullptr && textureLocations.find(facTex) == textureLocations.end()) {
                facTex = nullptr;
            }
            if (facTex == nullptr && facProp.IsValid()) {
                factorVal = facProp.Get<FbxDouble>();
            }

            auto val = FbxVector4(
                colorVal[0] * factorVal,
                colorVal[1] * factorVal,
                colorVal[2] * factorVal,
                factorVal);
            return std::make_tuple(val, colTex, facTex);
        };

        std::string name = fbxMaterial->GetName();
        std::unique_ptr<FbxTraditionalMaterialInfo> res(new FbxTraditionalMaterialInfo(name.c_str(), fbxMaterial->ShadingModel.Get()));

        // four properties are on the same structure and follow the same rules
        auto handleBasicProperty = [&](const char *colName, const char *facName) -> std::tuple<FbxVector4, FbxFileTexture *>{
            FbxFileTexture *colTex, *facTex;
            FbxVector4     vec;

            std::tie(vec, colTex, facTex) = getSurfaceValues(colName, facName);
            if (colTex) {
                if (facTex) {
                    fmt::printf("Warning: Mat [%s]: Can't handle both %s and %s textures; discarding %s.\n", name, colName, facName, facName);
                }
                return std::make_tuple(vec, colTex);
            }
            return std::make_tuple(vec, facTex);
        };

        std::tie(res->colAmbient, res->texAmbient) =
            handleBasicProperty(FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor);
        std::tie(res->colSpecular, res->texSpecular) =
            handleBasicProperty(FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor);
        std::tie(res->colDiffuse, res->texDiffuse) =
            handleBasicProperty(FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor);
        std::tie(res->colEmissive, res->texEmissive) =
            handleBasicProperty(FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor);

        // the normal map can only ever be a map, ignore everything else
        std::tie(std::ignore, res->texNormal) = getSurfaceVector(FbxSurfaceMaterial::sNormalMap);

        // shininess can be a map or a factor
        std::tie(res->shininess, res->texShininess) = getSurfaceScalar(FbxSurfaceMaterial::sShininess);

        // for transparency we just want a constant vector value;
        FbxVector4 transparency;
        // extract any existing textures only so we can warn that we're throwing them away
        FbxFileTexture *colTex, *facTex;
        std::tie(transparency, colTex, facTex) =
            getSurfaceValues(FbxSurfaceMaterial::sTransparentColor, FbxSurfaceMaterial::sTransparencyFactor);
        if (colTex) {
            fmt::printf("Warning: Mat [%s]: Can't handle texture for %s; discarding.\n", name, FbxSurfaceMaterial::sTransparentColor);
        }
        if (facTex) {
            fmt::printf("Warning: Mat [%s]: Can't handle texture for %s; discarding.\n", name, FbxSurfaceMaterial::sTransparencyFactor);
        }
        // FBX color is RGB, so we calculate the A channel as the average of the FBX transparency color vector
        res->colDiffuse[3] = 1.0 - (transparency[0] + transparency[1] + transparency[2])/3.0;

        return res;
    }
};


std::unique_ptr<FbxMaterialInfo>
GetMaterialInfo(FbxSurfaceMaterial *material, const std::map<const FbxTexture *, FbxString> &textureLocations)
{
    std::unique_ptr<FbxMaterialInfo> res;
    res = FbxRoughMetMaterialInfo::From(material, textureLocations);
    if (!res) {
        res = FbxTraditionalMaterialInfo::From(material, textureLocations);
    }
    return res;
}

class FbxMaterialsAccess
{
public:

    FbxMaterialsAccess(const FbxMesh *pMesh, const std::map<const FbxTexture *, FbxString> &textureLocations) :
        mappingMode(FbxGeometryElement::eNone),
        mesh(nullptr),
        indices(nullptr)
    {
        if (pMesh->GetElementMaterialCount() <= 0) {
            return;
        }

        const FbxGeometryElement::EMappingMode materialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
        if (materialMappingMode != FbxGeometryElement::eByPolygon && materialMappingMode != FbxGeometryElement::eAllSame) {
            return;
        }

        const FbxGeometryElement::EReferenceMode materialReferenceMode = pMesh->GetElementMaterial()->GetReferenceMode();
        if (materialReferenceMode != FbxGeometryElement::eIndexToDirect) {
            return;
        }

        mappingMode = materialMappingMode;
        mesh        = pMesh;
        indices     = &pMesh->GetElementMaterial()->GetIndexArray();

        for (int ii = 0; ii < indices->GetCount(); ii++) {
            int materialNum = indices->GetAt(ii);
            if (materialNum < 0) {
                continue;
            }
            if (materialNum >= summaries.size()) {
                summaries.resize(materialNum + 1);
            }
            auto summary = summaries[materialNum];
            if (summary == nullptr) {
                summary = summaries[materialNum] = GetMaterialInfo(
                    mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(materialNum),
                    textureLocations);
            }
        }
    }

    const std::shared_ptr<FbxMaterialInfo> GetMaterial(const int polygonIndex) const
    {
        if (mappingMode != FbxGeometryElement::eNone) {
            const int materialNum = indices->GetAt((mappingMode == FbxGeometryElement::eByPolygon) ? polygonIndex : 0);
            if (materialNum < 0) {
                return nullptr;
            }
            return summaries.at((unsigned long) materialNum);
        }
        return nullptr;
    }

private:
    FbxGeometryElement::EMappingMode              mappingMode;
    std::vector<std::shared_ptr<FbxMaterialInfo>> summaries {};
    const FbxMesh                                 *mesh;
    const FbxLayerElementArrayTemplate<int>       *indices;
};

class FbxSkinningAccess
{
public:

    static const int MAX_WEIGHTS = 4;

    FbxSkinningAccess(const FbxMesh *pMesh, FbxScene *pScene, FbxNode *pNode)
        : rootIndex(-1)
    {
        for (int deformerIndex = 0; deformerIndex < pMesh->GetDeformerCount(); deformerIndex++) {
            FbxSkin *skin = reinterpret_cast< FbxSkin * >( pMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
            if (skin != nullptr) {
                int controlPointCount = pMesh->GetControlPointsCount();

                vertexJointIndices.resize(controlPointCount, Vec4i(0, 0, 0, 0));
                vertexJointWeights.resize(controlPointCount, Vec4f(0.0f, 0.0f, 0.0f, 0.0f));

                const int clusterCount = skin->GetClusterCount();
                for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
                    FbxCluster   *cluster        = skin->GetCluster(clusterIndex);
                    const int    indexCount      = cluster->GetControlPointIndicesCount();
                    const int    *clusterIndices = cluster->GetControlPointIndices();
                    const double *clusterWeights = cluster->GetControlPointWeights();

                    assert(cluster->GetLinkMode() == FbxCluster::eNormalize);

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
                    jointNames.push_back(*cluster->GetLink()->GetName() != '\0' ? cluster->GetLink()->GetName() : cluster->GetName());

                    const FbxAMatrix globalNodeTransform = cluster->GetLink()->EvaluateGlobalTransform();
                    jointSkinningTransforms.push_back(FbxMatrix(globalNodeTransform * globalBindposeInverseMatrix));
                    jointInverseGlobalTransforms.push_back(FbxMatrix(globalNodeTransform.Inverse()));

                    for (int i = 0; i < indexCount; i++) {
                        if (clusterIndices[i] < 0 || clusterIndices[i] >= controlPointCount) {
                            continue;
                        }
                        if (clusterWeights[i] <= vertexJointWeights[clusterIndices[i]][MAX_WEIGHTS - 1]) {
                            continue;
                        }
                        vertexJointIndices[clusterIndices[i]][MAX_WEIGHTS - 1] = clusterIndex;
                        vertexJointWeights[clusterIndices[i]][MAX_WEIGHTS - 1] = (float) clusterWeights[i];
                        for (int j = MAX_WEIGHTS - 1; j > 0; j--) {
                            if (vertexJointWeights[clusterIndices[i]][j - 1] >= vertexJointWeights[clusterIndices[i]][j]) {
                                break;
                            }
                            std::swap(vertexJointIndices[clusterIndices[i]][j - 1], vertexJointIndices[clusterIndices[i]][j]);
                            std::swap(vertexJointWeights[clusterIndices[i]][j - 1], vertexJointWeights[clusterIndices[i]][j]);
                        }
                    }

                }
                for (int i = 0; i < controlPointCount; i++) {
                    vertexJointWeights[i] = vertexJointWeights[i].Normalized();
                }
            }
        }

        rootIndex = -1;
        for (size_t i = 0; i < jointNodes.size() && rootIndex == -1; i++) {
            rootIndex = (int) i;
            FbxNode *parent = jointNodes[i]->GetParent();
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

    const char *GetJointName(const int jointIndex) const
    {
        return jointNames[jointIndex].c_str();
    }

    const FbxMatrix &GetJointSkinningTransform(const int jointIndex) const
    {
        return jointSkinningTransforms[jointIndex];
    }

    const FbxMatrix &GetJointInverseGlobalTransforms(const int jointIndex) const
    {
        return jointInverseGlobalTransforms[jointIndex];
    }

    const char *GetRootNode() const
    {
        assert(rootIndex != -1);
        return jointNames[rootIndex].c_str();
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
    int                      rootIndex;
    std::vector<std::string> jointNames;
    std::vector<FbxNode *>   jointNodes;
    std::vector<FbxMatrix>   jointSkinningTransforms;
    std::vector<FbxMatrix>   jointInverseGlobalTransforms;
    std::vector<FbxAMatrix>  inverseBindMatrices;
    std::vector<Vec4i>       vertexJointIndices;
    std::vector<Vec4f>       vertexJointWeights;
};

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
        explicit TargetShape(const FbxShape *shape, FbxDouble fullWeight) :
            shape(shape),
            fullWeight(fullWeight),
            count(shape->GetControlPointsCount()),
            positions(shape->GetControlPoints()),
            normals(FbxLayerElementAccess<FbxVector4>(shape->GetElementNormal(), shape->GetElementNormalCount())),
            tangents(FbxLayerElementAccess<FbxVector4>(shape->GetElementTangent(), shape->GetElementTangentCount()))
        {}

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
            const std::vector<TargetShape> &targetShapes
        ) : mesh(mesh),
            blendShapeIx(blendShapeIx),
            channelIx(channelIx),
            deformPercent(deformPercent),
            targetShapes(targetShapes)
        {}

        FbxAnimCurve *ExtractAnimation(unsigned int animIx) const {
            FbxAnimStack *stack = mesh->GetScene()->GetSrcObject<FbxAnimStack>(animIx);
            FbxAnimLayer *layer = stack->GetMember<FbxAnimLayer>(0);
            return mesh->GetShapeChannel(blendShapeIx, channelIx, layer, true);
        }

        FbxMesh *const mesh;

        const unsigned int blendShapeIx;
        const unsigned int channelIx;
        const std::vector<TargetShape> targetShapes;

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
    std::vector<BlendChannel> extractChannels(FbxMesh *mesh) const {
        std::vector<BlendChannel> channels;
        for (int shapeIx = 0; shapeIx < mesh->GetDeformerCount(FbxDeformer::eBlendShape); shapeIx++) {
            auto *fbxBlendShape = static_cast<FbxBlendShape *>(mesh->GetDeformer(shapeIx, FbxDeformer::eBlendShape));

            for (int channelIx = 0; channelIx < fbxBlendShape->GetBlendShapeChannelCount(); ++channelIx) {
                FbxBlendShapeChannel *fbxChannel = fbxBlendShape->GetBlendShapeChannel(channelIx);

                if (fbxChannel->GetTargetShapeCount() > 0) {
                    std::vector<TargetShape> targetShapes;
                    const double *fullWeights = fbxChannel->GetTargetShapeFullWeights();
                    for (int targetIx = 0; targetIx < fbxChannel->GetTargetShapeCount(); targetIx ++) {
                        FbxShape *fbxShape = fbxChannel->GetTargetShape(targetIx);
                        targetShapes.push_back(TargetShape(fbxShape, fullWeights[targetIx]));
                    }
                    channels.push_back(BlendChannel(mesh, shapeIx, channelIx, fbxChannel->DeformPercent * 0.01, targetShapes));
                }
            }
        }
        return channels;
    }

    const std::vector<BlendChannel> channels;
};

static bool TriangleTexturePolarity(const Vec2f &uv0, const Vec2f &uv1, const Vec2f &uv2)
{
    const Vec2f d0 = uv1 - uv0;
    const Vec2f d1 = uv2 - uv0;

    return (d0[0] * d1[1] - d0[1] * d1[0] < 0.0f);
}

static RawMaterialType
GetMaterialType(const RawModel &raw, const int textures[RAW_TEXTURE_USAGE_MAX], const bool vertexTransparency, const bool skinned)
{
    // If diffusely texture, determine material type based on texture occlusion.
    if (textures[RAW_TEXTURE_USAGE_DIFFUSE] >= 0) {
        switch (raw.GetTexture(textures[RAW_TEXTURE_USAGE_DIFFUSE]).occlusion) {
            case RAW_TEXTURE_OCCLUSION_OPAQUE:
                return skinned ? RAW_MATERIAL_TYPE_SKINNED_OPAQUE : RAW_MATERIAL_TYPE_OPAQUE;
            case RAW_TEXTURE_OCCLUSION_TRANSPARENT:
                return skinned ? RAW_MATERIAL_TYPE_SKINNED_TRANSPARENT : RAW_MATERIAL_TYPE_TRANSPARENT;
        }
    }

	// else if there is any vertex transparency, treat whole mesh as transparent
	if (vertexTransparency) {
		return skinned ? RAW_MATERIAL_TYPE_SKINNED_TRANSPARENT : RAW_MATERIAL_TYPE_TRANSPARENT;
	}


    // Default to simply opaque.
    return skinned ? RAW_MATERIAL_TYPE_SKINNED_OPAQUE : RAW_MATERIAL_TYPE_OPAQUE;
}

static void ReadMesh(RawModel &raw, FbxScene *pScene, FbxNode *pNode, const std::map<const FbxTexture *, FbxString> &textureLocations)
{
    FbxGeometryConverter meshConverter(pScene->GetFbxManager());
    meshConverter.Triangulate(pNode->GetNodeAttribute(), true);
    FbxMesh *pMesh = pNode->GetMesh();

    // Obtains the surface Id
    const long surfaceId = pMesh->GetUniqueID();

    // Associate the node to this surface
    int nodeId = raw.GetNodeByName(pNode->GetName());
    if (nodeId >= 0) {
        RawNode &node = raw.GetNode(nodeId);
        node.surfaceId = surfaceId;
    }

    if (raw.GetSurfaceById(surfaceId) >= 0) {
        // This surface is already loaded
        return;
    }

    const char *meshName = (pNode->GetName()[0] != '\0') ? pNode->GetName() : pMesh->GetName();
    const int rawSurfaceIndex = raw.AddSurface(meshName, surfaceId);

    const FbxVector4 *controlPoints = pMesh->GetControlPoints();
    const FbxLayerElementAccess<FbxVector4> normalLayer(pMesh->GetElementNormal(), pMesh->GetElementNormalCount());
    const FbxLayerElementAccess<FbxVector4> binormalLayer(pMesh->GetElementBinormal(), pMesh->GetElementBinormalCount());
    const FbxLayerElementAccess<FbxVector4> tangentLayer(pMesh->GetElementTangent(), pMesh->GetElementTangentCount());
    const FbxLayerElementAccess<FbxColor>   colorLayer(pMesh->GetElementVertexColor(), pMesh->GetElementVertexColorCount());
    const FbxLayerElementAccess<FbxVector2> uvLayer0(pMesh->GetElementUV(0), pMesh->GetElementUVCount());
    const FbxLayerElementAccess<FbxVector2> uvLayer1(pMesh->GetElementUV(1), pMesh->GetElementUVCount());
    const FbxSkinningAccess                 skinning(pMesh, pScene, pNode);
    const FbxMaterialsAccess                materials(pMesh, textureLocations);
    const FbxBlendShapesAccess              blendShapes(pMesh);

    if (verboseOutput) {
        fmt::printf(
            "mesh %d: %s (skinned: %s)\n", rawSurfaceIndex, meshName,
            skinning.IsSkinned() ? skinning.GetRootNode() : "NO");
    }

    // The FbxNode geometric transformation describes how a FbxNodeAttribute is offset from
    // the FbxNode's local frame of reference. These geometric transforms are applied to the
    // FbxNodeAttribute after the FbxNode's local transforms are computed, and are not
    // inherited across the node hierarchy.
    // Apply the geometric transform to the mesh geometry (vertices, normal etc.) because
    // glTF does not have an equivalent to the geometric transform.
    const FbxVector4 meshTranslation           = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 meshRotation              = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 meshScaling               = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
    const FbxAMatrix meshTransform(meshTranslation, meshRotation, meshScaling);
    const FbxMatrix  transform                 = meshTransform;

    // Remove translation & scaling from transforms that will bi applied to normals, tangents & binormals
    const FbxMatrix  normalTransform(FbxVector4(), meshRotation, meshScaling);
    const FbxMatrix  inverseTransposeTransform = normalTransform.Inverse().Transpose();

    raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_POSITION);
    if (normalLayer.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_NORMAL); }
    if (tangentLayer.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_TANGENT); }
    if (binormalLayer.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_BINORMAL); }
    if (colorLayer.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_COLOR); }
    if (uvLayer0.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_UV0); }
    if (uvLayer1.LayerPresent()) { raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_UV1); }
    if (skinning.IsSkinned()) {
        raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS);
        raw.AddVertexAttribute(RAW_VERTEX_ATTRIBUTE_JOINT_INDICES);
    }

    RawSurface &rawSurface = raw.GetSurface(rawSurfaceIndex);

    rawSurface.skeletonRootName = (skinning.IsSkinned()) ? skinning.GetRootNode() : pNode->GetName();
    for (int jointIndex = 0; jointIndex < skinning.GetNodeCount(); jointIndex++) {
        const char *jointName = skinning.GetJointName(jointIndex);
        raw.GetNode(raw.GetNodeByName(jointName)).isJoint = true;

        rawSurface.jointNames.emplace_back(jointName);
        rawSurface.inverseBindMatrices.push_back(toMat4f(skinning.GetInverseBindMatrix(jointIndex)));
        rawSurface.jointGeometryMins.emplace_back(FLT_MAX, FLT_MAX, FLT_MAX);
        rawSurface.jointGeometryMaxs.emplace_back(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    }

    rawSurface.blendChannels.clear();
    std::vector<const FbxBlendShapesAccess::TargetShape *> targetShapes;
    for (size_t channelIx = 0; channelIx < blendShapes.GetChannelCount(); channelIx ++) {
        for (size_t targetIx = 0; targetIx < blendShapes.GetTargetShapeCount(channelIx); targetIx ++) {
            const FbxBlendShapesAccess::TargetShape &shape = blendShapes.GetTargetShape(channelIx, targetIx);
            targetShapes.push_back(&shape);

            rawSurface.blendChannels.push_back(RawBlendChannel {
                static_cast<float>(blendShapes.GetBlendChannel(channelIx).deformPercent),
                shape.normals.LayerPresent(),
                shape.tangents.LayerPresent(),
            });
        }
    }

    int polygonVertexIndex = 0;

    for (int polygonIndex = 0; polygonIndex < pMesh->GetPolygonCount(); polygonIndex++) {
        FBX_ASSERT(pMesh->GetPolygonSize(polygonIndex) == 3);
        const std::shared_ptr<FbxMaterialInfo> fbxMaterial = materials.GetMaterial(polygonIndex);

        int textures[RAW_TEXTURE_USAGE_MAX];
        std::fill_n(textures, (int) RAW_TEXTURE_USAGE_MAX, -1);

        std::shared_ptr<RawMatProps> rawMatProps;
        FbxString materialName;

        if (fbxMaterial == nullptr) {
            materialName = "DefaultMaterial";
            rawMatProps.reset(new RawTraditionalMatProps(RAW_SHADING_MODEL_LAMBERT,
                Vec3f(0, 0, 0), Vec4f(.5, .5, .5, 1), Vec3f(0, 0, 0), Vec3f(0, 0, 0), 0.5));

        } else {
            materialName = fbxMaterial->name;

            const auto maybeAddTexture = [&](const FbxFileTexture *tex, RawTextureUsage usage) {
                if (tex != nullptr) {
                    // dig out the inferred filename from the textureLocations map
                    FbxString inferredPath = textureLocations.find(tex)->second;
                    textures[usage] = raw.AddTexture(tex->GetName(), tex->GetFileName(), inferredPath.Buffer(), usage);
                }
            };

            std::shared_ptr<RawMatProps> matInfo;
            if (fbxMaterial->shadingModel == FbxRoughMetMaterialInfo::FBX_SHADER_METROUGH) {
                FbxRoughMetMaterialInfo *fbxMatInfo = static_cast<FbxRoughMetMaterialInfo *>(fbxMaterial.get());

                maybeAddTexture(fbxMatInfo->texColor, RAW_TEXTURE_USAGE_ALBEDO);
                maybeAddTexture(fbxMatInfo->texNormal, RAW_TEXTURE_USAGE_NORMAL);
                maybeAddTexture(fbxMatInfo->texEmissive, RAW_TEXTURE_USAGE_EMISSIVE);
                maybeAddTexture(fbxMatInfo->texRoughness, RAW_TEXTURE_USAGE_ROUGHNESS);
                maybeAddTexture(fbxMatInfo->texMetallic, RAW_TEXTURE_USAGE_METALLIC);
                maybeAddTexture(fbxMatInfo->texAmbientOcclusion, RAW_TEXTURE_USAGE_OCCLUSION);
                rawMatProps.reset(new RawMetRoughMatProps(
                    RAW_SHADING_MODEL_PBR_MET_ROUGH, toVec4f(fbxMatInfo->colBase), toVec3f(fbxMatInfo->colEmissive),
                    fbxMatInfo->emissiveIntensity, fbxMatInfo->metallic, fbxMatInfo->roughness));
            } else {

                FbxTraditionalMaterialInfo *fbxMatInfo = static_cast<FbxTraditionalMaterialInfo *>(fbxMaterial.get());
                RawShadingModel shadingModel;
                if (fbxMaterial->shadingModel == "Lambert") {
                    shadingModel = RAW_SHADING_MODEL_LAMBERT;
                } else if (fbxMaterial->shadingModel == "Blinn") {
                    shadingModel = RAW_SHADING_MODEL_BLINN;
                } else if (fbxMaterial->shadingModel == "Phong") {
                    shadingModel = RAW_SHADING_MODEL_PHONG;
                } else if (fbxMaterial->shadingModel == "Constant") {
                    shadingModel = RAW_SHADING_MODEL_PHONG;
                } else {
                    shadingModel = RAW_SHADING_MODEL_UNKNOWN;
                }
                maybeAddTexture(fbxMatInfo->texDiffuse, RAW_TEXTURE_USAGE_DIFFUSE);
                maybeAddTexture(fbxMatInfo->texNormal, RAW_TEXTURE_USAGE_NORMAL);
                maybeAddTexture(fbxMatInfo->texEmissive, RAW_TEXTURE_USAGE_EMISSIVE);
                maybeAddTexture(fbxMatInfo->texShininess, RAW_TEXTURE_USAGE_SHININESS);
                maybeAddTexture(fbxMatInfo->texAmbient, RAW_TEXTURE_USAGE_AMBIENT);
                maybeAddTexture(fbxMatInfo->texSpecular, RAW_TEXTURE_USAGE_SPECULAR);
                rawMatProps.reset(new RawTraditionalMatProps(shadingModel,
                    toVec3f(fbxMatInfo->colAmbient), toVec4f(fbxMatInfo->colDiffuse), toVec3f(fbxMatInfo->colEmissive),
                    toVec3f(fbxMatInfo->colSpecular), fbxMatInfo->shininess));
            }
        }

        RawVertex rawVertices[3];
        bool vertexTransparency = false;
        for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++, polygonVertexIndex++) {
            const int controlPointIndex = pMesh->GetPolygonVertex(polygonIndex, vertexIndex);

            // Note that the default values here must be the same as the RawVertex default values!
            const FbxVector4 fbxPosition = transform.MultNormalize(controlPoints[controlPointIndex]);
            const FbxVector4 fbxNormal   = normalLayer.GetElement(
                polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
            const FbxVector4 fbxTangent  = tangentLayer.GetElement(
                polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
            const FbxVector4 fbxBinormal = binormalLayer.GetElement(
                polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
            const FbxColor   fbxColor    = colorLayer
                .GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, FbxColor(0.0f, 0.0f, 0.0f, 0.0f));
            const FbxVector2 fbxUV0      = uvLayer0.GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector2(0.0f, 0.0f));
            const FbxVector2 fbxUV1      = uvLayer1.GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector2(0.0f, 0.0f));

            RawVertex &vertex = rawVertices[vertexIndex];
            vertex.position[0]   = (float) fbxPosition[0];
            vertex.position[1]   = (float) fbxPosition[1];
            vertex.position[2]   = (float) fbxPosition[2];
            vertex.normal[0]     = (float) fbxNormal[0];
            vertex.normal[1]     = (float) fbxNormal[1];
            vertex.normal[2]     = (float) fbxNormal[2];
            vertex.tangent[0]    = (float) fbxTangent[0];
            vertex.tangent[1]    = (float) fbxTangent[1];
            vertex.tangent[2]    = (float) fbxTangent[2];
            vertex.tangent[3]    = (float) fbxTangent[3];
            vertex.binormal[0]   = (float) fbxBinormal[0];
            vertex.binormal[1]   = (float) fbxBinormal[1];
            vertex.binormal[2]   = (float) fbxBinormal[2];
            vertex.color[0]      = (float) fbxColor.mRed;
            vertex.color[1]      = (float) fbxColor.mGreen;
            vertex.color[2]      = (float) fbxColor.mBlue;
            vertex.color[3]      = (float) fbxColor.mAlpha;
            vertex.uv0[0]        = (float) fbxUV0[0];
            vertex.uv0[1]        = (float) fbxUV0[1];
            vertex.uv1[0]        = (float) fbxUV1[0];
            vertex.uv1[1]        = (float) fbxUV1[1];
            vertex.jointIndices = skinning.GetVertexIndices(controlPointIndex);
            vertex.jointWeights = skinning.GetVertexWeights(controlPointIndex);
            vertex.polarityUv0  = false;

            // flag this triangle as transparent if any of its corner vertices substantially deviates from fully opaque
            vertexTransparency |= colorLayer.LayerPresent() && (fabs(fbxColor.mAlpha - 1.0) > 1e-3);

            rawSurface.bounds.AddPoint(vertex.position);

            if (!targetShapes.empty()) {
                vertex.blendSurfaceIx = rawSurfaceIndex;
                for (const auto *targetShape : targetShapes) {
                    RawBlendVertex blendVertex;
                    // the morph target data must be transformed just as with the vertex positions above
                    const FbxVector4 &shapePosition = transform.MultNormalize(targetShape->positions[controlPointIndex]);
                    blendVertex.position = toVec3f(shapePosition - fbxPosition);
                    if (targetShape->normals.LayerPresent()) {
                        const FbxVector4 &normal = targetShape->normals.GetElement(
                            polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
                        blendVertex.normal = toVec3f(normal - fbxNormal);
                    }
                    if (targetShape->tangents.LayerPresent()) {
                        const FbxVector4 &tangent = targetShape->tangents.GetElement(
                            polygonIndex, polygonVertexIndex, controlPointIndex, FbxVector4(0.0f, 0.0f, 0.0f, 0.0f), inverseTransposeTransform, true);
                        blendVertex.tangent = toVec4f(tangent - fbxTangent);
                    }
                    vertex.blends.push_back(blendVertex);
                }
            } else {
                vertex.blendSurfaceIx = -1;
            }

            if (skinning.IsSkinned()) {
                const int jointIndices[FbxSkinningAccess::MAX_WEIGHTS] = {
                    vertex.jointIndices[0],
                    vertex.jointIndices[1],
                    vertex.jointIndices[2],
                    vertex.jointIndices[3]
                };
                const float jointWeights[FbxSkinningAccess::MAX_WEIGHTS] = {
                    vertex.jointWeights[0],
                    vertex.jointWeights[1],
                    vertex.jointWeights[2],
                    vertex.jointWeights[3]
                };
                const FbxMatrix skinningMatrix =
                    skinning.GetJointSkinningTransform(jointIndices[0]) * jointWeights[0] +
                    skinning.GetJointSkinningTransform(jointIndices[1]) * jointWeights[1] +
                    skinning.GetJointSkinningTransform(jointIndices[2]) * jointWeights[2] +
                    skinning.GetJointSkinningTransform(jointIndices[3]) * jointWeights[3];

                const FbxVector4 globalPosition = skinningMatrix.MultNormalize(fbxPosition);
                for (int i = 0; i < FbxSkinningAccess::MAX_WEIGHTS; i++) {
                    if (jointWeights[i] > 0.0f) {
                        const FbxVector4 localPosition =
                            skinning.GetJointInverseGlobalTransforms(jointIndices[i]).MultNormalize(globalPosition);

                        Vec3f &mins = rawSurface.jointGeometryMins[jointIndices[i]];
                        mins[0] = std::min(mins[0], (float) localPosition[0]);
                        mins[1] = std::min(mins[1], (float) localPosition[1]);
                        mins[2] = std::min(mins[2], (float) localPosition[2]);

                        Vec3f &maxs = rawSurface.jointGeometryMaxs[jointIndices[i]];
                        maxs[0] = std::max(maxs[0], (float) localPosition[0]);
                        maxs[1] = std::max(maxs[1], (float) localPosition[1]);
                        maxs[2] = std::max(maxs[2], (float) localPosition[2]);
                    }
                }
            }
        }

        if (textures[RAW_TEXTURE_USAGE_NORMAL] != -1) {
            // Distinguish vertices that are used by triangles with a different texture polarity to avoid degenerate tangent space smoothing.
            const bool polarity = TriangleTexturePolarity(rawVertices[0].uv0, rawVertices[1].uv0, rawVertices[2].uv0);
            rawVertices[0].polarityUv0 = polarity;
            rawVertices[1].polarityUv0 = polarity;
            rawVertices[2].polarityUv0 = polarity;
        }

        int rawVertexIndices[3];
        for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
            rawVertexIndices[vertexIndex] = raw.AddVertex(rawVertices[vertexIndex]);
        }

        const RawMaterialType materialType = GetMaterialType(raw, textures, vertexTransparency, skinning.IsSkinned());
        const int rawMaterialIndex = raw.AddMaterial(materialName, materialType, textures, rawMatProps);

        raw.AddTriangle(rawVertexIndices[0], rawVertexIndices[1], rawVertexIndices[2], rawMaterialIndex, rawSurfaceIndex);
    }
}

static void ReadCamera(RawModel &raw, FbxScene *pScene, FbxNode *pNode)
{
    const FbxCamera *pCamera = pNode->GetCamera();
    if (pCamera->ProjectionType.Get() == FbxCamera::EProjectionType::ePerspective) {
        raw.AddCameraPerspective(
            "", pNode->GetName(), (float) pCamera->FilmAspectRatio,
            (float) pCamera->FieldOfViewX, (float) pCamera->FieldOfViewX,
            (float) pCamera->NearPlane, (float) pCamera->FarPlane);
    } else {
        raw.AddCameraOrthographic(
            "", pNode->GetName(),
            (float) pCamera->OrthoZoom, (float) pCamera->OrthoZoom,
            (float) pCamera->FarPlane, (float) pCamera->NearPlane);
    }
}

static void ReadNodeAttributes(
    RawModel &raw, FbxScene *pScene, FbxNode *pNode, const std::map<const FbxTexture *, FbxString> &textureLocations)
{
    if (!pNode->GetVisibility()) {
        return;
    }

    FbxNodeAttribute *pNodeAttribute = pNode->GetNodeAttribute();
    if (pNodeAttribute != nullptr) {
        const FbxNodeAttribute::EType attributeType = pNodeAttribute->GetAttributeType();
        switch (attributeType) {
            case FbxNodeAttribute::eMesh:
            case FbxNodeAttribute::eNurbs:
            case FbxNodeAttribute::eNurbsSurface:
            case FbxNodeAttribute::eTrimNurbsSurface:
            case FbxNodeAttribute::ePatch: {
                ReadMesh(raw, pScene, pNode, textureLocations);
                break;
            }
            case FbxNodeAttribute::eCamera: {
                ReadCamera(raw, pScene, pNode);
                break;
            }
            case FbxNodeAttribute::eUnknown:
            case FbxNodeAttribute::eNull:
            case FbxNodeAttribute::eMarker:
            case FbxNodeAttribute::eSkeleton:
            case FbxNodeAttribute::eCameraStereo:
            case FbxNodeAttribute::eCameraSwitcher:
            case FbxNodeAttribute::eLight:
            case FbxNodeAttribute::eOpticalReference:
            case FbxNodeAttribute::eOpticalMarker:
            case FbxNodeAttribute::eNurbsCurve:
            case FbxNodeAttribute::eBoundary:
            case FbxNodeAttribute::eShape:
            case FbxNodeAttribute::eLODGroup:
            case FbxNodeAttribute::eSubDiv:
            case FbxNodeAttribute::eCachedEffect:
            case FbxNodeAttribute::eLine: {
                break;
            }
        }
    }

    for (int child = 0; child < pNode->GetChildCount(); child++) {
        ReadNodeAttributes(raw, pScene, pNode->GetChild(child), textureLocations);
    }
}

/**
 * Compute the local scale vector to use for a given node. This is an imperfect hack to cope with
 * the FBX node transform's eInheritRrs inheritance type, in which ancestral scale is ignored
 */
static FbxVector4 computeLocalScale(FbxNode *pNode, FbxTime pTime = FBXSDK_TIME_INFINITE)
{
    const FbxVector4 lScale = pNode->EvaluateLocalTransform(pTime).GetS();

    if (pNode->GetParent() == nullptr ||
        pNode->GetTransform().GetInheritType() != FbxTransform::eInheritRrs) {
        return lScale;
    }
    // This is a very partial fix that is only correct for models that use identity scale in their rig's joints.
    // We could write better support that compares local scale to parent's global scale and apply the ratio to
    // our local translation. We'll always want to return scale 1, though -- that's the only way to encode the
    // missing 'S' (parent scale) in the transform chain.
    return FbxVector4(1, 1, 1, 1);
}

static void ReadNodeHierarchy(
    RawModel &raw, FbxScene *pScene, FbxNode *pNode,
    const std::string &parentName, const std::string &path)
{
    const char *nodeName = pNode->GetName();
    const int  nodeIndex = raw.AddNode(nodeName, parentName.c_str());
    RawNode    &node     = raw.GetNode(nodeIndex);

    FbxTransform::EInheritType lInheritType;
    pNode->GetTransformationInheritType(lInheritType);

    std::string newPath = path + "/" + nodeName;
    if (verboseOutput) {
        fmt::printf("node %d: %s\n", nodeIndex, newPath.c_str());
    }

    static int warnRrSsCount = 0;
    static int warnRrsCount  = 0;
    if (lInheritType == FbxTransform::eInheritRrSs && !parentName.empty()) {
        if (++warnRrSsCount == 1) {
            fmt::printf("Warning: node %s uses unsupported transform inheritance type 'eInheritRrSs'.\n", newPath);
            fmt::printf("         (Further warnings of this type squelched.)\n");
        }

    } else if (lInheritType == FbxTransform::eInheritRrs) {
        if (++warnRrsCount == 1) {
            fmt::printf(
                "Warning: node %s uses unsupported transform inheritance type 'eInheritRrs'\n"
                    "     This tool will attempt to partially compensate, but glTF cannot truly express this mode.\n"
                    "     If this was a Maya export, consider turning off 'Segment Scale Compensate' on all joints.\n"
                    "     (Further warnings of this type squelched.)\n",
                newPath);
        }
    }

    // Set the initial node transform.
    const FbxAMatrix    localTransform   = pNode->EvaluateLocalTransform();
    const FbxVector4    localTranslation = localTransform.GetT();
    const FbxQuaternion localRotation    = localTransform.GetQ();
    const FbxVector4    localScaling     = computeLocalScale(pNode);

    node.translation = toVec3f(localTranslation);
    node.rotation    = toQuatf(localRotation);
    node.scale       = toVec3f(localScaling);

    if (parentName.size() > 0) {
        RawNode &parentNode = raw.GetNode(raw.GetNodeByName(parentName.c_str()));
        // Add unique child name to the parent node.
        if (std::find(parentNode.childNames.begin(), parentNode.childNames.end(), nodeName) == parentNode.childNames.end()) {
            parentNode.childNames.push_back(nodeName);
        }
    } else {
        // If there is no parent then this is the root node.
        raw.SetRootNode(nodeName);
    }

    for (int child = 0; child < pNode->GetChildCount(); child++) {
        ReadNodeHierarchy(raw, pScene, pNode->GetChild(child), nodeName, newPath);
    }
}

static void ReadAnimations(RawModel &raw, FbxScene *pScene)
{
    FbxTime::EMode eMode = FbxTime::eFrames24;
    const double epsilon = 1e-5f;

    const int animationCount = pScene->GetSrcObjectCount<FbxAnimStack>();
    for (size_t animIx = 0; animIx < animationCount; animIx++) {
        FbxAnimStack *pAnimStack = pScene->GetSrcObject<FbxAnimStack>(animIx);
        FbxString animStackName = pAnimStack->GetName();

        pScene->SetCurrentAnimationStack(pAnimStack);

        FbxTakeInfo *takeInfo = pScene->GetTakeInfo(animStackName);
        if (takeInfo == nullptr) {
            fmt::printf("Warning:: animation '%s' has no Take information. Skipping.\n", animStackName);
            // not all animstacks have a take
            continue;
        }
        if (verboseOutput) {
            fmt::printf("animation %zu: %s (%d%%)", animIx, (const char *) animStackName, 0);
        }

        FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
        FbxTime end   = takeInfo->mLocalTimeSpan.GetStop();

        RawAnimation animation;
        animation.name = animStackName;

        FbxLongLong firstFrameIndex = start.GetFrameCount(eMode);
        FbxLongLong lastFrameIndex  = end.GetFrameCount(eMode);
        for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
            FbxTime pTime;
            // first frame is always at t = 0.0
            pTime.SetFrame(frameIndex - firstFrameIndex, eMode);
            animation.times.emplace_back((float) pTime.GetSecondDouble());
        }

        size_t totalSizeInBytes = 0;

        const int nodeCount = pScene->GetNodeCount();
        for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
            FbxNode *pNode = pScene->GetNode(nodeIndex);
            const FbxAMatrix    baseTransform   = pNode->EvaluateLocalTransform();
            const FbxVector4    baseTranslation = baseTransform.GetT();
            const FbxQuaternion baseRotation    = baseTransform.GetQ();
            const FbxVector4    baseScaling     = computeLocalScale(pNode);
            bool hasTranslation = false;
            bool hasRotation    = false;
            bool hasScale       = false;
            bool hasMorphs      = false;

            RawChannel channel;
            channel.nodeIndex = raw.GetNodeByName(pNode->GetName());

            for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
                FbxTime pTime;
                pTime.SetFrame(frameIndex, eMode);

                const FbxAMatrix    localTransform   = pNode->EvaluateLocalTransform(pTime);
                const FbxVector4    localTranslation = localTransform.GetT();
                const FbxQuaternion localRotation    = localTransform.GetQ();
                const FbxVector4    localScale       = computeLocalScale(pNode, pTime);

                hasTranslation |= (
                    fabs(localTranslation[0] - baseTranslation[0]) > epsilon ||
                    fabs(localTranslation[1] - baseTranslation[1]) > epsilon ||
                    fabs(localTranslation[2] - baseTranslation[2]) > epsilon);
                hasRotation |= (
                    fabs(localRotation[0] - baseRotation[0]) > epsilon ||
                    fabs(localRotation[1] - baseRotation[1]) > epsilon ||
                    fabs(localRotation[2] - baseRotation[2]) > epsilon ||
                    fabs(localRotation[3] - baseRotation[3]) > epsilon);
                hasScale |= (
                    fabs(localScale[0] - baseScaling[0]) > epsilon ||
                    fabs(localScale[1] - baseScaling[1]) > epsilon ||
                    fabs(localScale[2] - baseScaling[2]) > epsilon);

                channel.translations.push_back(toVec3f(localTranslation));
                channel.rotations.push_back(toQuatf(localRotation));
                channel.scales.push_back(toVec3f(localScale));
            }

            std::vector<FbxAnimCurve *> shapeAnimCurves;
            FbxNodeAttribute *nodeAttr = pNode->GetNodeAttribute();
            if (nodeAttr != nullptr && nodeAttr->GetAttributeType() == FbxNodeAttribute::EType::eMesh) {
                // it's inelegant to recreate this same access class multiple times, but it's also dirt cheap...
                FbxBlendShapesAccess blendShapes(static_cast<FbxMesh *>(nodeAttr));

                for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
                    FbxTime pTime;
                    pTime.SetFrame(frameIndex, eMode);

                    for (size_t channelIx = 0; channelIx < blendShapes.GetChannelCount(); channelIx++) {
                        FbxAnimCurve *curve = blendShapes.GetAnimation(channelIx, animIx);
                        float influence = (curve != nullptr) ? curve->Evaluate(pTime) : 0; // 0-100

                        int targetCount = static_cast<int>(blendShapes.GetTargetShapeCount(channelIx));

                        // the target shape 'fullWeight' values are a strictly ascending list of floats (between
                        // 0 and 100), forming a sequence of intervals -- this convenience function figures out if
                        // 'p' lays between some certain target fullWeights, and if so where (from 0 to 1).
                        auto findInInterval = [&](const double p, const int n) {
                            if (n >= targetCount) {
                                // p is certainly completely left of this interval
                                return NAN;
                            }
                            double leftWeight = 0;
                            if (n >= 0) {
                                leftWeight = blendShapes.GetTargetShape(channelIx, n).fullWeight;
                                if (p < leftWeight) {
                                    return NAN;
                                }
                                // the first interval implicitly includes all lesser influence values
                            }
                            double rightWeight = blendShapes.GetTargetShape(channelIx, n+1).fullWeight;
                            if (p > rightWeight && n+1 < targetCount-1) {
                                return NAN;
                                // the last interval implicitly includes all greater influence values
                            }
                            // transform p linearly such that [leftWeight, rightWeight] => [0, 1]
                            return static_cast<float>((p - leftWeight) / (rightWeight - leftWeight));
                        };

                        for (int targetIx = 0; targetIx < targetCount; targetIx++) {
                            if (curve) {
                                float result = findInInterval(influence, targetIx-1);
                                if (!isnan(result)) {
                                    // we're transitioning into targetIx
                                    channel.weights.push_back(result);
                                    hasMorphs = true;
                                    continue;
                                }
                                if (targetIx != targetCount-1) {
                                    result = findInInterval(influence, targetIx);
                                    if (!isnan(result)) {
                                        // we're transitioning AWAY from targetIx
                                        channel.weights.push_back(1.0f - result);
                                        hasMorphs = true;
                                        continue;
                                    }
                                }
                            }

                            // this is here because we have to fill in a weight for every channelIx/targetIx permutation,
                            // regardless of whether or not they participate in this animation.
                            channel.weights.push_back(0.0f);
                        }
                    }
                }
            }

            if (hasTranslation || hasRotation || hasScale || hasMorphs) {
                if (!hasTranslation) {
                    channel.translations.clear();
                }
                if (!hasRotation) {
                    channel.rotations.clear();
                }
                if (!hasScale) {
                    channel.scales.clear();
                }
                if (!hasMorphs) {
                    channel.weights.clear();
                }

                animation.channels.emplace_back(channel);

                totalSizeInBytes += channel.translations.size() * sizeof(channel.translations[0]) +
                                    channel.rotations.size() * sizeof(channel.rotations[0]) +
                                    channel.scales.size() * sizeof(channel.scales[0]) +
                                    channel.weights.size() * sizeof(channel.weights[0]);
            }

            if (verboseOutput) {
                fmt::printf("\ranimation %d: %s (%d%%)", animIx, (const char *) animStackName, nodeIndex * 100 / nodeCount);
            }
        }

        raw.AddAnimation(animation);

        if (verboseOutput) {
            fmt::printf(
                "\ranimation %d: %s (%d channels, %3.1f MB)\n", animIx, (const char *) animStackName,
                (int) animation.channels.size(), (float) totalSizeInBytes * 1e-6f);
        }
    }
}

static std::string GetInferredFileName(const std::string &fbxFileName, const std::string &directory, const std::vector<std::string> &directoryFileList)
{
    // Get the file name with file extension.
    const std::string fileName = Gltf::StringUtils::GetFileNameString(Gltf::StringUtils::GetCleanPathString(fbxFileName));

    // Try to find a match with extension.
    for (const auto &file : directoryFileList) {
        if (Gltf::StringUtils::CompareNoCase(fileName, file) == 0) {
            return std::string(directory) + file;
        }
    }

    // Get the file name without file extension.
    const std::string fileBase = Gltf::StringUtils::GetFileBaseString(fileName);

    // Try to find a match without file extension.
    for (const auto &file : directoryFileList) {
        // If the two extension-less base names match.
        if (Gltf::StringUtils::CompareNoCase(fileBase, Gltf::StringUtils::GetFileBaseString(file)) == 0) {
            // Return the name with extension of the file in the directory.
            return std::string(directory) + file;
        }
    }

    return "";
}

/*
    The texture file names inside of the FBX often contain some long author-specific
    path with the wrong extensions. For instance, all of the art assets may be PSD
    files in the FBX metadata, but in practice they are delivered as TGA or PNG files.

    This function takes a texture file name stored in the FBX, which may be an absolute
    path on the author's computer such as "C:\MyProject\TextureName.psd", and matches
    it to a list of existing texture files in the same directory as the FBX file.
*/
static void
FindFbxTextures(
    FbxScene *pScene, const char *fbxFileName, const char *extensions, std::map<const FbxTexture *, FbxString> &textureLocations)
{
    // Get the folder the FBX file is in.
    const std::string folder = Gltf::StringUtils::GetFolderString(fbxFileName);

    // Check if there is a filename.fbm folder to which embedded textures were extracted.
    const std::string fbmFolderName = folder + Gltf::StringUtils::GetFileBaseString(fbxFileName) + ".fbm/";

    // Search either in the folder with embedded textures or in the same folder as the FBX file.
    const std::string searchFolder = FileUtils::FolderExists(fbmFolderName) ? fbmFolderName : folder;

    // Get a list with all the texture files from either the folder with embedded textures or the same folder as the FBX file.
    std::vector<std::string> fileList = FileUtils::ListFolderFiles(searchFolder.c_str(), extensions);

    // Try to match the FBX texture names with the actual files on disk.
    for (int i = 0; i < pScene->GetTextureCount(); i++) {
        const FbxFileTexture *pFileTexture = FbxCast<FbxFileTexture>(pScene->GetTexture(i));
        if (pFileTexture == nullptr) {
            continue;
        }
        const std::string inferredName = GetInferredFileName(pFileTexture->GetFileName(), searchFolder, fileList);
        if (inferredName.empty()) {
            fmt::printf("Warning: could not find a local image file for texture: %s.\n"
            "Original filename: %s\n", pFileTexture->GetName(), pFileTexture->GetFileName());
        }
        // always extend the mapping, even for files we didn't find
        textureLocations.emplace(pFileTexture, inferredName.c_str());
    }
}

bool LoadFBXFile(RawModel &raw, const char *fbxFileName, const char *textureExtensions)
{
    FbxManager    *pManager    = FbxManager::Create();
    FbxIOSettings *pIoSettings = FbxIOSettings::Create(pManager, IOSROOT);
    pManager->SetIOSettings(pIoSettings);

    FbxImporter *pImporter = FbxImporter::Create(pManager, "");

    if (!pImporter->Initialize(fbxFileName, -1, pManager->GetIOSettings())) {
        if (verboseOutput) {
            fmt::printf("%s\n", pImporter->GetStatus().GetErrorString());
        }
        pImporter->Destroy();
        pManager->Destroy();
        return false;
    }

    FbxScene *pScene = FbxScene::Create(pManager, "fbxScene");
    pImporter->Import(pScene);
    pImporter->Destroy();

    if (pScene == nullptr) {
        pImporter->Destroy();
        pManager->Destroy();
        return false;
    }

    std::map<const FbxTexture *, FbxString> textureLocations;
    FindFbxTextures(pScene, fbxFileName, textureExtensions, textureLocations);

    // Use Y up for glTF
    FbxAxisSystem::MayaYUp.ConvertScene(pScene);

    // Use meters as the default unit for glTF
    FbxSystemUnit sceneSystemUnit = pScene->GetGlobalSettings().GetSystemUnit();
    if (sceneSystemUnit != FbxSystemUnit::m) {
        FbxSystemUnit::m.ConvertScene(pScene);
    }

    ReadNodeHierarchy(raw, pScene, pScene->GetRootNode(), "", "");
    ReadNodeAttributes(raw, pScene, pScene->GetRootNode(), textureLocations);
    ReadAnimations(raw, pScene);

    pScene->Destroy();
    pManager->Destroy();

    return true;
}
