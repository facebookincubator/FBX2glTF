/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Raw2Gltf.hpp"

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>

#include <stb_image.h>
#include <stb_image_write.h>

#include <utils/File_Utils.hpp>
#include "utils/Image_Utils.hpp"
#include "utils/String_Utils.hpp"

#include "raw/RawModel.hpp"

#include "gltf/properties/AccessorData.hpp"
#include "gltf/properties/AnimationData.hpp"
#include "gltf/properties/BufferData.hpp"
#include "gltf/properties/BufferViewData.hpp"
#include "gltf/properties/CameraData.hpp"
#include "gltf/properties/ImageData.hpp"
#include "gltf/properties/MaterialData.hpp"
#include "gltf/properties/MeshData.hpp"
#include "gltf/properties/NodeData.hpp"
#include "gltf/properties/PrimitiveData.hpp"
#include "gltf/properties/SamplerData.hpp"
#include "gltf/properties/SceneData.hpp"
#include "gltf/properties/SkinData.hpp"
#include "gltf/properties/TextureData.hpp"

#include "GltfModel.hpp"
#include "TextureBuilder.hpp"

typedef uint32_t TriangleIndex;

#define DEFAULT_SCENE_NAME "Root Scene"

/**
 * This method sanity-checks existance and then returns a *reference* to the *Data instance
 * registered under that name. This is safe in the context of this tool, where all such data
 * classes are guaranteed to stick around for the duration of the process.
 */
template <typename T>
T& require(std::map<std::string, std::shared_ptr<T>> map, const std::string& key) {
  auto iter = map.find(key);
  assert(iter != map.end());
  T& result = *iter->second;
  return result;
}

template <typename T>
T& require(std::map<long, std::shared_ptr<T>> map, long key) {
  auto iter = map.find(key);
  assert(iter != map.end());
  T& result = *iter->second;
  return result;
}

static const std::vector<TriangleIndex> getIndexArray(const RawModel& raw) {
  std::vector<TriangleIndex> result;

  for (int i = 0; i < raw.GetTriangleCount(); i++) {
    result.push_back((TriangleIndex)raw.GetTriangle(i).verts[0]);
    result.push_back((TriangleIndex)raw.GetTriangle(i).verts[1]);
    result.push_back((TriangleIndex)raw.GetTriangle(i).verts[2]);
  }
  return result;
}

