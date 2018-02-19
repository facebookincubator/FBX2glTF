/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "MaterialData.h"
#include "TextureData.h"

// TODO: retrieve & pass in correct UV set from FBX
std::unique_ptr<Tex> Tex::ref(const TextureData *tex, uint32_t texCoord)
{
    return std::unique_ptr<Tex> { (tex != nullptr) ? new Tex(tex->ix, texCoord) : nullptr };
}

Tex::Tex(uint32_t texRef, uint32_t texCoord)
    : texRef(texRef),
      texCoord(texCoord) {}

void to_json(json &j, const Tex &data) {
    j = json {
        { "index", data.texRef },
        { "texCoord", data.texCoord }
    };
}

KHRCmnUnlitMaterial::KHRCmnUnlitMaterial()
{
}

void to_json(json &j, const KHRCmnUnlitMaterial &d)
{
	j = json({});
}

PBRMetallicRoughness::PBRMetallicRoughness(
    const TextureData *baseColorTexture, const TextureData *metRoughTexture,
    const Vec4f &baseColorFactor, float metallic, float roughness)
    : baseColorTexture(Tex::ref(baseColorTexture)),
      metRoughTexture(Tex::ref(metRoughTexture)),
      baseColorFactor(baseColorFactor),
      metallic(metallic),
      roughness(roughness)
{
}

void to_json(json &j, const PBRMetallicRoughness &d)
{
    j = { };
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
    std::string name,  bool isTransparent, const TextureData *normalTexture,
    const TextureData *emissiveTexture, const Vec3f & emissiveFactor,
    std::shared_ptr<KHRCmnUnlitMaterial> const khrCmnConstantMaterial,
    std::shared_ptr<PBRMetallicRoughness> const pbrMetallicRoughness)
    : Holdable(),
      name(std::move(name)),
      isTransparent(isTransparent),
      normalTexture(Tex::ref(normalTexture)),
      emissiveTexture(Tex::ref(emissiveTexture)),
      emissiveFactor(emissiveFactor),
      khrCmnConstantMaterial(khrCmnConstantMaterial),
      pbrMetallicRoughness(pbrMetallicRoughness) {}

json MaterialData::serialize() const
{
    json result = {
        { "name", name },
        { "alphaMode", isTransparent ? "BLEND" : "OPAQUE" }
    };

    if (normalTexture != nullptr) {
        result["normalTexture"] = *normalTexture;
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
        json extensions = { };
        extensions[KHR_MATERIALS_CMN_UNLIT] = *khrCmnConstantMaterial;
        result["extensions"] = extensions;
    }
    return result;
}
