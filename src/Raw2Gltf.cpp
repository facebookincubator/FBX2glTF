/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <cstdint>
#include <cassert>
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "FBX2glTF.h"
#include "utils/String_Utils.h"
#include "utils/Image_Utils.h"
#include <utils/File_Utils.h>
#include "RawModel.h"
#include "Raw2Gltf.h"

#include "glTF/AccessorData.h"
#include "glTF/AnimationData.h"
#include "glTF/BufferData.h"
#include "glTF/BufferViewData.h"
#include "glTF/CameraData.h"
#include "glTF/ImageData.h"
#include "glTF/MaterialData.h"
#include "glTF/MeshData.h"
#include "glTF/NodeData.h"
#include "glTF/PrimitiveData.h"
#include "glTF/SamplerData.h"
#include "glTF/SceneData.h"
#include "glTF/SkinData.h"
#include "glTF/TextureData.h"

typedef unsigned short TriangleIndex;

extern bool verboseOutput;

const static std::string defaultSceneName = "Root Scene";

/**
 * glTF 2.0 is based on the idea that data structs within a file are referenced by index; an accessor will
 * point to the n:th buffer view, and so on. The Holder class takes a freshly instantiated class, and then
 * creates, stored, and returns a shared_ptr<T> for it.
 *
 * The idea is that every glTF resource in the file will live as long as the Holder does, and the Holders
 * are all kept in the GLTFData struct. Clients may certainly cnhoose to perpetuate the full shared_ptr<T>
 * reference counting type, but generally speaking we pass around simple T& and T* types because the GLTFData
 * struct will, by design, outlive all other activity that takes place during in a single conversion run.
 */
template<typename T>
struct Holder
{
    std::vector<std::shared_ptr<T>> ptrs;
    std::shared_ptr<T> hold(T *ptr)
    {
        ptr->ix = ptrs.size();
        ptrs.emplace_back(ptr);
        return ptrs.back();
    }
};

struct GLTFData
{
    explicit GLTFData(bool _isGlb)
        : binary(new std::vector<uint8_t>),
          isGlb(_isGlb)
    {
    }

    std::shared_ptr<BufferViewData> GetAlignedBufferView(BufferData &buffer, const BufferViewData::GL_ArrayType target)
    {
        unsigned long bufferSize = this->binary->size();
        if ((bufferSize % 4) > 0) {
            bufferSize += (4 - (bufferSize % 4));
            this->binary->resize(bufferSize);
        }
        return this->bufferViews.hold(new BufferViewData(buffer, bufferSize, target));
    }

    // add a bufferview on the fly and copy data into it
    std::shared_ptr<BufferViewData> AddRawBufferView(BufferData &buffer, const char *source, uint32_t bytes)
    {
        auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_NONE);
        bufferView->byteLength = bytes;

        // make space for the new bytes (possibly moving the underlying data)
        unsigned long bufferSize = this->binary->size();
        this->binary->resize(bufferSize + bytes);