ModelData* Raw2Gltf(
    std::ofstream& gltfOutStream,
    const std::string& outputFolder,
    const RawModel& raw,
    const GltfOptions& options) {
  if (verboseOutput) {
    fmt::printf("Building render model...\n");
    for (int i = 0; i < raw.GetMaterialCount(); i++) {
      fmt::printf(
          "Material %d: %s [shading: %s]\n",
          i,
          raw.GetMaterial(i).name.c_str(),
          Describe(raw.GetMaterial(i).info->shadingModel));
    }
    if (raw.GetVertexCount() > 2 * raw.GetTriangleCount()) {
      fmt::printf(
          "Warning: High vertex count. Make sure there are no unnecessary vertex attributes. (see -keepAttribute cmd-line option)");
    }
  }

  std::vector<RawModel> materialModels;
  raw.CreateMaterialModels(
      materialModels,
      options.useLongIndices == UseLongIndicesOptions::NEVER,
      options.keepAttribs,
      true);

  if (verboseOutput) {
    fmt::printf("%7d vertices\n", raw.GetVertexCount());
    fmt::printf("%7d triangles\n", raw.GetTriangleCount());
    fmt::printf("%7d textures\n", raw.GetTextureCount());
    fmt::printf("%7d nodes\n", raw.GetNodeCount());
    fmt::printf("%7d surfaces\n", (int)materialModels.size());
    fmt::printf("%7d animations\n", raw.GetAnimationCount());
    fmt::printf("%7d cameras\n", raw.GetCameraCount());
    fmt::printf("%7d lights\n", raw.GetLightCount());
  }

  std::unique_ptr<GltfModel> gltf(new GltfModel(options));

  std::map<long, std::shared_ptr<NodeData>> nodesById;
  std::map<long, std::shared_ptr<MaterialData>> materialsById;
  std::map<std::string, std::shared_ptr<TextureData>> textureByIndicesKey;
  std::map<long, std::shared_ptr<MeshData>> meshBySurfaceId;

  // for now, we only have one buffer; data->binary points to the same vector as that BufferData
  // does.
  BufferData& buffer = *gltf->defaultBuffer;
  {
    //
    // nodes
    //

    for (int i = 0; i < raw.GetNodeCount(); i++) {
      // assumption: RawNode index == NodeData index
      const RawNode& node = raw.GetNode(i);

      auto nodeData = gltf->nodes.hold(
          new NodeData(node.name, node.translation, node.rotation, node.scale, node.isJoint));

      if (options.enableUserProperties) {
        nodeData->userProperties = node.userProperties;
      }

      for (const auto& childId : node.childIds) {
        int childIx = raw.GetNodeById(childId);
        assert(childIx >= 0);
        nodeData->AddChildNode(childIx);
      }

      nodesById.insert(std::make_pair(node.id, nodeData));
    }

		std::vector<std::shared_ptr<AccessorData>> accessors;

    //
    // animations
    //

    for (int i = 0; i < raw.GetAnimationCount(); i++) {
      const RawAnimation& animation = raw.GetAnimation(i);

      if (animation.channels.size() == 0) {
        fmt::printf(
            "Warning: animation '%s' has zero channels. Skipping.\n", animation.name.c_str());
        continue;
      }

			AnimationData* animationData = nullptr;
			if (options.draco.enabledAnimation) {
				// create Draco KeyframeAnimation
				auto dracoKeyframeAnimation(std::make_shared<draco::KeyframeAnimation>());
				animationData = new AnimationData(animation.name, dracoKeyframeAnimation);
			} else {
				animationData = new AnimationData(animation.name);
			}
			AnimationData& aDat = *gltf->animations.hold(animationData);

			const auto timestampsAccessor =
				gltf->AddTimestampsToAnimation<float>(buffer, aDat, animation.times, GLT_FLOAT, draco::DT_FLOAT32);
			timestampsAccessor->min = { *std::min_element(std::begin(animation.times), std::end(animation.times)) };
			timestampsAccessor->max = { *std::max_element(std::begin(animation.times), std::end(animation.times)) };
			accessors.emplace_back(timestampsAccessor);

      if (verboseOutput) {
        fmt::printf(
            "Animation '%s' has %lu channels:\n",
            animation.name.c_str(),
            animation.channels.size());
      }

      for (size_t channelIx = 0; channelIx < animation.channels.size(); channelIx++) {
        const RawChannel& channel = animation.channels[channelIx];
        const RawNode& node = raw.GetNode(channel.nodeIndex);

        if (verboseOutput) {
          fmt::printf(
              "  Channel %lu (%s) has translations/rotations/scales/weights: [%lu, %lu, %lu, %lu]\n",
              channelIx,
              node.name.c_str(),
              channel.translations.size(),
              channel.rotations.size(),
              channel.scales.size(),
              channel.weights.size());
        }

        NodeData& nDat = require(nodesById, node.id);
				if (!channel.translations.empty()) {
					const ChannelDefinition<Vec3f> CHANNEL_TRANSLATIONS(
						"translation",
						channel.translations,
						GLT_VEC3F,
						draco::DT_FLOAT32);
					const auto _ =
						gltf->AddChannelToAnimation<Vec3f>(buffer, aDat, nDat, CHANNEL_TRANSLATIONS);
					accessors.emplace_back(_);
				}
				if (!channel.rotations.empty()) {
					const ChannelDefinition<Quatf> CHANNEL_ROTATIONS(
						"rotation",
						channel.rotations,
						GLT_QUATF,
						draco::DT_FLOAT32);
					const auto _ =
						gltf->AddChannelToAnimation<Quatf>(buffer, aDat, nDat, CHANNEL_ROTATIONS);
					accessors.emplace_back(_);
				}
				if (!channel.scales.empty()) {
					const ChannelDefinition<Vec3f> CHANNEL_SCALES(
						"scale",
						channel.scales,
						GLT_VEC3F,
						draco::DT_FLOAT32);
					const auto _ =
						gltf->AddChannelToAnimation<Vec3f>(buffer, aDat, nDat, CHANNEL_SCALES);
					accessors.emplace_back(_);
				}
				if (!channel.weights.empty()) {
					const ChannelDefinition<float> CHANNEL_WEIGHTS(
						"weights",
						channel.weights,
						{ CT_FLOAT, 1, "SCALAR" },
						draco::DT_FLOAT32);
					const auto _ =
						gltf->AddChannelToAnimation<float>(buffer, aDat, nDat, CHANNEL_WEIGHTS);
					accessors.emplace_back(_);
				}
      }

			if (options.draco.enabledAnimation) {
				draco::EncoderOptions encodeOptions = draco::EncoderOptions::CreateDefaultOptions();
				if (options.draco.animationCompressionLevel != -1) {
					int dracoSpeed = 10 - options.draco.animationCompressionLevel;
					int en = dracoSpeed;
					int de = dracoSpeed;
					encodeOptions.SetSpeed(en, de);
				}

				if (-1 != options.draco.quantBitsTimestamp) {
					// set quantization for timestamps.
					encodeOptions.SetAttributeInt(0, "quantization_bits", options.draco.quantBitsTimestamp);
				}

				if (-1 != options.draco.quantBitsKeyframe) {
					// set quantization for keyframes.
					for (int i = 1; i <= aDat.dracoKeyframeAnimation->num_animations(); ++i) {
						encodeOptions.SetAttributeInt(i, "quantization_bits", options.draco.quantBitsKeyframe);
					}
				}

				draco::EncoderBuffer dracoBuffer;
				draco::KeyframeAnimationEncoder encoder;
				draco::Status status = encoder.EncodeKeyframeAnimation(*(aDat.dracoKeyframeAnimation), encodeOptions, &dracoBuffer);
				assert(status.code() == draco::Status::OK);
				auto view = gltf->AddRawBufferView(buffer, dracoBuffer.data(), to_uint32(dracoBuffer.size()));
				dracoBuffer.Clear();

				for (auto accessor : accessors)
				{
					accessor->bufferView = view->ix;
					accessor->byteOffset = -1;
				}
			}
			accessors.clear();
    }

    //
    // samplers
    //

    // textures
    //

    TextureBuilder textureBuilder(raw, options, outputFolder, *gltf);

    //
    // materials
    //

    for (int materialIndex = 0; materialIndex < raw.GetMaterialCount(); materialIndex++) {
      const RawMaterial& material = raw.GetMaterial(materialIndex);
      const bool isTransparent = material.type == RAW_MATERIAL_TYPE_TRANSPARENT ||
          material.type == RAW_MATERIAL_TYPE_SKINNED_TRANSPARENT;

      Vec3f emissiveFactor;
      float emissiveIntensity;

      // acquire the texture of a specific RawTextureUsage as *TextData, or nullptr if none exists
      auto simpleTex = [&](RawTextureUsage usage) -> std::shared_ptr<TextureData> {
        return (material.textures[usage] >= 0)
            ? textureBuilder.simple(material.textures[usage], "simple")
            : nullptr;
      };

      TextureData* normalTexture = simpleTex(RAW_TEXTURE_USAGE_NORMAL).get();
      TextureData* emissiveTexture = simpleTex(RAW_TEXTURE_USAGE_EMISSIVE).get();
      TextureData* occlusionTexture = nullptr;

      std::shared_ptr<PBRMetallicRoughness> pbrMetRough;
      if (options.usePBRMetRough) {
        // albedo is a basic texture, no merging needed
        std::shared_ptr<TextureData> baseColorTex, aoMetRoughTex;

        Vec4f diffuseFactor;
        float metallic, roughness;
        if (material.info->shadingModel == RAW_SHADING_MODEL_PBR_MET_ROUGH) {
          /**
           * PBR FBX Material -> PBR Met/Rough glTF.
           *
           * METALLIC and ROUGHNESS textures are packed in G and B channels of a rough/met texture.
           * Other values translate directly.
           */
          RawMetRoughMatProps* props = (RawMetRoughMatProps*)material.info.get();

          // determine if we need to generate a combined map
          bool hasMetallicMap = material.textures[RAW_TEXTURE_USAGE_METALLIC] >= 0;
          bool hasRoughnessMap = material.textures[RAW_TEXTURE_USAGE_ROUGHNESS] >= 0;
          bool hasOcclusionMap = material.textures[RAW_TEXTURE_USAGE_OCCLUSION] >= 0;
          bool atLeastTwoMaps = hasMetallicMap ? (hasRoughnessMap || hasOcclusionMap)
                                               : (hasRoughnessMap && hasMetallicMap);
          if (!atLeastTwoMaps) {
            // this handles the case of 0 or 1 maps supplied
            aoMetRoughTex = hasMetallicMap
                ? simpleTex(RAW_TEXTURE_USAGE_METALLIC)
                : (hasRoughnessMap
                       ? simpleTex(RAW_TEXTURE_USAGE_ROUGHNESS)
                       : (hasOcclusionMap ? simpleTex(RAW_TEXTURE_USAGE_OCCLUSION) : nullptr));
          } else {
            // otherwise merge occlusion into the red channel, metallic into blue channel, and
            // roughness into the green, of a new combinatory texture
            aoMetRoughTex = textureBuilder.combine(
                {
                    material.textures[RAW_TEXTURE_USAGE_OCCLUSION],
                    material.textures[RAW_TEXTURE_USAGE_METALLIC],
                    material.textures[RAW_TEXTURE_USAGE_ROUGHNESS],
                },
                "ao_met_rough",
                [&](const std::vector<const TextureBuilder::pixel*> pixels)
                    -> TextureBuilder::pixel {
                  const float occlusion = (*pixels[0])[0];
                  const float metallic = (*pixels[1])[0] * (hasMetallicMap ? 1 : props->metallic);
                  const float roughness =
                      (*pixels[2])[0] * (hasRoughnessMap ? 1 : props->roughness);
                  return {{occlusion,
                           props->invertRoughnessMap ? 1.0f - roughness : roughness,
                           metallic,
                           1}};
                },
                false);
          }
          baseColorTex = simpleTex(RAW_TEXTURE_USAGE_ALBEDO);
          diffuseFactor = props->diffuseFactor;
          metallic = props->metallic;
          roughness = props->roughness;
          emissiveFactor = props->emissiveFactor;
          emissiveIntensity = props->emissiveIntensity;
          // this will set occlusionTexture to null, if no actual occlusion map exists
          occlusionTexture = aoMetRoughTex.get();
        } else {
          /**
           * Traditional FBX Material -> PBR Met/Rough glTF.
           *
           * Diffuse channel is used as base colour. Simple constants for metallic and roughness.
           */
          const RawTraditionalMatProps* props = ((RawTraditionalMatProps*)material.info.get());
          diffuseFactor = props->diffuseFactor;

          if (material.info->shadingModel == RAW_SHADING_MODEL_BLINN ||
              material.info->shadingModel == RAW_SHADING_MODEL_PHONG) {
            // blinn/phong hardcoded to 0.4 metallic
            metallic = 0.4f;

            // fairly arbitrary conversion equation, with properties:
            //   shininess 0 -> roughness 1
            //   shininess 2 -> roughness ~0.7
            //   shininess 6 -> roughness 0.5
            //   shininess 16 -> roughness ~0.33
            //   as shininess ==> oo, roughness ==> 0
            auto getRoughness = [&](float shininess) { return sqrtf(2.0f / (2.0f + shininess)); };

            aoMetRoughTex = textureBuilder.combine(
                {
                    material.textures[RAW_TEXTURE_USAGE_SHININESS],
                },
                "ao_met_rough",
                [&](const std::vector<const TextureBuilder::pixel*> pixels)
                    -> TextureBuilder::pixel {
                  // do not multiply with props->shininess; that doesn't work like the other
                  // factors.
                  float shininess = props->shininess * (*pixels[0])[0];
                  return {{0, getRoughness(shininess), metallic, 1}};
                },
                false);

            if (aoMetRoughTex != nullptr) {
              // if we successfully built a texture, factors are just multiplicative identity
              metallic = roughness = 1.0f;
            } else {
              // no shininess texture,
              roughness = getRoughness(props->shininess);
            }

          } else {
            metallic = 0.2f;
            roughness = 0.8f;
          }

          baseColorTex = simpleTex(RAW_TEXTURE_USAGE_DIFFUSE);

          emissiveFactor = props->emissiveFactor;
          emissiveIntensity = 1.0f;
        }
        pbrMetRough.reset(new PBRMetallicRoughness(
            baseColorTex.get(), aoMetRoughTex.get(), diffuseFactor, metallic, roughness));
      }

      std::shared_ptr<KHRCmnUnlitMaterial> khrCmnUnlitMat;
      if (options.useKHRMatUnlit) {
        normalTexture = nullptr;

        emissiveTexture = nullptr;
        emissiveFactor = Vec3f(0.00f, 0.00f, 0.00f);

        Vec4f diffuseFactor;
        std::shared_ptr<TextureData> baseColorTex;

        if (material.info->shadingModel == RAW_SHADING_MODEL_PBR_MET_ROUGH) {
          RawMetRoughMatProps* props = (RawMetRoughMatProps*)material.info.get();
          diffuseFactor = props->diffuseFactor;
          baseColorTex = simpleTex(RAW_TEXTURE_USAGE_ALBEDO);
        } else {
          RawTraditionalMatProps* props = ((RawTraditionalMatProps*)material.info.get());
          diffuseFactor = props->diffuseFactor;
          baseColorTex = simpleTex(RAW_TEXTURE_USAGE_DIFFUSE);
        }

        pbrMetRough.reset(
            new PBRMetallicRoughness(baseColorTex.get(), nullptr, diffuseFactor, 0.0f, 1.0f));

        khrCmnUnlitMat.reset(new KHRCmnUnlitMaterial());
      }
      if (!occlusionTexture) {
        occlusionTexture = simpleTex(RAW_TEXTURE_USAGE_OCCLUSION).get();
      }

      std::shared_ptr<MaterialData> mData = gltf->materials.hold(new MaterialData(
          material.name,
          isTransparent,
          material.info->shadingModel,
          normalTexture,
          occlusionTexture,
          emissiveTexture,
          emissiveFactor * emissiveIntensity,
          khrCmnUnlitMat,
          pbrMetRough));
      materialsById[material.id] = mData;

      if (options.enableUserProperties) {
        mData->userProperties = material.userProperties;
      }
    }

    for (const auto& surfaceModel : materialModels) {
      assert(surfaceModel.GetSurfaceCount() == 1);
      const RawSurface& rawSurface = surfaceModel.GetSurface(0);
      const long surfaceId = rawSurface.id;

      const RawMaterial& rawMaterial =
          surfaceModel.GetMaterial(surfaceModel.GetTriangle(0).materialIndex);
      const MaterialData& mData = require(materialsById, rawMaterial.id);

      MeshData* mesh = nullptr;
      auto meshIter = meshBySurfaceId.find(surfaceId);
      if (meshIter != meshBySurfaceId.end()) {
        mesh = meshIter->second.get();

      } else {
        std::vector<float> defaultDeforms;
        for (const auto& channel : rawSurface.blendChannels) {
          defaultDeforms.push_back(channel.defaultDeform);
        }
        auto meshPtr = gltf->meshes.hold(new MeshData(rawSurface.name, defaultDeforms));
        meshBySurfaceId[surfaceId] = meshPtr;
        mesh = meshPtr.get();
      }

      bool useLongIndices = (options.useLongIndices == UseLongIndicesOptions::ALWAYS) ||
          (options.useLongIndices == UseLongIndicesOptions::AUTO &&
           surfaceModel.GetVertexCount() > 65535);

      std::shared_ptr<PrimitiveData> primitive;
      if (options.draco.enabledMesh) {
        size_t triangleCount = surfaceModel.GetTriangleCount();

        // initialize Draco mesh with vertex index information
        auto dracoMesh(std::make_shared<draco::Mesh>());
        dracoMesh->SetNumFaces(triangleCount);
        dracoMesh->set_num_points(surfaceModel.GetVertexCount());

        for (uint32_t ii = 0; ii < triangleCount; ii++) {
          draco::Mesh::Face face;
          face[0] = surfaceModel.GetTriangle(ii).verts[0];
          face[1] = surfaceModel.GetTriangle(ii).verts[1];
          face[2] = surfaceModel.GetTriangle(ii).verts[2];
          dracoMesh->SetFace(draco::FaceIndex(ii), face);
        }

				std::shared_ptr<AccessorData> indexes =
            gltf->accessors.hold(new AccessorData(useLongIndices ? GLT_UINT : GLT_USHORT));
        indexes->count = to_uint32(3 * triangleCount);
        primitive.reset(new PrimitiveData(*indexes, mData, dracoMesh));
				accessors.emplace_back(indexes);

      } else {
        const AccessorData& indexes = *gltf->AddAccessorWithView(
            *gltf->GetAlignedBufferView(buffer, BufferViewData::GL_ELEMENT_ARRAY_BUFFER),
            useLongIndices ? GLT_UINT : GLT_USHORT,
            getIndexArray(surfaceModel),
            std::string(""));
        primitive.reset(new PrimitiveData(indexes, mData));
      };

      //
      // surface vertices
      //
      {
        if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_POSITION) != 0) {
          const AttributeDefinition<Vec3f> ATTR_POSITION(
              "POSITION",
              &RawVertex::position,
              GLT_VEC3F,
              draco::GeometryAttribute::POSITION,
              draco::DT_FLOAT32);
          auto accessor =
              gltf->AddAttributeToPrimitive<Vec3f>(buffer, surfaceModel, *primitive, ATTR_POSITION);

          accessor->min = toStdVec(rawSurface.bounds.min);
          accessor->max = toStdVec(rawSurface.bounds.max);
					accessors.emplace_back(accessor);
        }
        if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_NORMAL) != 0) {
          const AttributeDefinition<Vec3f> ATTR_NORMAL(
              "NORMAL",
              &RawVertex::normal,
              GLT_VEC3F,
              draco::GeometryAttribute::NORMAL,
              draco::DT_FLOAT32);
          const auto _ =
              gltf->AddAttributeToPrimitive<Vec3f>(buffer, surfaceModel, *primitive, ATTR_NORMAL);
					accessors.emplace_back(_);
				}
        if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_TANGENT) != 0) {
          const AttributeDefinition<Vec4f> ATTR_TANGENT("TANGENT", &RawVertex::tangent, GLT_VEC4F);
          const auto _ = gltf->AddAttributeToPrimitive<Vec4f>(buffer, surfaceModel, *primitive, ATTR_TANGENT);
					accessors.emplace_back(_);
        }
        if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_COLOR) != 0) {
          const AttributeDefinition<Vec4f> ATTR_COLOR(
              "COLOR_0",
              &RawVertex::color,
              GLT_VEC4F,
              draco::GeometryAttribute::COLOR,
              draco::DT_FLOAT32);
          const auto _ =
              gltf->AddAttributeToPrimitive<Vec4f>(buffer, surfaceModel, *primitive, ATTR_COLOR);
					accessors.emplace_back(_);
				}
        if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_UV0) != 0) {
          const AttributeDefinition<Vec2f> ATTR_TEXCOORD_0(
              "TEXCOORD_0",
              &RawVertex::uv0,
              GLT_VEC2F,
              draco::GeometryAttribute::TEX_COORD,
              draco::DT_FLOAT32);
          const auto _ = gltf->AddAttributeToPrimitive<Vec2f>(
              buffer, surfaceModel, *primitive, ATTR_TEXCOORD_0);
					accessors.emplace_back(_);
        }
        if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_UV1) != 0) {
          const AttributeDefinition<Vec2f> ATTR_TEXCOORD_1(
              "TEXCOORD_1",
              &RawVertex::uv1,
              GLT_VEC2F,
              draco::GeometryAttribute::TEX_COORD,
              draco::DT_FLOAT32);
          const auto _ = gltf->AddAttributeToPrimitive<Vec2f>(
              buffer, surfaceModel, *primitive, ATTR_TEXCOORD_1);
					accessors.emplace_back(_);
        }
        if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_JOINT_INDICES) != 0) {
          const AttributeDefinition<Vec4i> ATTR_JOINTS(
              "JOINTS_0",
              &RawVertex::jointIndices,
              GLT_VEC4I,
              draco::GeometryAttribute::GENERIC,
              draco::DT_UINT16);
          const auto _ =
              gltf->AddAttributeToPrimitive<Vec4i>(buffer, surfaceModel, *primitive, ATTR_JOINTS);
					accessors.emplace_back(_);
				}
        if ((surfaceModel.GetVertexAttributes() & RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS) != 0) {
          const AttributeDefinition<Vec4f> ATTR_WEIGHTS(
              "WEIGHTS_0",
              &RawVertex::jointWeights,
              GLT_VEC4F,
              draco::GeometryAttribute::GENERIC,
              draco::DT_FLOAT32);
          const auto _ =
              gltf->AddAttributeToPrimitive<Vec4f>(buffer, surfaceModel, *primitive, ATTR_WEIGHTS);
					accessors.emplace_back(_);
				}

        // each channel present in the mesh always ends up a target in the primitive
        for (int channelIx = 0; channelIx < rawSurface.blendChannels.size(); channelIx++) {
          const auto& channel = rawSurface.blendChannels[channelIx];

          // track the bounds of each shape channel
          Bounds<float, 3> shapeBounds;

          std::vector<Vec3f> positions, normals;
          std::vector<Vec4f> tangents;
          for (int jj = 0; jj < surfaceModel.GetVertexCount(); jj++) {
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
              GLT_VEC3F,
              positions,
              channel.name);
          pAcc->min = toStdVec(shapeBounds.min);
          pAcc->max = toStdVec(shapeBounds.max);

          std::shared_ptr<AccessorData> nAcc;
          if (!normals.empty()) {
            nAcc = gltf->AddAccessorWithView(
                *gltf->GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_BUFFER),
                GLT_VEC3F,
                normals,
                channel.name);
          }

          std::shared_ptr<AccessorData> tAcc;
          if (!tangents.empty()) {
            nAcc = gltf->AddAccessorWithView(
                *gltf->GetAlignedBufferView(buffer, BufferViewData::GL_ARRAY_BUFFER),
                GLT_VEC4F,
                tangents,
                channel.name);
          }

          primitive->AddTarget(pAcc.get(), nAcc.get(), tAcc.get());
        }
      }
      if (options.draco.enabledMesh) {
        // Set up the encoder.
        draco::Encoder encoder;

        if (options.draco.compressionLevel != -1) {
          int dracoSpeed = 10 - options.draco.compressionLevel;
          encoder.SetSpeedOptions(dracoSpeed, dracoSpeed);
        }
        if (options.draco.quantBitsPosition != -1) {
          encoder.SetAttributeQuantization(
              draco::GeometryAttribute::POSITION, options.draco.quantBitsPosition);
        }
        if (options.draco.quantBitsTexCoord != -1) {
          encoder.SetAttributeQuantization(
              draco::GeometryAttribute::TEX_COORD, options.draco.quantBitsTexCoord);
        }
        if (options.draco.quantBitsNormal != -1) {
          encoder.SetAttributeQuantization(
              draco::GeometryAttribute::NORMAL, options.draco.quantBitsNormal);
        }
        if (options.draco.quantBitsColor != -1) {
          encoder.SetAttributeQuantization(
              draco::GeometryAttribute::COLOR, options.draco.quantBitsColor);
        }
        if (options.draco.quantBitsGeneric != -1) {
          encoder.SetAttributeQuantization(
              draco::GeometryAttribute::GENERIC, options.draco.quantBitsGeneric);
        }

        draco::EncoderBuffer dracoBuffer;
        draco::Status status = encoder.EncodeMeshToBuffer(*primitive->dracoMesh, &dracoBuffer);
        assert(status.code() == draco::Status::OK);

        auto view = gltf->AddRawBufferView(buffer, dracoBuffer.data(), to_uint32(dracoBuffer.size()));
        primitive->NoteDracoBuffer(*view);
				dracoBuffer.Clear();

				for (auto accessor : accessors)
				{
					accessor->bufferView = view->ix;
					accessor->byteOffset = -1;
				}
      }
			accessors.clear();
      mesh->AddPrimitive(primitive);
    }

    //
    // Assign meshes to node
    //

    for (int i = 0; i < raw.GetNodeCount(); i++) {
      const RawNode& node = raw.GetNode(i);
      auto nodeData = gltf->nodes.ptrs[i];

      //
      // Assign mesh to node
      //
      if (node.surfaceId > 0) {
        int surfaceIndex = raw.GetSurfaceById(node.surfaceId);
        const RawSurface& rawSurface = raw.GetSurface(surfaceIndex);

        MeshData& meshData = require(meshBySurfaceId, rawSurface.id);
        nodeData->SetMesh(meshData.ix);

        //
        // surface skin
        //
        if (!rawSurface.jointIds.empty()) {
          if (nodeData->skin == -1) {
            // glTF uses column-major matrices
            std::vector<Mat4f> inverseBindMatrices;
            for (const auto& inverseBindMatrice : rawSurface.inverseBindMatrices) {
              inverseBindMatrices.push_back(inverseBindMatrice.Transpose());
            }

            std::vector<uint32_t> jointIndexes;
            for (const auto& jointId : rawSurface.jointIds) {
              jointIndexes.push_back(require(nodesById, jointId).ix);
            }

            // Write out inverseBindMatrices
            auto accIBM = gltf->AddAccessorAndView(buffer, GLT_MAT4F, inverseBindMatrices);

            auto skeletonRoot = require(nodesById, rawSurface.skeletonRootId);
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
      const RawCamera& cam = raw.GetCamera(i);
      CameraData& camera = *gltf->cameras.hold(new CameraData());
      camera.name = cam.name;

      if (cam.mode == RawCamera::CAMERA_MODE_PERSPECTIVE) {
        camera.type = "perspective";
        camera.aspectRatio = cam.perspective.aspectRatio;
        camera.yfov = cam.perspective.fovDegreesY * ((float)M_PI / 180.0f);
        camera.znear = cam.perspective.nearZ;
        camera.zfar = cam.perspective.farZ;
      } else {
        camera.type = "orthographic";
        camera.xmag = cam.orthographic.magX;
        camera.ymag = cam.orthographic.magY;
        camera.znear = cam.orthographic.nearZ;
        camera.zfar = cam.orthographic.farZ;
      }
      // Add the camera to the node hierarchy.

      auto iter = nodesById.find(cam.nodeId);
      if (iter == nodesById.end()) {
        fmt::printf("Warning: Camera node id %lu does not exist.\n", cam.nodeId);
        continue;
      }
      iter->second->SetCamera(camera.ix);
    }

    //
    // lights
    //
    std::vector<json> khrPunctualLights;
    if (options.useKHRLightsPunctual) {
      for (int i = 0; i < raw.GetLightCount(); i++) {
        const RawLight& light = raw.GetLight(i);
        LightData::Type type;
        switch (light.type) {
          case RAW_LIGHT_TYPE_DIRECTIONAL:
            type = LightData::Type::Directional;
            break;
          case RAW_LIGHT_TYPE_POINT:
            type = LightData::Type::Point;
            break;
          case RAW_LIGHT_TYPE_SPOT:
            type = LightData::Type::Spot;
            break;
        }
        const auto _ = gltf->lights.hold(new LightData(
            light.name,
            type,
            light.color,
            // FBX intensity defaults to 100, so let's call that 1.0;
            // but caveat: I find nothing in the documentation to suggest
            // what unit the FBX value is meant to be measured in...
            light.intensity / 100,
            light.innerConeAngle,
            light.outerConeAngle));
      }
      for (int i = 0; i < raw.GetNodeCount(); i++) {
        const RawNode& node = raw.GetNode(i);
        const auto nodeData = gltf->nodes.ptrs[i];

        if (node.lightIx >= 0) {
          // we lean on the fact that in this simple case, raw and gltf indexing are aligned
          nodeData->SetLight(node.lightIx);
        }
      }
    }
  }

  NodeData& rootNode = require(nodesById, raw.GetRootNode());
  const SceneData& rootScene = *gltf->scenes.hold(new SceneData(DEFAULT_SCENE_NAME, rootNode));

  if (options.outputBinary) {
    // note: glTF binary is little-endian
    const char glbHeader[] = {
        'g',
        'l',
        'T',
        'F', // magic
        0x02,
        0x00,
        0x00,
        0x00, // version
        0x00,
        0x00,
        0x00,
        0x00, // total length: written in later
    };
    gltfOutStream.write(glbHeader, 12);

    // binary glTF 2.0 has a sub-header for each of the JSON and BIN chunks
    const char glb2JsonHeader[] = {
        0x00,
        0x00,
        0x00,
        0x00, // chunk length: written in later
        'J',
        'S',
        'O',
        'N', // chunk type: 0x4E4F534A aka JSON
    };
    gltfOutStream.write(glb2JsonHeader, 8);
  }

  {
    std::vector<std::string> extensionsUsed, extensionsRequired;
    if (options.useKHRMatUnlit) {
      extensionsUsed.push_back(KHR_MATERIALS_CMN_UNLIT);
    }
    if (!gltf->lights.ptrs.empty()) {
      extensionsUsed.push_back(KHR_LIGHTS_PUNCTUAL);
    }
    if (options.draco.enabledMesh) {
      extensionsUsed.push_back(KHR_DRACO_MESH_COMPRESSION);
      extensionsRequired.push_back(KHR_DRACO_MESH_COMPRESSION);
    }
		if (options.draco.enabledAnimation) {
			extensionsUsed.push_back(DRACO_ANIMATION_COMPRESSION);
			extensionsRequired.push_back(DRACO_ANIMATION_COMPRESSION);
		}

    json glTFJson{{"asset", {{"generator", "FBX2glTF v" + FBX2GLTF_VERSION}, {"version", "2.0"}}},
                  {"scene", rootScene.ix}};
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
    uint32_t jsonLength = (uint32_t)gltfOutStream.tellp() - 20;
    // the binary body must begin on a 4-aligned address, so pad json with spaces if necessary
    while ((jsonLength % 4) != 0) {
      gltfOutStream.put(' ');
      jsonLength++;
    }

    uint32_t binHeader = (uint32_t)gltfOutStream.tellp();
    // binary glTF 2.0 has a sub-header for each of the JSON and BIN chunks
    const char glb2BinaryHeader[] = {
        0x00,
        0x00,
        0x00,
        0x00, // chunk length: written in later
        'B',
        'I',
        'N',
        0x00, // chunk type: 0x004E4942 aka BIN
    };
    gltfOutStream.write(glb2BinaryHeader, 8);

    // append binary buffer directly to .glb file
    size_t binaryLength = gltf->binary->size();
    gltfOutStream.write((const char*)&(*gltf->binary)[0], binaryLength);
    while ((binaryLength % 4) != 0) {
      gltfOutStream.put('\0');
      binaryLength++;
    }
    uint32_t totalLength = to_uint32(gltfOutStream.tellp());

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
