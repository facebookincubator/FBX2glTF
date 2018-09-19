/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "RoughnessMetallicMaterials.hpp"

std::unique_ptr<FbxRoughMetMaterialInfo> Fbx3dsMaxPhysicalMaterialResolver::resolve() const {
    const FbxProperty topProp = fbxMaterial->FindProperty("3dsMax");
    if (topProp.GetPropertyDataType() != FbxCompoundDT) {
        return nullptr;
    }
    const FbxProperty props = fbxMaterial->FindProperty("Parameters");

    FbxString shadingModel = fbxMaterial->ShadingModel.Get();
    if (!shadingModel.IsEmpty() && shadingModel != "unknown") {
        ::fmt::printf("Warning: Material %s has surprising shading model: %s\n",
            fbxMaterial->GetName(), shadingModel);
    }

    auto getTex = [&](std::string propName) -> const FbxFileTexture * {
        const FbxFileTexture *ptr = nullptr;

        const FbxProperty useProp = props.FindHierarchical((propName + "_map_on").c_str());
        if (useProp.IsValid() && useProp.Get<bool>()) {
            const FbxProperty texProp = useProp.FindHierarchical((propName + "_map").c_str());
            if (texProp.IsValid()) {
                ptr = texProp.GetSrcObject<FbxFileTexture>();
                if (ptr != nullptr && textureLocations.find(ptr) == textureLocations.end()) {
                    ptr = nullptr;
                }
            }
        } else if (verboseOutput && useProp.IsValid()) {
            fmt::printf("Note: property '%s' of 3dsMax Physical material '%s' exists, but is flagged as 'off'.\n",
                propName, fbxMaterial->GetName());
        }
        return ptr;
    };

    int materialMode = getValue(props, "material_mode", 0);
    fmt::printf("Note: 3dsMax Physical material has material_mode = %d.\n", materialMode);

    // baseWeight && baseColor
    FbxDouble baseWeight = getValue(props, "base_weight", 1.0);
    const auto *baseWeightMap = getTex("base_weight");
    FbxDouble4 baseCol = getValue(props, "base_color", FbxDouble4(0.5, 0.5, 0.5, 1.0));
    const auto *baseTex = getTex("base_color");

    double emissiveWeight = getValue(props, "emission", 0.0);
    const auto *emissiveWeightMap = getTex("emission");
    FbxDouble4 emissiveColor = getValue(props, "emit_color", FbxDouble4(1, 1, 1, 1));
    const auto *emissiveColorMap = getTex("emit_color");
    // TODO: emit_luminance, emit_kelvin?

    // roughness & metalness: supported
    double roughness = getValue(props, "roughness", 0.0);
    const auto *roughnessMap = getTex("roughness");
    double metalness = getValue(props, "metalness", 0.0);
    const auto *metalnessMap = getTex("metalness");

    // TODO: does invertRoughness affect roughness_map too?
    bool invertRoughness = getValue(props, "inv_roughness", false);
    if (invertRoughness) {
        roughness = 1.0f - roughness;
    }

    // TODO: attempt to bake transparency > 0.0f into the alpha of baseColour?
    double transparency = getValue(props, "transparency", 0.0);
    const auto *transparencyMap = getTex("transparency");

    // SSS: not supported
    double scattering = getValue(props, "scattering", 0.0);
    const auto *scatteringMap = getTex("scattering");

    // reflectivity: not supported
    double reflectivityWeight = getValue(props, "reflectivity", 1.);
    const auto *reflectivityWeightMap = getTex("reflectivity");
    FbxDouble4 reflectivityColor = getValue(props, "refl_color", FbxDouble4(1, 1, 1, 1));
    const auto *reflectivityColorMap = getTex("refl_color");

    // coatings: not supported
    double coating = getValue(props, "coating", 0.0);

    // diffuse roughness: not supported
    double diffuseRoughness = getValue(props, "diff_roughness", 0.);

    // explicit brdf curve control: not supported
    bool isBrdfMode = getValue(props, "brdf_mode", false);

    // anisotrophy: not supported
    double anisotropy = getValue(props, "anisotropy", 1.0);

    // TODO: how the heck do we combine these to generate a normal map?
    const auto *bumpMap = getTex("bump");
    const auto *displacementMap = getTex("displacement");

    std::unique_ptr<FbxRoughMetMaterialInfo> res(
        new FbxRoughMetMaterialInfo(
            fbxMaterial->GetName(),
            FbxRoughMetMaterialInfo::FBX_SHADER_METROUGH,
            baseCol,
            metalness,
            roughness
        )
    );
    res->texBaseColor = baseTex;
    res->baseWeight = baseWeight;
    res->texBaseWeight = baseWeightMap;

    res->texMetallic = metalnessMap;
    res->texRoughness = roughnessMap;

    res->texNormal = bumpMap; // TODO LOL NO NONO

    res->emissive = emissiveColor;
    res->emissiveIntensity = emissiveWeight;
    res->texEmissive = emissiveColorMap;
    res->texEmissiveWeight = emissiveWeightMap;

    return res;
}
