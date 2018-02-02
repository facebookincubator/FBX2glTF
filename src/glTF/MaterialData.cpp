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

static inline Vec4f toRGBA(const Vec3f &colour, float alpha = 1) {
    return { colour[0], colour[1], colour[2], alpha };
};

KHRCommonMats::KHRCommonMats(
    MaterialType type,
    const TextureData *shininessTexture, float shininess,
    const TextureData *ambientTexture, const Vec3f &ambientFactor,
    const TextureData *diffuseTexture, const Vec4f &diffuseFactor,
    const TextureData *specularTexture, const Vec3f &specularFactor)
    : type(type),
      shininessTexture(Tex::ref(shininessTexture)),
      shininess(shininess),
      ambientTexture(Tex::ref(ambientTexture)),
      ambientFactor(ambientFactor),
      diffuseTexture(Tex::ref(diffuseTexture)),
      diffuseFactor(diffuseFactor),
      specularTexture(Tex::ref(specularTexture)),
      specularFactor(specularFactor)
{
}

std::string KHRCommonMats::typeDesc(MaterialType type)
{
    switch (type) {
        case Blinn:
            return "commonBlinn";
        case Phong:
            return "commonPhong";
        case Lambert:
            return "commonLambert";
        case Constant:
            return "commonConstant";
    }
}

void to_json(json &j, const KHRCommonMats &d)
{
    j = {{"type", KHRCommonMats::typeDesc(d.type)}};

    if (d.shininessTexture != nullptr) {
        j["shininessTexture"] = *d.shininessTexture;
    }
    if (d.shininess != 0) {
        j["shininess"] = d.shininess;
    }
    if (d.ambientTexture != nullptr) {
        j["ambientTexture"] = *d.ambientTexture;
    }
    if (d.ambientFactor.LengthSquared() > 0) {
        j["ambientFactor"] = toStdVec(toRGBA(d.ambientFactor));
    }
    if (d.diffuseTexture != nullptr) {
        j["diffuseTexture"] = *d.diffuseTexture;
    }
    if (d.diffuseFactor.LengthSquared() > 0) {
        j["diffuseFactor"] = toStdVec(d.diffuseFactor);
    }
    if (d.specularTexture != nullptr) {
        j["specularTexture"] = *d.specularTexture;
    }
    if (d.specularFactor.LengthSquared() > 0) {
        j["specularFactor"] = toStdVec(toRGBA(d.specularFactor));
    }
}

KHRCmnUnlitMaterial::KHRCmnUnlitMaterial()
{
}

void to_json(json &j, const KHRCmnUnlitMaterial &d)
{
	j = json({});
}

PBRSpecularGlossiness::PBRSpecularGlossiness(
    const TextureData *diffuseTexture, const Vec4f &diffuseFactor,
    const TextureData *specularGlossinessTexture, const Vec3f &specularFactor,
    float glossinessFactor)
    : diffuseTexture(Tex::ref(diffuseTexture)),
      diffuseFactor(diffuseFactor),
      specularGlossinessTexture(Tex::ref(specularGlossinessTexture)),
      specularFactor(specularFactor),
      glossinessFactor(glossinessFactor)
{
}

void to_json(json &j, const PBRSpecularGlossiness &d)
{
    j = {};
    if (d.diffuseTexture != nullptr) {
        j["diffuseTexture"] = *d.diffuseTexture;
    }
    if (d.diffuseFactor.LengthSquared() > 0) {
        j["diffuseFactor"] = toStdVec(d.diffuseFactor);
    }
    if (d.specularGlossinessTexture != nullptr) {
        j["specularGlossinessTexture"] = *d.specularGlossinessTexture;
    }
    if (d.specularFactor.LengthSquared() > 0) {
        j["specularFactor"] = toStdVec(d.specularFactor);
    }
    if (d.glossinessFactor != 0) {
        j["glossinessFactor"] = d.glossinessFactor;
    }
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
    std::shared_ptr<KHRCommonMats> const khrCommonMats,
    std::shared_ptr<KHRCmnUnlitMaterial> const khrCmnConstantMaterial,
    std::shared_ptr<PBRMetallicRoughness> const pbrMetallicRoughness,
    std::shared_ptr<PBRSpecularGlossiness> const pbrSpecularGlossiness)
    : Holdable(),
      name(std::move(name)),
      isTransparent(isTransparent),
      normalTexture(Tex::ref(normalTexture)),
      emissiveTexture(Tex::ref(emissiveTexture)),
      emissiveFactor(std::move(emissiveFactor)),
      khrCommonMats(khrCommonMats),
      khrCmnConstantMaterial(khrCmnConstantMaterial),
      pbrMetallicRoughness(pbrMetallicRoughness),
      pbrSpecularGlossiness(pbrSpecularGlossiness) {}

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
    if (khrCommonMats != nullptr || khrCmnConstantMaterial != nullptr || pbrSpecularGlossiness != nullptr) {
        json extensions = { };
        if (khrCommonMats != nullptr) {
            extensions[KHR_MATERIALS_COMMON] = *khrCommonMats;
        }
        if (khrCmnConstantMaterial != nullptr) {
            extensions[KHR_MATERIALS_CMN_UNLIT] = *khrCmnConstantMaterial;
        }
        if (pbrSpecularGlossiness != nullptr) {
            extensions[KHR_MATERIALS_PBR_SPECULAR_GLOSSINESS] = *pbrSpecularGlossiness;
        }

        result["extensions"] = extensions;
    }
    return result;
}
