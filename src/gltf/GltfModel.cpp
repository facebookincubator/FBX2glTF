/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "GltfModel.hpp"

std::shared_ptr<BufferViewData> GltfModel::GetAlignedBufferView(
    BufferData& buffer,
    const BufferViewData::GL_ArrayType target) {
  unsigned long bufferSize = this->binary->size();
  if ((bufferSize % 4) > 0) {
    bufferSize += (4 - (bufferSize % 4));
    this->binary->resize(bufferSize);
  }
  return this->bufferViews.hold(new BufferViewData(buffer, bufferSize, target));
}

// add a bufferview on the fly and copy data into it
std::shared_ptr<BufferViewData>
GltfModel::AddRawBufferView(BufferData& buffer, const char* source, uint32_t bytes) {
  auto bufferView = GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_NONE);
  bufferView->byteLength = bytes;

  // make space for the new bytes (possibly moving the underlying data)
  unsigned long bufferSize = this->binary->size();
  this->binary->resize(bufferSize + bytes);

  // and copy them into place
  memcpy(&(*this->binary)[bufferSize], source, bytes);
  return bufferView;
}

std::shared_ptr<BufferViewData> GltfModel::AddBufferViewForFile(
    BufferData& buffer,
    const std::string& filename) {
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

void GltfModel::serializeHolders(json& glTFJson) {
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
  if (!lights.ptrs.empty()) {
    json lightsJson = json::object();
    serializeHolder(lightsJson, "lights", lights);
    glTFJson["extensions"][KHR_LIGHTS_PUNCTUAL] = lightsJson;
  }
}
