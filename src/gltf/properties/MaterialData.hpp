/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include "gltf/Raw2Gltf.hpp"

struct Tex {
  static std::unique_ptr<Tex> ref(const TextureData* tex, uint32_t texCoord = 0);
  explicit Tex(uint32_t texRef, uint32_t texCoord);

  const uint32_t texRef;
  const uint32_t texCoord;
};

struct KHRCmnUnlitMaterial {
  KHRCmnUnlitMaterial();
};

struct PBRMetallicRoughness {
  PBRMetallicRoughness(
      const TextureData* baseColorTexture,
      const TextureData* metRoughTexture,
      const Vec4f& baseColorFactor,
      float metallic = 0.1f,
      float roughness = 0.6f);

  std::unique_ptr<Tex> baseColorTexture;
  std::unique_ptr<Tex> metRoughTexture;
  const Vec4f baseColorFactor;
  const float metallic;
  const float roughness;
};

struct MaterialData : Holdable {
  MaterialData(
      std::string name,
      bool isTransparent,
      bool isDoubleSided,
      RawShadingModel shadingModel,
      const TextureData* normalTexture,
      const TextureData* occlusionTexture,
      const TextureData* emissiveTexture,
      const Vec3f& emissiveFactor,
      std::shared_ptr<KHRCmnUnlitMaterial> const khrCmnConstantMaterial,
      std::shared_ptr<PBRMetallicRoughness> const pbrMetallicRoughness);

  json serialize() const override;

  const std::string name;
  const RawShadingModel shadingModel;
  const bool isTransparent;
  const bool isDoubleSided;
  const std::unique_ptr<const Tex> normalTexture;
  const std::unique_ptr<const Tex> occlusionTexture;
  const std::unique_ptr<const Tex> emissiveTexture;
  const Vec3f emissiveFactor;

  const std::shared_ptr<const KHRCmnUnlitMaterial> khrCmnConstantMaterial;
  const std::shared_ptr<const PBRMetallicRoughness> pbrMetallicRoughness;

  std::vector<std::string> userProperties;
};

void to_json(json& j, const Tex& data);
void to_json(json& j, const KHRCmnUnlitMaterial& d);
void to_json(json& j, const PBRMetallicRoughness& d);
