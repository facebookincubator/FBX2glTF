/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MaterialData.hpp"
#include "TextureData.hpp"

// TODO: retrieve & pass in correct UV set from FBX
std::unique_ptr<Tex> Tex::ref(const TextureData* tex, uint32_t texCoord) {
  return std::unique_ptr<Tex>{(tex != nullptr) ? new Tex(tex->ix, texCoord) : nullptr};
}

Tex::Tex(uint32_t texRef, uint32_t texCoord) : texRef(texRef), texCoord(texCoord) {}

void to_json(json& j, const Tex& data) {
  j = json{{"index", data.texRef}, {"texCoord", data.texCoord}};
}

KHRCmnUnlitMaterial::KHRCmnUnlitMaterial() {}

void to_json(json& j, const KHRCmnUnlitMaterial& d) {
  j = json({});
}

inline float clamp(float d, float bottom = 0, float top = 1) {
  return std::max(bottom, std::min(top, d));
}
inline Vec3f
clamp(const Vec3f& vec, const Vec3f& bottom = VEC3F_ZERO, const Vec3f& top = VEC3F_ONE) {
  return Vec3f::Max(bottom, Vec3f::Min(top, vec));
}
inline Vec4f
clamp(const Vec4f& vec, const Vec4f& bottom = VEC4F_ZERO, const Vec4f& top = VEC4F_ONE) {
  return Vec4f::Max(bottom, Vec4f::Min(top, vec));
}

PBRMetallicRoughness::PBRMetallicRoughness(
    const TextureData* baseColorTexture,
    const TextureData* metRoughTexture,
    const Vec4f& baseColorFactor,
    float metallic,
    float roughness)
    : baseColorTexture(Tex::ref(baseColorTexture)),
      metRoughTexture(Tex::ref(metRoughTexture)),
      baseColorFactor(clamp(baseColorFactor)),
      metallic(clamp(metallic)),
      roughness(clamp(roughness)) {}

void to_json(json& j, const PBRMetallicRoughness& d) {
  j = {};
  if (d.baseColorTexture != nullptr) {
    j["baseColorTexture"] = *d.baseColorTexture;
  }
  if (d.baseColorFactor.LengthSquared() > 0) {
    j["baseColorFactor"] = toStdVec(d.baseColorFactor);
  }
  if (d.metRoughTexture != nullptr) {
    j["metallicRoughnessTexture"] = *d.metRoughTexture;
    // if a texture is provided, throw away metallic/roughness values
    j["roughnessFactor"] = 1.0f;
    j["metallicFactor"] = 1.0f;
  } else {
    // without a texture, however, use metallic/roughness as constants
    j["metallicFactor"] = d.metallic;
    j["roughnessFactor"] = d.roughness;
  }
}

MaterialData::MaterialData(
    std::string name,
    bool isTransparent,
    const RawShadingModel shadingModel,
    const TextureData* normalTexture,
    const TextureData* occlusionTexture,
    const TextureData* emissiveTexture,
    const Vec3f& emissiveFactor,
    std::shared_ptr<KHRCmnUnlitMaterial> const khrCmnConstantMaterial,
    std::shared_ptr<PBRMetallicRoughness> const pbrMetallicRoughness)
    : Holdable(),
      name(std::move(name)),
      shadingModel(shadingModel),
      isTransparent(isTransparent),
      normalTexture(Tex::ref(normalTexture)),
      occlusionTexture(Tex::ref(occlusionTexture)),
      emissiveTexture(Tex::ref(emissiveTexture)),
      emissiveFactor(clamp(emissiveFactor)),
      khrCmnConstantMaterial(khrCmnConstantMaterial),
      pbrMetallicRoughness(pbrMetallicRoughness) {}

json MaterialData::serialize() const {
  json result = {{"name", name},
                 {"alphaMode", isTransparent ? "BLEND" : "OPAQUE"},
                 {"extras",
                  {{"fromFBX",
                    {{"shadingModel", Describe(shadingModel)},
                     {"isTruePBR", shadingModel == RAW_SHADING_MODEL_PBR_MET_ROUGH}}}}}};

  if (normalTexture != nullptr) {
    result["normalTexture"] = *normalTexture;
  }
  if (occlusionTexture != nullptr) {
    result["occlusionTexture"] = *occlusionTexture;
  }
  if (emissiveTexture != nullptr) {
    result["emissiveTexture"] = *emissiveTexture;
  }
  if (emissiveFactor.LengthSquared() > 0) {
    result["emissiveFactor"] = toStdVec(emissiveFactor);
  }
  if (pbrMetallicRoughness != nullptr) {
    result["pbrMetallicRoughness"] = *pbrMetallicRoughness;
  }
  if (khrCmnConstantMaterial != nullptr) {
    json extensions = {};
    extensions[KHR_MATERIALS_CMN_UNLIT] = *khrCmnConstantMaterial;
    result["extensions"] = extensions;
  }

  for (const auto& i : userProperties) {
    auto& prop_map = result["extras"]["fromFBX"]["userProperties"];

    json j = json::parse(i);
    for (const auto& k : json::iterator_wrapper(j)) {
      prop_map[k.key()] = k.value();
    }
  }

  return result;
}
