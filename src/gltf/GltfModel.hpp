/**
* Copyright (c) 2014-present, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*/

#pragma once

#include "FBX2glTF.h"

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

struct GltfModel
{
    explicit GltfModel(bool _isGlb)
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
        BufferViewData &bufferView, const GLType &type, const std::vector<T> &source, std::string name)
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
        return AddAccessorWithView(*bufferView, type, source, std::string(""));
    }

    template<class T>
    std::shared_ptr<AccessorData> AddAccessorAndView(
        BufferData &buffer, const GLType &type, const std::vector<T> &source, std::string name)
    {
        auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_NONE);
        return AddAccessorWithView(*bufferView, type, source, name);
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
