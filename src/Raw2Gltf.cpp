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

template<typename T>
T &require(std::map<long, std::shared_ptr<T>> map, long key)
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
                Describe(raw.GetMaterial(i).info->shadingModel));
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
    std::map<std::string, std::shared_ptr<TextureData>>  textureByIndicesKey;
    std::map<long, std::shared_ptr<MeshData>>            meshBySurfaceId;

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
        typedef std::array<float, 4> pixel; // pixel components are floats in [0, 1]
        typedef std::function<pixel(const std::vector<const pixel *>)> pixel_merger;

        auto texIndicesKey = [&](std::vector<int> ixVec, std::string tag) -> std::string {
            std::string result = tag;
            for (int ix : ixVec) {
                result += "_";
                result += ix;
            }
            return result;
        };

        /**
         * Create a new derived TextureData for the two given RawTexture indexes, or return a previously created one.
         * Each pixel in the derived texture will be determined from its equivalent in each source pixels, as decided
         * by the provided `combine` function.
         */
        auto getDerivedTexture = [&](
            std::vector<int> rawTexIndices,
            const pixel_merger &combine,
            const std::string &tag
        ) -> std::shared_ptr<TextureData>
        {
            const std::string key = texIndicesKey(rawTexIndices, tag);
            auto iter = textureByIndicesKey.find(key);
            if (iter != textureByIndicesKey.end()) {
                return iter->second;
            }

            auto describeChannel = [&](int channels) -> std::string {
                switch(channels) {
                    case 1: return "G";
                    case 2: return "GA";
                    case 3: return "RGB";
                    case 4: return "RGBA";
                    default:
                        return fmt::format("?%d?", channels);
                }
            };

            // keep track of some texture data as we load them
            struct TexInfo {
                explicit TexInfo(int rawTexIx) : rawTexIx(rawTexIx) {}

                const int rawTexIx;
                int width {};
                int height {};
                int channels {};
                uint8_t *pixels {};
            };

            int width = -1, height = -1;
            std::string mergedName = tag;
            std::string mergedFilename = tag;
            std::vector<TexInfo> texes { };
            for (const int rawTexIx : rawTexIndices) {
                TexInfo info(rawTexIx);
                if (rawTexIx >= 0) {
                    const RawTexture &rawTex = raw.GetTexture(rawTexIx);
                    const std::string &fileLoc = rawTex.fileLocation;
                    const std::string &fileLocBase = Gltf::StringUtils::GetFileBaseString(Gltf::StringUtils::GetFileNameString(fileLoc));
                    if (!fileLoc.empty()) {
                        info.pixels = stbi_load(fileLoc.c_str(), &info.width, &info.height, &info.channels, 4);
                        if (!info.pixels) {
                            fmt::printf("Warning: merge texture [%d](%s) could not be loaded.\n",
                                rawTexIx,
                                Gltf::StringUtils::GetFileBaseString(Gltf::StringUtils::GetFileNameString(fileLoc)));
                        } else {
                            if (width < 0) {
                                width = info.width;
                                height = info.height;
                            } else if (width != info.width || height != info.height) {
                                fmt::printf("Warning: texture %s (%d, %d) can't be merged with previous texture(s) of dimension (%d, %d)\n",
                                    Gltf::StringUtils::GetFileBaseString(Gltf::StringUtils::GetFileNameString(fileLoc)),
                                    info.width, info.height, width, height);
                                // this is bad enough that we abort the whole merge
                                return nullptr;
                            }
                            mergedName += "_" + rawTex.fileName;
                            mergedFilename += "_" + fileLocBase;
                        }
                    }
                }
                texes.push_back(info);
            }

            if (width < 0) {
                // no textures to merge; bail
                return nullptr;
            }
            // TODO: which channel combinations make sense in input files?

            std::vector<uint8_t > mergedPixels(4 * width * height);
            std::vector<pixel> pixels(texes.size());
            std::vector<const pixel *> pixelPointers(texes.size());
            for (int ii = 0; ii < mergedPixels.size(); ii += 4) {
                pixels.clear();
                for (int jj = 0; jj < texes.size(); jj ++) {
                    const TexInfo &tex = texes[jj];
                    if (tex.pixels != nullptr) {
                        pixels[jj] = { tex.pixels[ii+0]/255.0f, tex.pixels[ii+1]/255.0f, tex.pixels[ii+2]/255.0f, tex.pixels[ii+3]/255.0f };
                    } else {
                        // absent textures are to be treated as all ones
                        pixels[jj] = { 1.0f, 1.0f, 1.0f, 1.0f };
                    }
                    pixelPointers[jj] = &pixels[jj];
                }
                const pixel &merged = combine(pixelPointers);
                for (int jj = 0; jj < 4; jj ++) {
                    mergedPixels[ii + jj] = static_cast<uint8_t>(fmax(0, fmin(255.0f, merged[jj] * 255.0f)));
                }
            }

            bool png = true;
            std::vector<char> imgBuffer;
            int res;
            if (png) {
                res = stbi_write_png_to_func(WriteToVectorContext<char>, &imgBuffer, width, height, 4, mergedPixels.data(), width * 4);
            } else {
                res = stbi_write_jpg_to_func(WriteToVectorContext<char>, &imgBuffer, width, height, 4, mergedPixels.data(), 80);
            }
            if (!res) {
                fmt::printf("Warning: failed to generate merge texture '%s'.\n", mergedFilename);
                return nullptr;
            }

            ImageData *image;
            if (options.outputBinary) {
                const auto bufferView = gltf->AddRawBufferView(buffer, imgBuffer.data(), imgBuffer.size());
                image = new ImageData(mergedName, *bufferView, png ? "image/png" : "image/jpeg");

            } else {
                const std::string imageFilename = mergedFilename + (png ? ".png" : ".jpg");
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

                image = new ImageData(mergedName, imageFilename);
            }

            std::shared_ptr<TextureData> texDat = gltf->textures.hold(
                new TextureData(mergedName, defaultSampler, *gltf->images.hold(image)));
            textureByIndicesKey.insert(std::make_pair(key, texDat));
            return texDat;
        };

        /** Create a new TextureData for the given RawTexture index, or return a previously created one. */
        auto getSimpleTexture = [&](int rawTexIndex, const std::string &tag) {
            const std::string key = texIndicesKey({ rawTexIndex }, tag);
            auto iter = textureByIndicesKey.find(key);
            if (iter != textureByIndicesKey.end()) {
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
            textureByIndicesKey.insert(std::make_pair(key, texDat));
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

            Vec3f emissiveFactor;
            float emissiveIntensity;

            const Vec3f dielectric(0.04f, 0.04f, 0.04f);
            const float epsilon = 1e-6f;

            // acquire the texture of a specific RawTextureUsage as *TextData, or nullptr if none exists
            auto simpleTex = [&](RawTextureUsage usage) -> std::shared_ptr<TextureData> {
                return (material.textures[usage] >= 0) ? getSimpleTexture(material.textures[usage], "simple") : nullptr;
            };

            // acquire derived texture of two RawTextureUsage as *TextData, or nullptr if neither exists
            auto merge2Tex = [&](
                const std::string tag,
                RawTextureUsage u1,
                RawTextureUsage u2,
                const pixel_merger &combine
            ) -> std::shared_ptr<TextureData> {
                return getDerivedTexture({ material.textures[u1], material.textures[u2] }, combine, tag);
            };

            // acquire derived texture of two RawTextureUsage as *TextData, or nullptr if neither exists
            auto merge3Tex = [&](
                const std::string tag,
                RawTextureUsage u1,
                RawTextureUsage u2,
                RawTextureUsage u3,
                const pixel_merger &combine
            ) -> std::shared_ptr<TextureData> {
                return getDerivedTexture({ material.textures[u1], material.textures[u2], material.textures[u3] }, combine, tag);
            };

            auto getMaxComponent = [&](const Vec3f &color) {
                return fmax(color.x, fmax(color.y, color.z));
            };
            auto getPerceivedBrightness = [&](const Vec3f &color) {
                return sqrt(0.299 * color.x * color.x + 0.587 * color.y * color.y + 0.114 * color.z * color.z);
            };
            auto toVec3f = [&](const pixel &pix) -> const Vec3f {
                return Vec3f(pix[0], pix[1], pix[2]);
            };
            auto toVec4f = [&](const pixel &pix) -> const Vec4f {
                return Vec4f(pix[0], pix[1], pix[2], pix[3]);
            };

            std::shared_ptr<PBRMetallicRoughness> pbrMetRough;
            if (options.usePBRMetRough) {
                // albedo is a basic texture, no merging needed
                std::shared_ptr<TextureData> baseColorTex, metRoughTex;

                Vec4f diffuseFactor;
                float metallic, roughness;
                if (material.info->shadingModel == RAW_SHADING_MODEL_PBR_MET_ROUGH) {
                    /**
                     * PBR FBX Material -> PBR Met/Rough glTF.
                     *
                     * METALLIC and ROUGHNESS textures are packed in G and B channels of a rough/met texture.
                     * Other values translate directly.
                     */
                    RawMetRoughMatProps *props = (RawMetRoughMatProps *) material.info.get();
                    // merge metallic into the blue channel and roughness into the green, of a new combinatory texture
                    metRoughTex = merge2Tex("met_rough",
                        RAW_TEXTURE_USAGE_METALLIC, RAW_TEXTURE_USAGE_ROUGHNESS,
                        [&](const std::vector<const pixel *> pixels) -> pixel { return { 0, (*pixels[1])[0], (*pixels[0])[0], 0 }; });
                    baseColorTex      = simpleTex(RAW_TEXTURE_USAGE_ALBEDO);
                    diffuseFactor     = props->diffuseFactor;
                    metallic          = props->metallic;
                    roughness         = props->roughness;
                    emissiveFactor    = props->emissiveFactor;
                    emissiveIntensity = props->emissiveIntensity;
                } else {
                    /**
                     * Traditional FBX Material -> PBR Met/Rough glTF.
                     *
                     * Diffuse channel is used as base colour. No metallic/roughness texture is attempted. Constant
                     * metallic/roughness values are hard-coded.
                     *
                     * TODO: If we have specular & optional shininess map, we can generate a reasonable rough/met map.
                     */
                    const RawTraditionalMatProps *props = ((RawTraditionalMatProps *) material.info.get());
                    diffuseFactor = props->diffuseFactor;
                    // TODO: make configurable on the command line, or do a better job guessing from other, supplied params
                    if (material.info->shadingModel == RAW_SHADING_MODEL_LAMBERT) {
                        metallic  = 0.6f;
                        roughness = 1.0f;
                    } else {
                        metallic  = 0.2f;
                        roughness = 0.6f;
                    }
                    auto solveMetallic = [&](float pDiff, float pSpec, float oneMinusSpecularStrength)
                    {
                        if (pSpec < dielectric.x) {
                            return 0.0;
                        }

                        float a = dielectric.x;
                        float b = pDiff * oneMinusSpecularStrength / (1 - dielectric.x) + pSpec - 2 * dielectric.x;
                        float c = dielectric.x - pSpec;
                        float D = fmax(b * b - 4 * a * c, 0);
                        return fmax(0.0, fmin(1.0, (-b + sqrt(D)) / (2 * a)));
                    };
                    metRoughTex = merge3Tex("rough_met",
                        RAW_TEXTURE_USAGE_SPECULAR, RAW_TEXTURE_USAGE_SHININESS, RAW_TEXTURE_USAGE_DIFFUSE,
                        [&](const std::vector<const pixel *> pixels) -> pixel {
                            const Vec3f specular = pixels[0] ? toVec3f(*pixels[0]) : props->specularFactor;
                            float shininess = pixels[1] ? (*pixels[1])[0] : props->shininess;
                            const Vec4f diffuse = pixels[2] ? toVec4f(*pixels[2]) : props->diffuseFactor;

                            float pixelMet = solveMetallic(
                                getPerceivedBrightness(diffuse.xyz()),
                                getPerceivedBrightness(specular),
                                1 - getMaxComponent(specular));
                            float pixelRough = 1 - shininess;

                            return { 0, pixelRough, pixelMet, 0 };
                    });
                    if (material.textures[RAW_TEXTURE_USAGE_DIFFUSE] >= 0) {
                        baseColorTex = merge2Tex("base_col",
                            RAW_TEXTURE_USAGE_DIFFUSE, RAW_TEXTURE_USAGE_SPECULAR,
                        [&](const std::vector<const pixel *> pixels) -> pixel {
                            const Vec4f diffuse = pixels[0] ? toVec4f(*pixels[0]) : props->diffuseFactor;
                            const Vec3f specular = pixels[1] ? toVec3f(*pixels[1]) : props->specularFactor;

                            float oneMinus = 1 - getMaxComponent(specular);

                            float pixelMet = solveMetallic(
                                getPerceivedBrightness(diffuse.xyz()),
                                getPerceivedBrightness(specular),
                                oneMinus);

                            Vec3f fromDiffuse = diffuse.xyz() * (oneMinus / (1.0 - dielectric.x) / fmax(1.0 - pixelMet, epsilon));
                            Vec3f fromSpecular = specular - dielectric * (1.0 - pixelMet) * (1.0 / fmax(pixelMet, epsilon));
                            Vec3f baseColor = Vec3f::Lerp(fromDiffuse, fromSpecular, pixelMet * pixelMet);

                            return { baseColor[0], baseColor[1], baseColor[2], diffuse[3] };
                        });
                    }
                    emissiveFactor    = props->emissiveFactor;
                    emissiveIntensity = 1.0f;
                }
                pbrMetRough.reset(new PBRMetallicRoughness(baseColorTex.get(), metRoughTex.get(), diffuseFactor, metallic, roughness));
            }

            std::shared_ptr<PBRSpecularGlossiness> pbrSpecGloss;
            if (options.usePBRSpecGloss) {
                Vec4f                        diffuseFactor;
                Vec3f                        specularFactor;
                float                        glossiness;
                std::shared_ptr<TextureData> specGlossTex;
                std::shared_ptr<TextureData> diffuseTex;

                const Vec3f dielectric(0.04f, 0.04f, 0.04f);
                const float epsilon = 1e-6f;
                if (material.info->shadingModel == RAW_SHADING_MODEL_PBR_MET_ROUGH) {
                    /**
                     * PBR FBX Material -> PBR Spec/Gloss glTF.
                     *
                     * TODO: A complete mess. Low priority.
                     */
                    RawMetRoughMatProps *props = (RawMetRoughMatProps *) material.info.get();
                    // we can estimate spec/gloss from met/rough by between Vec4f(0.04, 0.04, 0.04) and baseColor
                    // according to metalness, and then taking gloss to be the inverse of roughness
                    specGlossTex = merge3Tex("specgloss",
                        RAW_TEXTURE_USAGE_ALBEDO, RAW_TEXTURE_USAGE_METALLIC, RAW_TEXTURE_USAGE_ROUGHNESS,
                        [&](const std::vector<const pixel *> pixels) -> pixel {
                            Vec3f baseColor(1.0f);
                            if (pixels[0]) {
                                baseColor.x = (*pixels[0])[0];
                                baseColor.y = (*pixels[0])[1];
                                baseColor.z = (*pixels[0])[2];
                            }
                            float metallic = pixels[1] ? (*pixels[1])[0] : 1.0f;
                            float roughness = pixels[2] ? (*pixels[2])[0] : 1.0f;
                            Vec3f spec = Vec3f::Lerp(dielectric, baseColor, metallic);
                            return { spec[0], spec[1], spec[2], 1.0f - roughness };
                         });
                    diffuseTex = merge2Tex("albedo",
                        RAW_TEXTURE_USAGE_ALBEDO, RAW_TEXTURE_USAGE_METALLIC,
                        [&](const std::vector<const pixel *> pixels) -> pixel {
                            Vec3f baseColor(1.0f);
                            float alpha = 1.0f;
                            if (pixels[0]) {
                                baseColor[0] = (*pixels[0])[0];
                                baseColor[1] = (*pixels[0])[1];
                                baseColor[2] = (*pixels[0])[2];
                                alpha = (*pixels[0])[3];
                            }
                            float metallic = pixels[1] ? (*pixels[1])[0] : 1.0f;
                            Vec3f spec = Vec3f::Lerp(dielectric, baseColor, metallic);
                            float maxSpecComp = fmax(fmax(spec.x, spec.y), spec.z);
                            // attenuate baseColor to get specgloss-compliant diffuse; for details consult
                            // https://github.com/KhronosGroup/glTF/tree/master/extensions/Khronos/KHR_materials_pbrSpecularGlossiness
                            Vec3f diffuse = baseColor * (1 - dielectric[0]) * (1 - metallic) * fmax(1.0f - maxSpecComp, epsilon);
                            return { diffuse[0], diffuse[1], diffuse[2], alpha };
                         });
                    diffuseFactor = props->diffuseFactor;
                    specularFactor = Vec3f::Lerp(dielectric, props->diffuseFactor.xyz(), props->metallic);
                    glossiness = 1.0f - props->roughness;

                } else {
                    /**
                     * Traditional FBX Material -> PBR Spec/Gloss glTF.
                     *
                     * TODO: A complete mess. Low priority.
                     */
                    // TODO: this section a ludicrous over-simplifictation; we can surely do better.
                    const RawTraditionalMatProps *props = ((RawTraditionalMatProps *) material.info.get());
                    specGlossTex = merge2Tex("specgloss",
                        RAW_TEXTURE_USAGE_SPECULAR, RAW_TEXTURE_USAGE_SHININESS,
                        [&](const std::vector<const pixel *> pixels) -> pixel {
                            const auto &spec = *(pixels[0]);
                            const auto &shine = *(pixels[1]);
                            return { spec[0], spec[1], spec[2], shine[0] };
                        });
                    diffuseTex = simpleTex(RAW_TEXTURE_USAGE_DIFFUSE);
                    diffuseFactor = props->diffuseFactor;
                    specularFactor = props->specularFactor;
                    glossiness = props->shininess;
                }

                pbrSpecGloss.reset(
                    new PBRSpecularGlossiness(
                        diffuseTex.get(), diffuseFactor, specGlossTex.get(), specularFactor, glossiness));
            }

            std::shared_ptr<KHRCommonMats> khrComMat;
            if (options.useKHRMatCom) {
                float                        shininess;
                Vec3f                        ambientFactor, specularFactor;
                Vec4f                        diffuseFactor;
                std::shared_ptr<TextureData> diffuseTex;
                auto                         type = KHRCommonMats::MaterialType::Constant;

                if (material.info->shadingModel == RAW_SHADING_MODEL_PBR_MET_ROUGH) {
                    /**
                     * PBR FBX Material -> KHR Common Materials glTF.
                     *
                     * TODO: We can use the specularFactor calculation below to generate a reasonable specular map, too.
                     */
                    const RawMetRoughMatProps *props = (RawMetRoughMatProps *) material.info.get();
                    shininess = 1.0f - props->roughness;
                    ambientFactor = Vec3f(0.0f, 0.0f, 0.0f);
                    diffuseTex = simpleTex(RAW_TEXTURE_USAGE_ALBEDO);
                    diffuseFactor = props->diffuseFactor;
                    specularFactor = Vec3f::Lerp(Vec3f(0.04f, 0.04f, 0.04f), props->diffuseFactor.xyz(), props->metallic);
                    // render as Phong if there's any specularity, otherwise Lambert
                    type = (specularFactor.LengthSquared() > 1e-4) ?
                        KHRCommonMats::MaterialType::Phong : KHRCommonMats::MaterialType::Lambert;
                    // TODO: convert textures

                } else {
                    /**
                     * Traditional FBX Material -> KHR Common Materials glTF.
                     *
                     * Should be in good shape. Essentially pass-through.
                     */
                    const RawTraditionalMatProps *props = (RawTraditionalMatProps *) material.info.get();
                    shininess = props->shininess;
                    ambientFactor = props->ambientFactor;
                    diffuseTex = simpleTex(RAW_TEXTURE_USAGE_DIFFUSE);
                    diffuseFactor = props->diffuseFactor;
                    specularFactor = props->specularFactor;

                    if (material.info->shadingModel == RAW_SHADING_MODEL_LAMBERT) {
                        type = KHRCommonMats::MaterialType::Lambert;
                    } else if (material.info->shadingModel == RAW_SHADING_MODEL_BLINN) {
                        type = KHRCommonMats::MaterialType::Blinn;
                    } else if (material.info->shadingModel == RAW_SHADING_MODEL_PHONG) {
                        type = KHRCommonMats::MaterialType::Phong;
                    }
                }
                khrComMat.reset(
                    new KHRCommonMats(type,
                        simpleTex(RAW_TEXTURE_USAGE_SHININESS).get(), shininess,
                        simpleTex(RAW_TEXTURE_USAGE_AMBIENT).get(), ambientFactor,
                        diffuseTex.get(), diffuseFactor,
                        simpleTex(RAW_TEXTURE_USAGE_SPECULAR).get(), specularFactor));
            }
            std::shared_ptr<MaterialData> mData = gltf->materials.hold(
                new MaterialData(
                    material.name, isTransparent,
                    simpleTex(RAW_TEXTURE_USAGE_NORMAL).get(), simpleTex(RAW_TEXTURE_USAGE_EMISSIVE).get(),
                    emissiveFactor * emissiveIntensity,
                    khrComMat, pbrMetRough, pbrSpecGloss));
            materialsByName[materialHash(material)] = mData;
        }

        for (size_t surfaceIndex = 0; surfaceIndex < materialModels.size(); surfaceIndex++) {
            const RawModel &surfaceModel = materialModels[surfaceIndex];

            assert(surfaceModel.GetSurfaceCount() == 1);
            const RawSurface  &rawSurface = surfaceModel.GetSurface(0);
            const int surfaceId = rawSurface.id;

            const RawMaterial &rawMaterial = surfaceModel.GetMaterial(surfaceModel.GetTriangle(0).materialIndex);
            const MaterialData &mData = require(materialsByName, materialHash(rawMaterial));


            MeshData *mesh = nullptr;
            auto meshIter = meshBySurfaceId.find(surfaceId);
            if (meshIter != meshBySurfaceId.end()) {
                mesh = meshIter->second.get();

            } else {
                std::vector<float> defaultDeforms;
                for (const auto &channel : rawSurface.blendChannels) {
                    defaultDeforms.push_back(channel.defaultDeform);
                }
                auto meshPtr = gltf->meshes.hold(new MeshData(rawSurface.name, defaultDeforms));
                meshBySurfaceId[surfaceId] = meshPtr;
                mesh = meshPtr.get();
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
        // Assign meshes to node
        //

        for (int i = 0; i < raw.GetNodeCount(); i++) {

            const RawNode &node = raw.GetNode(i);
            auto nodeData = gltf->nodes.ptrs[i];

            //
            // Assign mesh to node
            //
            if (node.surfaceId > 0)
            {
                int surfaceIndex = raw.GetSurfaceById(node.surfaceId);
                const RawSurface &rawSurface = raw.GetSurface(surfaceIndex);

                MeshData &meshData = require(meshBySurfaceId, rawSurface.id);
                nodeData->SetMesh(meshData.ix);

                //
                // surface skin
                //
                if (!rawSurface.jointNames.empty()) {
                    if (nodeData->skin == -1) {
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
                        auto skin = *gltf->skins.hold(new SkinData(jointIndexes, *accIBM, skeletonRoot));
                        nodeData->SetSkin(skin.ix);
                    }
                }
            }
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