        // and copy them into place
        memcpy(&(*this->binary)[bufferSize], source, bytes);
        return bufferView;
    }

    std::shared_ptr<BufferViewData> AddBufferViewForFile(BufferData &buffer, const std::string &filename)
    {
        // see if we've already created a BufferViewData for this precise file
        auto iter = filenameToBufferView.find(filename);
        if (iter != filenameToBufferView.end()) {
            return iter->second;
        }

        std::shared_ptr<BufferViewData> result;
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (file) {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<char> fileBuffer(size);
            if (file.read(fileBuffer.data(), size)) {
                result = AddRawBufferView(buffer, fileBuffer.data(), size);
            } else {
                fmt::printf("Warning: Couldn't read %lu bytes from %s, skipping file.\n", size, filename);
            }
        } else {
            fmt::printf("Warning: Couldn't open file %s, skipping file.\n", filename);
        }
        // note that we persist here not only success, but also failure, as nullptr
        filenameToBufferView[filename] = result;
        return result;
    }


    template<class T>
    std::shared_ptr<AccessorData> AddAccessorWithView(
        BufferViewData &bufferView, const GLType &type, const std::vector<T> &source)
    {
        auto accessor = accessors.hold(new AccessorData(bufferView, type));
        accessor->appendAsBinaryArray(source, *binary);
        bufferView.byteLength = accessor->byteLength();
        return accessor;
    }

    template<class T>
    std::shared_ptr<AccessorData> AddAccessorAndView(
        BufferData &buffer, const GLType &type, const std::vector<T> &source)
    {
        auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_NONE);
        return AddAccessorWithView(*bufferView, type, source);
    }

    template<class T>
    std::shared_ptr<AccessorData> AddAttributeToPrimitive(
        BufferData &buffer, const RawModel &surfaceModel, PrimitiveData &primitive,
        const AttributeDefinition<T> &attrDef)
    {
        // copy attribute data into vector
        std::vector<T> attribArr;
        surfaceModel.GetAttributeArray<T>(attribArr, attrDef.rawAttributeIx);

        std::shared_ptr<AccessorData> accessor;
        if (attrDef.dracoComponentType != draco::DT_INVALID && primitive.dracoMesh != nullptr) {
            primitive.AddDracoAttrib(attrDef, attribArr);

            accessor = accessors.hold(new AccessorData(attrDef.glType));
            accessor->count = attribArr.size();
        } else {
            auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_BUFFER);
            accessor = AddAccessorWithView(*bufferView, attrDef.glType, attribArr);
        }
        primitive.AddAttrib(attrDef.gltfName, *accessor);
        return accessor;
    };

    template<class T>
    void serializeHolder(json &glTFJson, std::string key, const Holder<T> holder)
    {
        if (!holder.ptrs.empty()) {
            std::vector<json> bits;
            for (const auto &ptr : holder.ptrs) {
                bits.push_back(ptr->serialize());
            }
            glTFJson[key] = bits;
        }
    }

    void serializeHolders(json &glTFJson)
    {
      serializeHolder(glTFJson, "buffers", buffers);
      serializeHolder(glTFJson, "bufferViews", bufferViews);
      serializeHolder(glTFJson, "scenes", scenes);
      serializeHolder(glTFJson, "accessors", accessors);
      serializeHolder(glTFJson, "images", images);
      serializeHolder(glTFJson, "samplers", samplers);
      serializeHolder(glTFJson, "textures", textures);
      serializeHolder(glTFJson, "materials", materials);
      serializeHolder(glTFJson, "meshes", meshes);
      serializeHolder(glTFJson, "skins", skins);
      serializeHolder(glTFJson, "animations", animations);
      serializeHolder(glTFJson, "cameras", cameras);
      serializeHolder(glTFJson, "nodes", nodes);
    }

    const bool isGlb;

    // cache BufferViewData instances that've already been created from a given filename
    std::map<std::string, std::shared_ptr<BufferViewData>> filenameToBufferView;

    std::shared_ptr<std::vector<uint8_t> > binary;


    Holder<BufferData>     buffers;
    Holder<BufferViewData> bufferViews;
    Holder<AccessorData>   accessors;
    Holder<ImageData>      images;
    Holder<SamplerData>    samplers;
    Holder<TextureData>    textures;
    Holder<MaterialData>   materials;
    Holder<MeshData>       meshes;
    Holder<SkinData>       skins;
    Holder<AnimationData>  animations;
    Holder<CameraData>     cameras;
    Holder<NodeData>       nodes;
    Holder<SceneData>      scenes;
};

template <class T>
static void WriteToVectorContext(void *context, void *data, int size) {
    std::vector<T> *vec = static_cast<std::vector<T> *>(context);
    for (int ii = 0; ii < size; ii ++) {
        vec->emplace_back(((T *) data)[ii]);
    }
}

/**
 * This method sanity-checks existance and then returns a *reference* to the *Data instance
 * registered under that name. This is safe in the context of this tool, where all such data
 * classes are guaranteed to stick around for the duration of the process.
 */
template<typename T>
T &require(std::map<std::string, std::shared_ptr<T>> map, const std::string &key)
{
    auto iter = map.find(key);
    assert(iter != map.end());
    T &result = *iter->second;
    return result;
}

static const std::vector<TriangleIndex> getIndexArray(const RawModel &raw)
{
    std::vector<TriangleIndex> result;

    for (int i = 0; i < raw.GetTriangleCount(); i++) {
        result.push_back((TriangleIndex) raw.GetTriangle(i).verts[0]);
        result.push_back((TriangleIndex) raw.GetTriangle(i).verts[1]);
        result.push_back((TriangleIndex) raw.GetTriangle(i).verts[2]);
    }
    return result;
}

// TODO: replace with a proper MaterialHasher class
static const std::string materialHash(const RawMaterial &m) {
    return m.name + "_" + std::to_string(m.type);
}

