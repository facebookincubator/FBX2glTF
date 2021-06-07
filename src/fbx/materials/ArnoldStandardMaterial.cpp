/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RoughnessMetallicMaterials.hpp"

std::unique_ptr<FbxRoughMetMaterialInfo> ArnoldStandardMaterialResolver::resolve() const {
  const FbxProperty mayaProp = fbxMaterial->FindProperty("Maya");
  if (mayaProp.GetPropertyDataType() != FbxCompoundDT) {
    return nullptr;
  }
  if (!fbxMaterial->ShadingModel.Get().IsEmpty()) {
    ::fmt::printf(
        "Warning: Material %s has surprising shading model: %s\n",
        fbxMaterial->GetName(),
        fbxMaterial->ShadingModel.Get());
  }

  auto getTex = [&](std::string propName) {
    const FbxFileTexture* ptr = nullptr;

    const FbxProperty useProp = mayaProp.FindHierarchical((propName).c_str());
    if (useProp.IsValid() && useProp.Get<bool>()) {
      const FbxProperty texProp = mayaProp.FindHierarchical((propName).c_str());
      if (texProp.IsValid()) {
        ptr = texProp.GetSrcObject<FbxFileTexture>();
        if (ptr != nullptr && textureLocations.find(ptr) == textureLocations.end()) {
          ptr = nullptr;
        }
      }
    }
    return ptr;
  };

  auto getVec3 = [&](std::string propName) -> FbxDouble3 {
    const FbxProperty vecProp = mayaProp.FindHierarchical(propName.c_str());
    return vecProp.IsValid() ? vecProp.Get<FbxDouble3>() : FbxDouble3(1, 1, 1);
  };  
  
  auto getVal = [&](std::string propName) -> FbxDouble {
    const FbxProperty vecProp = mayaProp.FindHierarchical(propName.c_str());
    return vecProp.IsValid() ? vecProp.Get<FbxDouble>() : 0;
  };

  enum PropertyDesc {
    TEXTURE_EMISSION,
    TEXTURE_AMBIENT_OCCLUSION,
    TEXTURE_NORMAL,
    TEXTURE_ROUGHNESS,
    TEXTURE_METALLIC,
    TEXTURE_ALBEDO,
    PROPERTY_DESC_NOT_FOUND,
    PROPERTY_DESC_ALBEDO_COLOR,
    PROPERTY_DESC_TRANSPARENT,
    PROPERTY_DESC_TRANSPARENT_COLOR,
    PROPERTY_DESC_METALLIC,
    PROPERTY_DESC_ROUGHNESS,
    PROPERTY_DESC_SPECULAR,
    PROPERTY_DESC_SPECULAR_COLOR,
    PROPERTY_DESC_SHINYNESS,
    PROPERTY_DESC_COAT,
    PROPERTY_DESC_COAT_ROUGHNESS,
    PROPERTY_DESC_EMISSIVE,
    PROPERTY_DESC_EMISSIVE_COLOR,
    PROPERTY_DESC_IGNORE,
    PROPERTY_DESC_UNSUPPORTED,
  };
  // See https://github.com/godotengine/godot/blob/master/modules/fbx/data/fbx_material.h
  const std::map<std::string, PropertyDesc> fbxPropertiesDesc =
      {
          /* Diffuse */
          {"Maya|base", TEXTURE_ALBEDO},
          {"DiffuseColor", TEXTURE_ALBEDO},
          {"Maya|DiffuseTexture", TEXTURE_ALBEDO},
          {"Maya|baseColor", TEXTURE_ALBEDO},
          {"Maya|baseColor|file", TEXTURE_ALBEDO},
          {"Maya|TEX_color_map|file", TEXTURE_ALBEDO},
          {"Maya|TEX_color_map", TEXTURE_ALBEDO},
          /* Emission */
          {"EmissiveColor", TEXTURE_EMISSION},
          {"EmissiveFactor", TEXTURE_EMISSION},
          {"Maya|emissionColor", TEXTURE_EMISSION},
          {"Maya|emissionColor|file", TEXTURE_EMISSION},
          {"Maya|TEX_emissive_map", TEXTURE_EMISSION},
          {"Maya|TEX_emissive_map|file", TEXTURE_EMISSION},
          /* Metallic */
          {"Maya|metalness", TEXTURE_METALLIC},
          {"Maya|metalness|file", TEXTURE_METALLIC},
          {"Maya|TEX_metallic_map", TEXTURE_METALLIC},
          {"Maya|TEX_metallic_map|file", TEXTURE_METALLIC},

          /* Roughness */
          // Arnold Roughness Map
          {"Maya|specularRoughness", TEXTURE_ROUGHNESS},

          {"Maya|TEX_roughness_map", TEXTURE_ROUGHNESS},
          {"Maya|TEX_roughness_map|file", TEXTURE_ROUGHNESS},

          /* Normal */
          {"NormalMap", TEXTURE_NORMAL},
          //{ "Bump", TEXTURE_NORMAL },
          //{ "3dsMax|Parameters|bump_map", TEXTURE_NORMAL },
          {"Maya|NormalTexture", TEXTURE_NORMAL},
          //{ "Maya|normalCamera", TEXTURE_NORMAL },
          //{ "Maya|normalCamera|file", TEXTURE_NORMAL },
          {"Maya|TEX_normal_map", TEXTURE_NORMAL},
          {"Maya|TEX_normal_map|file", TEXTURE_NORMAL},
          /* AO */
          {"Maya|TEX_ao_map", TEXTURE_AMBIENT_OCCLUSION},
          {"Maya|TEX_ao_map|file", TEXTURE_AMBIENT_OCCLUSION},

          // TODO: specular workflow conversion
          //		{ "SpecularColor", TEXTURE_METALLIC },
          //		{ "Maya|specularColor", TEXTURE_METALLIC },
          //		{ "Maya|SpecularTexture", TEXTURE_METALLIC },
          //		{ "Maya|SpecularTexture|file", TEXTURE_METALLIC },
          //		{ "ShininessExponent", PROPERTY_DESC_UNSUPPORTED },
          //		{ "ReflectionFactor", PROPERTY_DESC_UNSUPPORTED },

          //{ "TransparentColor",PROPERTY_DESC_TRANSPARENT_COLOR },
          //{ "TransparencyFactor",PROPERTY_DESC_TRANSPARENT }

          // TODO: diffuse roughness
          //{ "Maya|diffuseRoughness", PROPERTY_DESC_UNSUPPORTED },
          //{ "Maya|diffuseRoughness|file", PROPERTY_DESC_UNSUPPORTED },

          /* Albedo */
          {"DiffuseColor", PROPERTY_DESC_ALBEDO_COLOR},
          {"Maya|baseColor", PROPERTY_DESC_ALBEDO_COLOR},

          /* Specular */
          {"Maya|specular", PROPERTY_DESC_SPECULAR},
          {"Maya|specularColor", PROPERTY_DESC_SPECULAR_COLOR},

          /* Specular roughness - arnold roughness map */
          {"Maya|specularRoughness", PROPERTY_DESC_ROUGHNESS},

          /* Transparent */
          {"Opacity", PROPERTY_DESC_TRANSPARENT},
          {"TransparencyFactor", PROPERTY_DESC_TRANSPARENT},
          {"Maya|opacity", PROPERTY_DESC_TRANSPARENT},

          {"Maya|metalness", PROPERTY_DESC_METALLIC},
          {"Maya|metallic", PROPERTY_DESC_METALLIC},

          /* Roughness */
          {"Maya|roughness", PROPERTY_DESC_ROUGHNESS},

          /* Coat */
          //{ "Maya|coat", PROPERTY_DESC_COAT },

          /* Coat roughness */
          //{ "Maya|coatRoughness", PROPERTY_DESC_COAT_ROUGHNESS },

          /* Emissive */
          {"Maya|emission", PROPERTY_DESC_EMISSIVE},
          {"Maya|emissive", PROPERTY_DESC_EMISSIVE},

          /* Emissive color */
          {"EmissiveColor", PROPERTY_DESC_EMISSIVE_COLOR},
          {"Maya|emissionColor", PROPERTY_DESC_EMISSIVE_COLOR},
      };

std::unique_ptr<FbxRoughMetMaterialInfo> res(new FbxRoughMetMaterialInfo(
    fbxMaterial->GetUniqueID(),
    fbxMaterial->GetName(),
    FbxRoughMetMaterialInfo::FBX_SHADER_METROUGH,
    FbxDouble4(0.0, 0.0, 0.0, 1),
    0.0,
    1.0));
  for (std::pair<std::string, PropertyDesc> const& it : fbxPropertiesDesc) {
    switch (it.second) {
      case TEXTURE_EMISSION: {
        res->texEmissive = getTex(it.first);
        break;
      }
      case TEXTURE_AMBIENT_OCCLUSION: {
        res->texAmbientOcclusion = getTex(it.first);
        break;
      }
      case TEXTURE_NORMAL: {
        res->texNormal = getTex(it.first);
        break;
      }
      case TEXTURE_ROUGHNESS: {
        res->texRoughness = getTex(it.first);
        break;
      }
      case TEXTURE_METALLIC: {
        res->texMetallic = getTex(it.first);
        break;
      }
      case TEXTURE_ALBEDO: {
        res->texBaseColor = getTex(it.first);
        break;
      }
      case PROPERTY_DESC_NOT_FOUND: {
        break;
      }
      case PROPERTY_DESC_ALBEDO_COLOR: {
        FbxDouble3 baseColor = getVec3(it.first);
        res->baseColor = FbxVector4(baseColor[0],baseColor[1], baseColor[2], res->baseColor[3]);
        break;
      }
      case PROPERTY_DESC_TRANSPARENT: {
        res->baseColor = FbxVector4(res->baseColor[0], res->baseColor[1], res->baseColor[2], getVal(it.first));
        break;
      }
      case PROPERTY_DESC_METALLIC: {
        res->metallic = getVal(it.first);
        break;
      }
      case PROPERTY_DESC_ROUGHNESS: {
        res->roughness = getVal(it.first);
        break;
      }
      case PROPERTY_DESC_SPECULAR: {
        break;
      }
      case PROPERTY_DESC_SPECULAR_COLOR: {
        break;
      }
      case PROPERTY_DESC_SHINYNESS: {
        break;
      }
      case PROPERTY_DESC_COAT: {
        break;
      }
      case PROPERTY_DESC_COAT_ROUGHNESS: {
        break;
      }
      case PROPERTY_DESC_EMISSIVE: {
        res->emissiveIntensity = getVal(it.first);
        break;
      }
      case PROPERTY_DESC_EMISSIVE_COLOR: {
        res->emissive = getVec3(it.first);
        break;
      }
      case PROPERTY_DESC_IGNORE: {
        break;
      }
      default: {
        break;
      }
    }
  }
  return res;
};
