/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

#include <CLI11.hpp>

#include "FBX2glTF.h"
#include "fbx/Fbx2Raw.hpp"
#include "gltf/Raw2Gltf.hpp"
#include "utils/File_Utils.hpp"
#include "utils/String_Utils.hpp"

bool verboseOutput = false;

int main(int argc, char* argv[]) {
  GltfOptions gltfOptions;

  CLI::App app{
      fmt::sprintf(
          "FBX2glTF %s: Generate a glTF 2.0 representation of an FBX model.", FBX2GLTF_VERSION),
      "FBX2glTF"};

  app.add_flag(
      "-v,--verbose",
      verboseOutput,
      "Include blend shape tangents, if reported present by the FBX SDK.");

  app.add_flag_function("-V,--version", [&](size_t count) {
    fmt::printf("FBX2glTF version %s\nCopyright (c) 2016-2018 Oculus VR, LLC.\n", FBX2GLTF_VERSION);
    exit(0);
  });

  std::string inputPath;
  app.add_option("FBX Model", inputPath, "The FBX model to convert.")->check(CLI::ExistingFile);
  app.add_option("-i,--input", inputPath, "The FBX model to convert.")->check(CLI::ExistingFile);

  std::string outputPath;
  app.add_option("-o,--output", outputPath, "Where to generate the output, without suffix.");

  app.add_flag(
      "-e,--embed",
      gltfOptions.embedResources,
      "Inline buffers as data:// URIs within generated non-binary glTF.");
  app.add_flag("-b,--binary", gltfOptions.outputBinary, "Output a single binary format .glb file.");

  app.add_option(
         "--long-indices",
         [&](std::vector<std::string> choices) -> bool {
           for (const std::string choice : choices) {
             if (choice == "never") {
               gltfOptions.useLongIndices = UseLongIndicesOptions::NEVER;
             } else if (choice == "auto") {
               gltfOptions.useLongIndices = UseLongIndicesOptions::AUTO;
             } else if (choice == "always") {
               gltfOptions.useLongIndices = UseLongIndicesOptions::ALWAYS;
             } else {
               fmt::printf("Unknown --long-indices: %s\n", choice);
               throw CLI::RuntimeError(1);
             }
           }
           return true;
         },
         "Whether to use 32-bit indices.")
      ->type_name("(never|auto|always)");

  app.add_option(
         "--compute-normals",
         [&](std::vector<std::string> choices) -> bool {
           for (const std::string choice : choices) {
             if (choice == "never") {
               gltfOptions.computeNormals = ComputeNormalsOption::NEVER;
             } else if (choice == "broken") {
               gltfOptions.computeNormals = ComputeNormalsOption::BROKEN;
             } else if (choice == "missing") {
               gltfOptions.computeNormals = ComputeNormalsOption::MISSING;
             } else if (choice == "always") {
               gltfOptions.computeNormals = ComputeNormalsOption::ALWAYS;
             } else {
               fmt::printf("Unknown --compute-normals option: %s\n", choice);
               throw CLI::RuntimeError(1);
             }
           }
           return true;
         },
         "When to compute vertex normals from mesh geometry.")
      ->type_name("(never|broken|missing|always)");

  app.add_option(
         "--anim-framerate",
         [&](std::vector<std::string> choices) -> bool {
           for (const std::string choice : choices) {
             if (choice == "bake24") {
               gltfOptions.animationFramerate = AnimationFramerateOptions::BAKE24;
             } else if (choice == "bake30") {
               gltfOptions.animationFramerate = AnimationFramerateOptions::BAKE30;
             } else if (choice == "bake60") {
               gltfOptions.animationFramerate = AnimationFramerateOptions::BAKE60;
             } else {
               fmt::printf("Unknown --anim-framerate: %s\n", choice);
               throw CLI::RuntimeError(1);
             }
           }
           return true;
         },
         "Select baked animation framerate.")
      ->type_name("(bake24|bake30|bake60)");

  const auto opt_flip_u = app.add_flag("--flip-u", "Flip all U texture coordinates.");
  const auto opt_no_flip_u = app.add_flag("--no-flip-u", "Don't flip U texture coordinates.");
  const auto opt_flip_v = app.add_flag("--flip-v", "Flip all V texture coordinates.");
  const auto opt_no_flip_v = app.add_flag("--no-flip-v", "Don't flip V texture coordinates.");

  app.add_flag(
         "--pbr-metallic-roughness",
         gltfOptions.usePBRMetRough,
         "Try to glean glTF 2.0 native PBR attributes from the FBX.")
      ->group("Materials");

  app.add_flag(
         "--khr-materials-unlit",
         gltfOptions.useKHRMatUnlit,
         "Use KHR_materials_unlit extension to request an unlit shader.")
      ->group("Materials");

  app.add_flag_function(
      "--no-khr-lights-punctual",
      [&](size_t count) { gltfOptions.useKHRLightsPunctual = (count == 0); },
      "Don't use KHR_lights_punctual extension to export FBX lights.");

  app.add_flag(
      "--user-properties",
      gltfOptions.enableUserProperties,
      "Transcribe FBX User Properties into glTF node and material 'extras'.");

  app.add_flag(
      "--blend-shape-no-sparse",
      gltfOptions.disableSparseBlendShapes,
      "Don't use sparse accessors to store blend shapes");

  app.add_flag(
      "--blend-shape-normals",
      gltfOptions.useBlendShapeNormals,
      "Include blend shape normals, if reported present by the FBX SDK.");

  app.add_flag(
      "--blend-shape-tangents",
      gltfOptions.useBlendShapeTangents,
      "Include blend shape tangents, if reported present by the FBX SDK.");

  app.add_option(
      "--normalize-weights",
      gltfOptions.normalizeSkinningWeights,
      "Normalize skinning weights.",
      true);

  app.add_option(
         "--skinning-weights",
         gltfOptions.maxSkinningWeights,
         "The number of joint influences per vertex.",
         true)
      ->check(CLI::Range(0, 512));

  app.add_option(
         "-k,--keep-attribute",
         [&](std::vector<std::string> attributes) -> bool {
           gltfOptions.keepAttribs =
               RAW_VERTEX_ATTRIBUTE_JOINT_INDICES | RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS;
           for (std::string attribute : attributes) {
             if (attribute == "position") {
               gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_POSITION;
             } else if (attribute == "normal") {
               gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_NORMAL;
             } else if (attribute == "tangent") {
               gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_TANGENT;
             } else if (attribute == "binormal") {
               gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_BINORMAL;
             } else if (attribute == "color") {
               gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_COLOR;
             } else if (attribute == "uv0") {
               gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_UV0;
             } else if (attribute == "uv1") {
               gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_UV1;
             } else if (attribute == "auto") {
               gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_AUTO;
             } else {
               fmt::printf("Unknown --keep-attribute option: %s\n", attribute);
               throw CLI::RuntimeError(1);
             }
           }
           return true;
         },
         "Used repeatedly to build a limiting set of vertex attributes to keep.")
      ->type_size(-1)
      ->type_name("(position|normal|tangent|binormial|color|uv0|uv1|auto)");

  app.add_flag(
         "-d,--draco", gltfOptions.draco.enabled, "Apply Draco mesh compression to geometries.")
      ->group("Draco");

  app.add_option(
         "--draco-compression-level",
         gltfOptions.draco.compressionLevel,
         "The compression level to tune Draco to.",
         true)
      ->check(CLI::Range(0, 10))
      ->group("Draco");

  app.add_option(
         "--draco-bits-for-position",
         gltfOptions.draco.quantBitsPosition,
         "How many bits to quantize position to.",
         true)
      ->check(CLI::Range(1, 32))
      ->group("Draco");

  app.add_option(
         "--draco-bits-for-uv",
         gltfOptions.draco.quantBitsTexCoord,
         "How many bits to quantize UV coordinates to.",
         true)
      ->check(CLI::Range(1, 32))
      ->group("Draco");

  app.add_option(
         "--draco-bits-for-normals",
         gltfOptions.draco.quantBitsNormal,
         "How many bits to quantize normals to.",
         true)
      ->check(CLI::Range(1, 32))
      ->group("Draco");

  app.add_option(
         "--draco-bits-for-colors",
         gltfOptions.draco.quantBitsColor,
         "How many bits to quantize colors to.",
         true)
      ->check(CLI::Range(1, 32))
      ->group("Draco");

  app.add_option(
         "--draco-bits-for-other",
         gltfOptions.draco.quantBitsGeneric,
         "How many bits to quantize all other vertex attributes to.",
         true)
      ->check(CLI::Range(1, 32))
      ->group("Draco");

  app.add_option(
         "--fbx-temp-dir", gltfOptions.fbxTempDir, "Temporary directory to be used by FBX SDK.")
      ->check(CLI::ExistingDirectory);

  CLI11_PARSE(app, argc, argv);

  bool do_flip_u = false;
  bool do_flip_v = true;
  // somewhat tedious way to resolve --flag vs --no-flag in order provided
  for (const auto opt : app.parse_order()) {
    do_flip_u = (do_flip_u || (opt == opt_flip_u)) && (opt != opt_no_flip_u);
    do_flip_v = (do_flip_v || (opt == opt_flip_v)) && (opt != opt_no_flip_v);
  }
  std::vector<std::function<Vec2f(Vec2f)>> texturesTransforms;
  if (do_flip_u || do_flip_v) {
    if (do_flip_u && do_flip_v) {
      texturesTransforms.emplace_back([](Vec2f uv) { return Vec2f(1.0 - uv[0], 1.0 - uv[1]); });
    } else if (do_flip_u) {
      texturesTransforms.emplace_back([](Vec2f uv) { return Vec2f(1.0 - uv[0], uv[1]); });
    } else {
      texturesTransforms.emplace_back([](Vec2f uv) { return Vec2f(uv[0], 1.0 - uv[1]); });
    }
  }
  if (verboseOutput) {
    if (do_flip_u) {
      fmt::printf("Flipping texture coordinates in the 'U' dimension.\n");
    }
    if (!do_flip_v) {
      fmt::printf("NOT flipping texture coordinates in the 'V' dimension.\n");
    }
  }

  if (inputPath.empty()) {
    fmt::printf("You must supply a FBX file to convert.\n");
    exit(1);
  }

  if (!gltfOptions.useKHRMatUnlit && !gltfOptions.usePBRMetRough) {
    if (verboseOutput) {
      fmt::printf("Defaulting to --pbr-metallic-roughness material support.\n");
    }
    gltfOptions.usePBRMetRough = true;
  }

  if (gltfOptions.embedResources && gltfOptions.outputBinary) {
    fmt::printf("Note: Ignoring --embed; it's meaningless with --binary.\n");
  }

  if (outputPath.empty()) {
    // if -o is not given, default to the basename of the .fbx
    outputPath = "./" + FileUtils::GetFileBase(inputPath);
  }
  // the output folder in .gltf mode, not used for .glb
  std::string outputFolder;

  // the path of the actual .glb or .gltf file
  std::string modelPath;
  const auto& suffix = FileUtils::GetFileSuffix(outputPath);

  // Assume binary output if extension is glb
  if (suffix.has_value() && suffix.value() == "glb") {
    gltfOptions.outputBinary = true;
  }

  if (gltfOptions.outputBinary) {
    // add .glb to output path, unless it already ends in exactly that
    if (suffix.has_value() && suffix.value() == "glb") {
      modelPath = outputPath;
    } else {
      modelPath = outputPath + ".glb";
    }
    // if the extension is gltf set the output folder to the parent directory
  } else if (suffix.has_value() && suffix.value() == "gltf") {
    outputFolder = FileUtils::getFolder(outputPath) + "/";
    modelPath = outputPath;
  } else {
    // in gltf mode, we create a folder and write into that
    outputFolder = fmt::format("{}_out/", outputPath.c_str());
    modelPath = outputFolder + FileUtils::GetFileName(outputPath) + ".gltf";
  }
  if (!FileUtils::CreatePath(modelPath.c_str())) {
    fmt::fprintf(stderr, "ERROR: Failed to create folder: %s'\n", outputFolder.c_str());
    return 1;
  }

  ModelData* data_render_model = nullptr;
  RawModel raw;

  if (verboseOutput) {
    fmt::printf("Loading FBX File: %s\n", inputPath);
  }
  if (!LoadFBXFile(raw, inputPath, {"png", "jpg", "jpeg"}, gltfOptions)) {
    fmt::fprintf(stderr, "ERROR:: Failed to parse FBX: %s\n", inputPath);
    return 1;
  }

  if (!texturesTransforms.empty()) {
    raw.TransformTextures(texturesTransforms);
  }
  raw.Condense(gltfOptions.maxSkinningWeights, gltfOptions.normalizeSkinningWeights);
  raw.TransformGeometry(gltfOptions.computeNormals);

  std::ofstream outStream; // note: auto-flushes in destructor
  const auto streamStart = outStream.tellp();

  outStream.open(modelPath, std::ios::trunc | std::ios::ate | std::ios::out | std::ios::binary);
  if (outStream.fail()) {
    fmt::fprintf(stderr, "ERROR:: Couldn't open file for writing: %s\n", modelPath.c_str());
    return 1;
  }
  data_render_model = Raw2Gltf(outStream, outputFolder, raw, gltfOptions);

  if (gltfOptions.outputBinary) {
    fmt::printf(
        "Wrote %lu bytes of binary glTF to %s.\n",
        (unsigned long)(outStream.tellp() - streamStart),
        modelPath);
    delete data_render_model;
    return 0;
  }

  fmt::printf(
      "Wrote %lu bytes of glTF to %s.\n",
      (unsigned long)(outStream.tellp() - streamStart),
      modelPath);

  if (gltfOptions.embedResources) {
    // we're done: everything was inlined into the glTF JSON
    delete data_render_model;
    return 0;
  }

  assert(!outputFolder.empty());

  const std::string binaryPath = outputFolder + extBufferFilename;
  FILE* fp = fopen(binaryPath.c_str(), "wb");
  if (fp == nullptr) {
    fmt::fprintf(stderr, "ERROR:: Couldn't open file '%s' for writing.\n", binaryPath);
    return 1;
  }

  if (data_render_model->binary->empty() == false) {
    const unsigned char* binaryData = &(*data_render_model->binary)[0];
    unsigned long binarySize = data_render_model->binary->size();
    if (fwrite(binaryData, binarySize, 1, fp) != 1) {
      fmt::fprintf(
          stderr, "ERROR: Failed to write %lu bytes to file '%s'.\n", binarySize, binaryPath);
      fclose(fp);
      return 1;
    }
    fclose(fp);
    fmt::printf("Wrote %lu bytes of binary data to %s.\n", binarySize, binaryPath);
  }

  delete data_render_model;
  return 0;
}