ModelData *Raw2Gltf(
    std::ofstream &gltfOutStream,
    const std::string &outputFolder,
    const RawModel &raw,
    const GltfOptions &options
)
{
    if (verboseOutput) {
        fmt::printf("Building render model...\n");
        for (int i = 0; i < raw.GetMaterialCount(); i++) {
            fmt::printf(
                "Material %d: %s [shading: %s]\n", i, raw.GetMaterial(i).name.c_str(),
                raw.GetMaterial(i).shadingModel.c_str());
        }
        if (raw.GetVertexCount() > 2 * raw.GetTriangleCount()) {
            fmt::printf(
                "Warning: High vertex count. Make sure there are no unnecessary vertex attributes. (see -keepAttribute cmd-line option)");
        }
    }

    std::vector<RawModel> materialModels;
    raw.CreateMaterialModels(materialModels, (1 << (sizeof(TriangleIndex) * 8)), options.keepAttribs, true);

    if (verboseOutput) {
        fmt::printf("%7d vertices\n", raw.GetVertexCount());
        fmt::printf("%7d triangles\n", raw.GetTriangleCount());
        fmt::printf("%7d textures\n", raw.GetTextureCount());
        fmt::printf("%7d nodes\n", raw.GetNodeCount());
        fmt::printf("%7d surfaces\n", (int) materialModels.size());
        fmt::printf("%7d animations\n", raw.GetAnimationCount());
    }

    std::unique_ptr<GLTFData> gltf(new GLTFData(options.outputBinary));

    std::map<std::string, std::shared_ptr<NodeData>>     nodesByName;
    std::map<std::string, std::shared_ptr<MaterialData>> materialsByName;
    std::map<std::string, std::shared_ptr<MeshData>>     meshByNodeName;
    std::map<int, std::shared_ptr<TextureData>>          textureByRawIndex;

    // for now, we only have one buffer; data->binary points to the same vector as that BufferData does.
    BufferData &buffer = *gltf->buffers.hold(
        options.outputBinary ?
        new BufferData(gltf->binary) :
        new BufferData(extBufferFilename, gltf->binary, options.embedResources));
    {
        //
        // nodes
        //

        for (int i = 0; i < raw.GetNodeCount(); i++) {
            // assumption: RawNode index == NodeData index
            const RawNode &node = raw.GetNode(i);

            auto nodeData = gltf->nodes.hold(
                new NodeData(node.name, node.translation, node.rotation, node.scale, node.isJoint));

            for (const auto &childName : node.childNames) {
                int childIx = raw.GetNodeByName(childName.c_str());
                assert(childIx >= 0);
                nodeData->AddChildNode(childIx);
            }
            assert(nodesByName.find(nodeData->name) == nodesByName.end());
            nodesByName.insert(std::make_pair(nodeData->name, nodeData));
        }

        //
        // animations
        //

        for (int i = 0; i < raw.GetAnimationCount(); i++) {
            const RawAnimation &animation = raw.GetAnimation(i);

            if (animation.channels.size() == 0) {
                fmt::printf("Warning: animation '%s' has zero channels. Skipping.\n", animation.name.c_str());
                continue;
            }

            auto accessor = gltf->AddAccessorAndView(buffer, GLT_FLOAT, animation.times);
            accessor->min = { *std::min_element(std::begin(animation.times), std::end(animation.times)) };
            accessor->max = { *std::max_element(std::begin(animation.times), std::end(animation.times)) };

            AnimationData &aDat = *gltf->animations.hold(new AnimationData(animation.name, *accessor));
            if (verboseOutput) {
                fmt::printf("Animation '%s' has %lu channels:\n", animation.name.c_str(), animation.channels.size());
            }

            for (size_t channelIx = 0; channelIx < animation.channels.size(); channelIx++) {
                const RawChannel &channel = animation.channels[channelIx];
                const RawNode    &node    = raw.GetNode(channel.nodeIndex);

                if (verboseOutput) {
                    fmt::printf(
                        "  Channel %lu (%s) has translations/rotations/scales/weights: [%lu, %lu, %lu, %lu]\n",
                        channelIx, node.name.c_str(), channel.translations.size(), channel.rotations.size(),
                        channel.scales.size(), channel.weights.size());
                }

                NodeData &nDat = require(nodesByName, node.name);
                if (!channel.translations.empty()) {
                    aDat.AddNodeChannel(nDat, *gltf->AddAccessorAndView(buffer, GLT_VEC3F, channel.translations), "translation");
                }
                if (!channel.rotations.empty()) {
                    aDat.AddNodeChannel(nDat, *gltf->AddAccessorAndView(buffer, GLT_QUATF, channel.rotations), "rotation");
                }
                if (!channel.scales.empty()) {
                    aDat.AddNodeChannel(nDat, *gltf->AddAccessorAndView(buffer, GLT_VEC3F, channel.scales), "scale");
                }
                if (!channel.weights.empty()) {
                    aDat.AddNodeChannel(nDat, *gltf->AddAccessorAndView(buffer, {CT_FLOAT, 1, "SCALAR"}, channel.weights), "weights");
                }
            }
        }

        //
        // samplers
        //

        SamplerData &defaultSampler = *gltf->samplers.hold(new SamplerData());

        //
        // textures
        //
        typedef std::array<uint8_t, 4> pixel;
        typedef std::function<pixel(const pixel &, const pixel &)> pixel_merger;

        /**
         * Create a new derived TextureData for the two given RawTexture indexes, or return a previously created one.
         * Each pixel in the derived texture will be determined from its equivalent in each source pixels, as decided
         * by the provided `combine` function.
         */
        auto getDerivedTexture = [&](int rawTexIx1, int rawTexIx2, const pixel_merger &combine) -> std::shared_ptr<TextureData> {
            // TODO: index on first texture? what if it's -1?
            auto iter = textureByRawIndex.find(rawTexIx1);
            if (iter != textureByRawIndex.end()) {
                return iter->second;
            }

            const RawTexture &rawTex1 = raw.GetTexture(rawTexIx1);
            const RawTexture &rawTex2 = raw.GetTexture(rawTexIx2);

            auto channelStr = [&](int channels) -> std::string {
                switch(channels) {
                    case 1: return "G";
                    case 2: return "GA";
                    case 3: return "RGB";
                    case 4: return "RGBA";
                    default:
                        return fmt::format("?%d?", channels);
                }
            };

            int w1, h1, c1;
            uint8_t *s1 = stbi_load(rawTex1.fileLocation.c_str(), &w1, &h1, &c1, 4);
            if (!s1) {
                fmt::printf("Warning: merge texture 1 (%s) could not be loaded.\n", rawTex1.fileName);
                return nullptr;
            }

            int w2, h2, c2;
            uint8_t *s2 = stbi_load(rawTex2.fileLocation.c_str(), &w2, &h2, &c2, 4);
            if (!s2) {
                fmt::printf("Warning: merge texture 2 (%s) could not be loaded.\n", rawTex2.fileName);
                return nullptr;
            }

            if (w1 != w2 || h1 != h2) {
                fmt::printf("Warning: textures %s and %s have different dimensions and can't be combined\n",
                    rawTex1.fileName, rawTex2.fileName);
                return nullptr;
            }
            // TODO: which channel combinations make sense in input files?

            std::vector<uint8_t > mergedPixels(4 * w1 * h1);
            for (int ii = 0; ii < mergedPixels.size(); ii += 4) {
                pixel merged = combine(
                    pixel { s1[ii+0], s1[ii+1], s1[ii+2], s1[ii+3] },
                    pixel { s2[ii+0], s2[ii+1], s2[ii+2], s2[ii+3] });
                for (int jj = 0; jj < 4; jj ++) {
                    mergedPixels[ii + jj] = merged[jj];
                }
            }

            bool png = false;
            std::vector<char> imgBuffer;
            int res;
            if (png) {
                res = stbi_write_png_to_func(WriteToVectorContext<char>, &imgBuffer, w1, h1, 4, mergedPixels.data(), w1 * 4);
            } else {
                res = stbi_write_jpg_to_func(WriteToVectorContext<char>, &imgBuffer, w1, h1, 4, mergedPixels.data(), 80);
            }
            // TODO
            assert(res != 0);

            const std::string name = "merge_" + rawTex1.name + "_" + rawTex2.name;
            const std::string fileName = "merge_" +
                Gltf::StringUtils::GetFileBaseString(Gltf::StringUtils::GetFileNameString(rawTex1.fileLocation)) + "_" +
                Gltf::StringUtils::GetFileBaseString(Gltf::StringUtils::GetFileNameString(rawTex2.fileLocation));

            ImageData *image;
            if (options.outputBinary) {
                const auto bufferView = gltf->AddRawBufferView(buffer, imgBuffer.data(), imgBuffer.size());
                image = new ImageData(name, *bufferView, png ? "image/png" : "image/jpeg");

            } else {
                const std::string imageFilename = fileName + (png ? ".png" : ".jpg");
                const std::string imagePath = outputFolder + imageFilename;
                FILE *fp = fopen(imagePath.c_str(), "wb");
                if (fp == nullptr) {
                    fmt::printf("Warning:: Couldn't write file '%s' for writing.\n", imagePath);
                    return nullptr;
                }

                if (fwrite(imgBuffer.data(), imgBuffer.size(), 1, fp) != 1) {
                    fmt::printf("Warning: Failed to write %lu bytes to file '%s'.\n", imgBuffer.size(), imagePath);
                    fclose(fp);
                    return nullptr;
                }
                fclose(fp);
                if (verboseOutput) {
                    fmt::printf("Wrote %lu bytes to texture '%s'.\n", imgBuffer.size(), imagePath);
                }

                image = new ImageData(name, imageFilename);
            }

            std::shared_ptr<TextureData> texDat = gltf->textures.hold(
                new TextureData(name, defaultSampler, *gltf->images.hold(image)));
            textureByRawIndex.insert(std::make_pair(rawTexIx1, texDat));
            return texDat;
        };

        /** Create a new TextureData for the given RawTexture index, or return a previously created one. */
        auto getSimpleTexture = [&](int rawTexIndex) {
            auto iter = textureByRawIndex.find(rawTexIndex);
            if (iter != textureByRawIndex.end()) {
                return iter->second;
            }

            const RawTexture  &rawTexture      = raw.GetTexture(rawTexIndex);
            const std::string textureName      = Gltf::StringUtils::GetFileBaseString(rawTexture.name);
            const std::string relativeFilename = Gltf::StringUtils::GetFileNameString(rawTexture.fileLocation);

            ImageData *image = nullptr;
            if (options.outputBinary) {
                auto bufferView = gltf->AddBufferViewForFile(buffer, rawTexture.fileLocation);
                if (bufferView) {
                    std::string suffix = Gltf::StringUtils::GetFileSuffixString(rawTexture.fileLocation);
                    image = new ImageData(relativeFilename, *bufferView, suffixToMimeType(suffix));
                }

            } else if (!relativeFilename.empty()) {
                image = new ImageData(relativeFilename, relativeFilename);
                std::string outputPath = outputFolder + relativeFilename;
                if (FileUtils::CopyFile(rawTexture.fileLocation, outputPath)) {
                    if (verboseOutput) {
                        fmt::printf("Copied texture '%s' to output folder: %s\n", textureName, outputPath);
                    }
                } else {
                    // no point commenting further on read/write error; CopyFile() does enough of that, and we
                    // certainly want to to add an image struct to the glTF JSON, with the correct relative path
                    // reference, even if the copy failed.
                }
            }
            if (!image) {
                // fallback is tiny transparent gif
                image = new ImageData(textureName, "data:image/gif;base64,R0lGODlhAQABAAD/ACwAAAAAAQABAAACADs=");
            }

            std::shared_ptr<TextureData> texDat = gltf->textures.hold(
                new TextureData(textureName, defaultSampler, *gltf->images.hold(image)));
            textureByRawIndex.insert(std::make_pair(rawTexIndex, texDat));
            return texDat;
        };

        //
        // materials
        //

        for (int materialIndex = 0; materialIndex < raw.GetMaterialCount(); materialIndex++) {
            const RawMaterial &material = raw.GetMaterial(materialIndex);
            const bool isTransparent =
                           material.type == RAW_MATERIAL_TYPE_TRANSPARENT ||
                           material.type == RAW_MATERIAL_TYPE_SKINNED_TRANSPARENT;

            // acquire the texture of a specific RawTextureUsage as *TextData, or nullptr if none exists
            auto simpleTex = [&](RawTextureUsage usage) -> const TextureData * {
                return (material.textures[usage] >= 0) ? getSimpleTexture(material.textures[usage]).get() : nullptr;
            };

            // acquire derived texture of two RawTextureUsage as *TextData, or nullptr if neither exists
            auto derivedTex = [&](RawTextureUsage u1, RawTextureUsage u2, const pixel_merger &combine) -> const TextureData * {
                int t1 = material.textures[u1], t2 = material.textures[u2];
                return (t1 >= 0 || t2 >= 0) ? getDerivedTexture(t1, t2, combine).get() : nullptr;
            };

            std::shared_ptr<PBRMetallicRoughness> pbrMetRough;
            if (options.usePBRMetRough) {
                // merge metallic into the blue channel and roughness into the green, of a new combinatory texture
                const TextureData *metRoughTex = derivedTex(
                    RAW_TEXTURE_USAGE_METALLIC, RAW_TEXTURE_USAGE_ROUGHNESS,
                    [&](const pixel &met, const pixel &rough) -> pixel { return { 0, rough[0], met[0], 0 }; });
                // albedo is basic
                const TextureData *albedoTex = simpleTex(RAW_TEXTURE_USAGE_ALBEDO);

                // if there's a met/rough texture, just set the factors to 1.0 multipliers, else use reasonable
                // defaults for a possible vertex-coloured or solid colour setup
                float metallic  = metRoughTex ? 1.0f : 0.3f;
                float roughness = metRoughTex ? 1.0f : 0.6f;
                pbrMetRough.reset(new PBRMetallicRoughness(albedoTex, metRoughTex, material.diffuseFactor, metallic, roughness));
            }

            std::shared_ptr<PBRSpecularGlossiness> pbrSpecGloss;
            if (options.usePBRSpecGloss) {
                pbrSpecGloss.reset(
                    new PBRSpecularGlossiness(
                        simpleTex(RAW_TEXTURE_USAGE_DIFFUSE), material.diffuseFactor,
                        simpleTex(RAW_TEXTURE_USAGE_SPECULAR), material.specularFactor, material.shininess));
            }

            std::shared_ptr<KHRCommonMats> khrComMat;
            if (options.useKHRMatCom) {
                auto type = KHRCommonMats::MaterialType::Constant;
                if (material.shadingModel == "Lambert") {
                    type = KHRCommonMats::MaterialType::Lambert;
                } else if (material.shadingModel == "Blinn") {
                    type = KHRCommonMats::MaterialType::Blinn;
                } else if (material.shadingModel == "Phong") {
                    type = KHRCommonMats::MaterialType::Phong;
                }
                khrComMat.reset(
                    new KHRCommonMats(
                        type,
                        simpleTex(RAW_TEXTURE_USAGE_SHININESS), material.shininess,
                        simpleTex(RAW_TEXTURE_USAGE_AMBIENT), material.ambientFactor,
                        simpleTex(RAW_TEXTURE_USAGE_DIFFUSE), material.diffuseFactor,
                        simpleTex(RAW_TEXTURE_USAGE_SPECULAR), material.specularFactor));
            }
            std::shared_ptr<MaterialData> mData = gltf->materials.hold(
                new MaterialData(
                    material.name, isTransparent, simpleTex(RAW_TEXTURE_USAGE_NORMAL),
                    simpleTex(RAW_TEXTURE_USAGE_EMISSIVE), material.emissiveFactor,
                    khrComMat, pbrMetRough, pbrSpecGloss));
            materialsByName[materialHash(material)] = mData;
        }

        //
        // surfaces
        //

        // in GLTF 2.0, the structural limitation is that a node can
        // only belong to a single mesh. A mesh can however contain any
        // number of primitives, which are essentially little meshes.
        //
        // so each RawSurface turns into a primitive, and we sort them
        // by root node using this map; one mesh per node.

        for (size_t surfaceIndex = 0; surfaceIndex < materialModels.size(); surfaceIndex++) {
            const RawModel &surfaceModel = materialModels[surfaceIndex];

            assert(surfaceModel.GetSurfaceCount() == 1);
            const RawSurface  &rawSurface = surfaceModel.GetSurface(0);

            const RawMaterial &rawMaterial = surfaceModel.GetMaterial(surfaceModel.GetTriangle(0).materialIndex);
            const MaterialData &mData = require(materialsByName, materialHash(rawMaterial));

            std::string nodeName  = rawSurface.nodeName;
            NodeData    &meshNode = require(nodesByName, nodeName);

            MeshData *mesh    = nullptr;
            auto     meshIter = meshByNodeName.find(nodeName);
            if (meshIter != meshByNodeName.end()) {
                mesh = meshIter->second.get();

            } else {
                std::vector<float> defaultDeforms;
                for (const auto &channel : rawSurface.blendChannels) {
                    defaultDeforms.push_back(channel.defaultDeform);
                }
                auto meshPtr = gltf->meshes.hold(new MeshData(rawSurface.name, defaultDeforms));
                meshByNodeName[nodeName] = meshPtr;
                meshNode.SetMesh(meshPtr->ix);
                mesh = meshPtr.get();
            }

            //
            // surface skin
            //
            if (!rawSurface.jointNames.empty()) {
                if (meshNode.skin == -1) {
                    // glTF uses column-major matrices
                    std::vector<Mat4f> inverseBindMatrices;
                    for (const auto &inverseBindMatrice : rawSurface.inverseBindMatrices) {
                        inverseBindMatrices.push_back(inverseBindMatrice.Transpose());
                    }

                    std::vector<uint32_t> jointIndexes;
                    for (const auto &jointName : rawSurface.jointNames) {
                        jointIndexes.push_back(require(nodesByName, jointName).ix);
                    }

                    // Write out inverseBindMatrices
                    auto accIBM = gltf->AddAccessorAndView(buffer, GLT_MAT4F, inverseBindMatrices);

                    auto skeletonRoot = require(nodesByName, rawSurface.skeletonRootName);
                    auto skin         = *gltf->skins.hold(new SkinData(jointIndexes, *accIBM, skeletonRoot));
                    meshNode.SetSkin(skin.ix);
                }
            }

            std::shared_ptr<PrimitiveData> primitive;
            if (options.useDraco) {
                int triangleCount = surfaceModel.GetTriangleCount();

                // initialize Draco mesh with vertex index information
                auto dracoMesh(std::make_shared<draco::Mesh>());
                dracoMesh->SetNumFaces(static_cast<size_t>(triangleCount));

                for (uint32_t ii = 0; ii < triangleCount; ii++) {
                    draco::Mesh::Face face;
                    face[0] = surfaceModel.GetTriangle(ii).verts[0];
                    face[1] = surfaceModel.GetTriangle(ii).verts[1];
                    face[2] = surfaceModel.GetTriangle(ii).verts[2];
                    dracoMesh->SetFace(draco::FaceIndex(ii), face);
                }

                AccessorData &indexes = *gltf->accessors.hold(new AccessorData(GLT_USHORT));
                indexes.count = 3 * triangleCount;
                primitive.reset(new PrimitiveData(indexes, mData, dracoMesh));
            } else {
                const AccessorData &indexes = *gltf->AddAccessorWithView(
                    *gltf->GetAlignedBufferView(buffer, BufferViewData::GL_ELEMENT_ARRAY_BUFFER),
                    GLT_USHORT, getIndexArray(surfaceModel));
                primitive.reset(new PrimitiveData(indexes, mData));
            };

            //
            // surface vertices
            //
            {
                if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_POSITION) != 0) {
                    const AttributeDefinition<Vec3f> ATTR_POSITION("POSITION", &RawVertex::position,
                        GLT_VEC3F, draco::GeometryAttribute::POSITION, draco::DT_FLOAT32);
                    auto accessor = gltf->AddAttributeToPrimitive<Vec3f>(
                        buffer, surfaceModel, *primitive, ATTR_POSITION);

                    accessor->min = toStdVec(rawSurface.bounds.min);
                    accessor->max = toStdVec(rawSurface.bounds.max);
                }
                if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_NORMAL) != 0) {
                    const AttributeDefinition<Vec3f> ATTR_NORMAL("NORMAL", &RawVertex::normal,
                        GLT_VEC3F, draco::GeometryAttribute::NORMAL, draco::DT_FLOAT32);
                    gltf->AddAttributeToPrimitive<Vec3f>(buffer, surfaceModel, *primitive, ATTR_NORMAL);
                }
                if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_TANGENT) != 0) {
                    const AttributeDefinition<Vec4f> ATTR_TANGENT("TANGENT", &RawVertex::tangent, GLT_VEC4F);
                    gltf->AddAttributeToPrimitive<Vec4f>(buffer, surfaceModel, *primitive, ATTR_TANGENT);
                }
                if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_COLOR) != 0) {
                    const AttributeDefinition<Vec4f> ATTR_COLOR("COLOR_0", &RawVertex::color, GLT_VEC4F,
                        draco::GeometryAttribute::COLOR, draco::DT_FLOAT32);
                    gltf->AddAttributeToPrimitive<Vec4f>(buffer, surfaceModel, *primitive, ATTR_COLOR);
                }
                if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
                    const AttributeDefinition<Vec2f> ATTR_TEXCOORD_0("TEXCOORD_0", &RawVertex::uv0,
                        GLT_VEC2F, draco::GeometryAttribute::TEX_COORD, draco::DT_FLOAT32);
                    gltf->AddAttributeToPrimitive<Vec2f>(buffer, surfaceModel, *primitive, ATTR_TEXCOORD_0);
                }
                if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_UV1) != 0) {
                    const AttributeDefinition<Vec2f> ATTR_TEXCOORD_1("TEXCOORD_1", &RawVertex::uv1,
                        GLT_VEC2F, draco::GeometryAttribute::TEX_COORD, draco::DT_FLOAT32);
                    gltf->AddAttributeToPrimitive<Vec2f>(buffer, surfaceModel, *primitive, ATTR_TEXCOORD_1);
                }
                if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_JOINT_INDICES) != 0) {
                    const AttributeDefinition<Vec4i> ATTR_JOINTS("JOINTS_0", &RawVertex::jointIndices,
                        GLT_VEC4I, draco::GeometryAttribute::GENERIC, draco::DT_UINT16);
                    gltf->AddAttributeToPrimitive<Vec4i>(buffer, surfaceModel, *primitive, ATTR_JOINTS);
                }
                if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS) != 0) {
                    const AttributeDefinition<Vec4f> ATTR_WEIGHTS("WEIGHTS_0", &RawVertex::jointWeights,
                        GLT_VEC4F, draco::GeometryAttribute::GENERIC, draco::DT_FLOAT32);
                    gltf->AddAttributeToPrimitive<Vec4f>(buffer, surfaceModel, *primitive, ATTR_WEIGHTS);
                }

                // each channel present in the mesh always ends up a target in the primitive
                for (int channelIx = 0; channelIx < rawSurface.blendChannels.size(); channelIx ++) {
                    const auto &channel = rawSurface.blendChannels[channelIx];

                    // track the bounds of each shape channel
                    Bounds<float, 3> shapeBounds;

                    std::vector<Vec3f> positions, normals;
                    std::vector<Vec4f> tangents;
                    for (int jj = 0; jj < surfaceModel.GetVertexCount(); jj ++) {
                        auto blendVertex = surfaceModel.GetVertex(jj).blends[channelIx];
                        shapeBounds.AddPoint(blendVertex.position);
                        positions.push_back(blendVertex.position);
                        if (options.useBlendShapeTangents && channel.hasNormals) {
                            normals.push_back(blendVertex.normal);
                        }
                        if (options.useBlendShapeTangents && channel.hasTangents) {
                            tangents.push_back(blendVertex.tangent);
                        }
                    }
                    std::shared_ptr<AccessorData> pAcc = gltf->AddAccessorWithView(
                        *gltf->GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_BUFFER),
                        GLT_VEC3F, positions);
                    pAcc->min = toStdVec(shapeBounds.min);
                    pAcc->max = toStdVec(shapeBounds.max);

                    std::shared_ptr<AccessorData> nAcc;
                    if (!normals.empty()) {
                        nAcc = gltf->AddAccessorWithView(
                            *gltf->GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_BUFFER),
                            GLT_VEC3F, normals);
                    }

                    std::shared_ptr<AccessorData> tAcc;
                    if (!tangents.empty()) {
                        nAcc = gltf->AddAccessorWithView(
                            *gltf->GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_BUFFER),
                            GLT_VEC4F, tangents);
                    }

                    primitive->AddTarget(pAcc.get(), nAcc.get(), tAcc.get());
                }
            }
            if (options.useDraco) {
                // Set up the encoder.
                draco::Encoder encoder;

                // TODO: generalize / allow configuration
                encoder.SetSpeedOptions(5, 5);
                encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14);
                encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 10);
                encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 10);
                encoder.SetAttributeQuantization(draco::GeometryAttribute::COLOR, 8);
                encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, 8);
                encoder.SetEncodingMethod(draco::MeshEncoderMethod::MESH_EDGEBREAKER_ENCODING);

                draco::EncoderBuffer dracoBuffer;
                draco::Status        status = encoder.EncodeMeshToBuffer(*primitive->dracoMesh, &dracoBuffer);
                assert(status.code() == draco::Status::OK);

                auto view = gltf->AddRawBufferView(buffer, dracoBuffer.data(), dracoBuffer.size());
                primitive->NoteDracoBuffer(*view);
            }
            mesh->AddPrimitive(primitive);
        }

        //
        // cameras
        //

        for (int i = 0; i < raw.GetCameraCount(); i++) {
            const RawCamera &cam    = raw.GetCamera(i);
            CameraData      &camera = *gltf->cameras.hold(new CameraData());
            camera.name = cam.name;

            if (cam.mode == RawCamera::CAMERA_MODE_PERSPECTIVE) {
                camera.type        = "perspective";
                camera.aspectRatio = cam.perspective.aspectRatio;
                camera.yfov        = cam.perspective.fovDegreesY * ((float) M_PI / 180.0f);
                camera.znear       = cam.perspective.nearZ;
                camera.zfar        = cam.perspective.farZ;
            } else {
                camera.type  = "orthographic";
                camera.xmag  = cam.orthographic.magX;
                camera.ymag  = cam.orthographic.magY;
                camera.znear = cam.orthographic.nearZ;
                camera.zfar  = cam.orthographic.farZ;
            }
            // Add the camera to the node hierarchy.

            auto iter = nodesByName.find(cam.nodeName);
            if (iter == nodesByName.end()) {
                fmt::printf("Warning: Camera node name %s does not exist.\n", cam.nodeName);
                continue;
            }
            iter->second->AddCamera(cam.name);
        }
    }

    NodeData        &rootNode  = require(nodesByName, "RootNode");
    const SceneData &rootScene = *gltf->scenes.hold(new SceneData(defaultSceneName, rootNode));

    if (options.outputBinary) {
        // note: glTF binary is little-endian
        const char glbHeader[] = {
            'g', 'l', 'T', 'F',        // magic
            0x02, 0x00, 0x00, 0x00,        // version
            0x00, 0x00, 0x00, 0x00,        // total length: written in later
        };
        gltfOutStream.write(glbHeader, 12);

        // binary glTF 2.0 has a sub-header for each of the JSON and BIN chunks
        const char glb2JsonHeader[] = {
            0x00, 0x00, 0x00, 0x00, // chunk length: written in later
            'J', 'S', 'O', 'N',     // chunk type: 0x4E4F534A aka JSON
        };
        gltfOutStream.write(glb2JsonHeader, 8);
    }

    {
        std::vector<std::string> extensionsUsed, extensionsRequired;
        if (options.useKHRMatCom) {
            extensionsUsed.push_back(KHR_MATERIALS_COMMON);
            if (!options.usePBRSpecGloss && !options.usePBRMetRough) {
                extensionsRequired.push_back(KHR_MATERIALS_COMMON);
            }
        }
        if (options.usePBRSpecGloss) {
            extensionsUsed.push_back(KHR_MATERIALS_PBR_SPECULAR_GLOSSINESS);
            if (!options.useKHRMatCom && !options.usePBRMetRough) {
                extensionsRequired.push_back(KHR_MATERIALS_PBR_SPECULAR_GLOSSINESS);
            }
        }
        if (options.useDraco) {
            extensionsUsed.push_back(KHR_DRACO_MESH_COMPRESSION);
            extensionsRequired.push_back(KHR_DRACO_MESH_COMPRESSION);
        }

        json glTFJson {
          { "asset", {
              { "generator", "FBX2glTF" },
              { "version", "2.0" }}},
          { "scene", rootScene.ix }
        };
        if (!extensionsUsed.empty()) {
            glTFJson["extensionsUsed"] = extensionsUsed;
        }
        if (!extensionsRequired.empty()) {
            glTFJson["extensionsRequired"] = extensionsRequired;
        }

        gltf->serializeHolders(glTFJson);
        gltfOutStream << glTFJson.dump(options.outputBinary ? 0 : 4);
    }
    if (options.outputBinary) {
        uint32_t jsonLength = (uint32_t) gltfOutStream.tellp() - 20;
        // the binary body must begin on a 4-aligned address, so pad json with spaces if necessary
        while ((jsonLength % 4) != 0) {
            gltfOutStream.put(' ');
            jsonLength++;
        }

        uint32_t binHeader = (uint32_t) gltfOutStream.tellp();
        // binary glTF 2.0 has a sub-header for each of the JSON and BIN chunks
        const char glb2BinaryHeader[] = {
            0x00, 0x00, 0x00, 0x00, // chunk length: written in later
            'B', 'I', 'N', 0x00,    // chunk type: 0x004E4942 aka BIN
        };
        gltfOutStream.write(glb2BinaryHeader, 8);

        // append binary buffer directly to .glb file
        uint32_t binaryLength = gltf->binary->size();
        gltfOutStream.write((const char *) &(*gltf->binary)[0], binaryLength);
        while ((binaryLength % 4) != 0) {
            gltfOutStream.put('\0');
            binaryLength++;
        }
        uint32_t totalLength = (uint32_t) gltfOutStream.tellp();

        // seek back to sub-header for json chunk
        gltfOutStream.seekp(8);

        // write total length, little-endian
        gltfOutStream.put((totalLength >> 0) & 0xFF);
        gltfOutStream.put((totalLength >> 8) & 0xFF);
        gltfOutStream.put((totalLength >> 16) & 0xFF);
        gltfOutStream.put((totalLength >> 24) & 0xFF);

        // write JSON length, little-endian
        gltfOutStream.put((jsonLength >> 0) & 0xFF);
        gltfOutStream.put((jsonLength >> 8) & 0xFF);
        gltfOutStream.put((jsonLength >> 16) & 0xFF);
        gltfOutStream.put((jsonLength >> 24) & 0xFF);

        // seek back to the gltf 2.0 binary chunk header
        gltfOutStream.seekp(binHeader);

        // write total length, little-endian
        gltfOutStream.put((binaryLength >> 0) & 0xFF);
        gltfOutStream.put((binaryLength >> 8) & 0xFF);
        gltfOutStream.put((binaryLength >> 16) & 0xFF);
        gltfOutStream.put((binaryLength >> 24) & 0xFF);

        // be tidy and return write pointer to end-of-file
        gltfOutStream.seekp(0, std::ios::end);
    }

    return new ModelData(gltf->binary);
}
