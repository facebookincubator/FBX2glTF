/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <fstream>

#if defined( __unix__ ) || defined( __APPLE__ )

#include <sys/stat.h>

#define _stricmp strcasecmp
#endif

#include <cxxopts.hpp>

#include "FBX2glTF.h"
#include "utils/String_Utils.hpp"
#include "utils/File_Utils.hpp"
#include "fbx/Fbx2Raw.hpp"
#include "gltf/Raw2Gltf.hpp"

bool verboseOutput = false;

int main(int argc, char *argv[])
{
    cxxopts::Options options(
        "FBX2glTF",
        fmt::sprintf("FBX2glTF %s: Generate a glTF 2.0 representation of an FBX model.", FBX2GLTF_VERSION)
    );

    std::string inputPath;
    std::string outputPath;

    std::vector<std::function<Vec2f(Vec2f)>> texturesTransforms;

    GltfOptions gltfOptions;

    options.positional_help("[<FBX File>]");
    options.add_options()
               (
                   "i,input", "The FBX model to convert.",
                   cxxopts::value<std::string>(inputPath))
               (
                   "o,output", "Where to generate the output, without suffix.",
                   cxxopts::value<std::string>(outputPath))
               (
                   "e,embed", "Inline buffers as data:// URIs within generated non-binary glTF.",
                   cxxopts::value<bool>(gltfOptions.embedResources))
               (
                   "b,binary", "Output a single binary format .glb file.",
                   cxxopts::value<bool>(gltfOptions.outputBinary))
               (
                   "long-indices", "Whether to use 32-bit indices (never|auto|always).",
                   cxxopts::value<std::vector<std::string>>())
               (
                   "d,draco", "Apply Draco mesh compression to geometries.",
                   cxxopts::value<bool>(gltfOptions.draco.enabled))
               (
                   "draco-compression-level", "The compression level to tune Draco to, from 0 to 10. (default: 7)",
                   cxxopts::value<int>(gltfOptions.draco.compressionLevel))
               (
                   "draco-bits-for-positions", "How many bits to quantize position to. (default: 14)",
                   cxxopts::value<int>(gltfOptions.draco.quantBitsPosition))
               (
                   "draco-bits-for-uv", "How many bits to quantize UV coordinates to. (default: 10)",
                   cxxopts::value<int>(gltfOptions.draco.quantBitsTexCoord))
               (
                   "draco-bits-for-normals", "How many bits to quantize normals to. (default: 10)",
                   cxxopts::value<int>(gltfOptions.draco.quantBitsNormal))
               (
                   "draco-bits-for-colors", "How many bits to quantize color to. (default: 8)",
                   cxxopts::value<int>(gltfOptions.draco.quantBitsColor))
               (
                   "draco-bits-for-other", "How many bits to quantize other vertex attributes to to. (default: 8)",
                   cxxopts::value<int>(gltfOptions.draco.quantBitsGeneric))
               (
                   "compute-normals", "When to compute normals for vertices (never|broken|missing|always).",
                   cxxopts::value<std::vector<std::string>>())
               ("flip-u", "Flip all U texture coordinates.")
               ("flip-v", "Flip all V texture coordinates (default behaviour!)")
               ("no-flip-v", "Suppress the default flipping of V texture coordinates")
               (
                   "pbr-metallic-roughness", "Try to glean glTF 2.0 native PBR attributes from the FBX.",
                   cxxopts::value<bool>(gltfOptions.usePBRMetRough))
               (
                   "khr-materials-unlit", "Use KHR_materials_unlit extension to specify Unlit shader.",
                   cxxopts::value<bool>(gltfOptions.useKHRMatUnlit))
               (
                   "no-khr-punctual-lights", "Don't use KHR_punctual_lights extension to export lights.",
                   cxxopts::value<bool>(gltfOptions.useKHRPunctualLights))
               (
                   "user-properties", "Transcribe FBX User Properties into glTF node and material 'extras'.",
                   cxxopts::value<bool>(gltfOptions.enableUserProperties))
               (
                   "blend-shape-normals", "Include blend shape normals, if reported present by the FBX SDK.",
                   cxxopts::value<bool>(gltfOptions.useBlendShapeNormals))
               (
                   "blend-shape-tangents", "Include blend shape tangents, if reported present by the FBX SDK.",
                   cxxopts::value<bool>(gltfOptions.useBlendShapeTangents))
               (
                   "k,keep-attribute", "Used repeatedly to build a limiting set of vertex attributes to keep.",
                   cxxopts::value<std::vector<std::string>>())
               ("v,verbose", "Enable verbose output.")
               ("h,help", "Show this help.")
               ("V,version", "Display the current program version.");

    try {
        options.parse_positional("input");
        options.parse(argc, argv);

    } catch (const cxxopts::OptionException &e) {
        fmt::printf(options.help());
        return 1;
    }

    if (options.count("version")) {
        fmt::printf("FBX2glTF version %s\nCopyright (c) 2016-2018 Oculus VR, LLC.\n", FBX2GLTF_VERSION);
        return 0;
    }

    if (options.count("help")) {
        fmt::printf(options.help());
        return 0;
    }

    if (!options.count("input")) {
        fmt::printf("You must supply a FBX file to convert.\n");
        fmt::printf(options.help());
        return 1;
    }

    if (options.count("verbose")) {
        verboseOutput = true;
    }

    if (!gltfOptions.useKHRMatUnlit && !gltfOptions.usePBRMetRough) {
        if (verboseOutput) {
            fmt::printf("Defaulting to --pbr-metallic-roughness material support.\n");
        }
        gltfOptions.usePBRMetRough = true;
    }

    if (gltfOptions.draco.compressionLevel != -1 &&
        (gltfOptions.draco.compressionLevel < 1 || gltfOptions.draco.compressionLevel > 10)) {
        fmt::printf("Draco compression level must lie in [1, 10].\n");
        return 0;
    }

    if (options.count("flip-u") > 0) {
        texturesTransforms.emplace_back([](Vec2f uv) { return Vec2f(1.0f - uv[0], uv[1]); });
    }
    if (options.count("flip-v") > 0) {
        fmt::printf("Note: The --flip-v command switch is now default behaviour.\n");
    }
    if (options.count("no-flip-v") == 0) {
        texturesTransforms.emplace_back([](Vec2f uv) { return Vec2f(uv[0], 1.0f - uv[1]); });
    } else if (verboseOutput) {
        fmt::printf("Suppressing --flip-v transformation of texture coordinates.\n");
    }

    for (const std::string &choice : options["long-indices"].as<std::vector<std::string>>()) {
        if (choice == "never") {
            gltfOptions.useLongIndices = UseLongIndicesOptions::NEVER;
        } else if (choice == "auto") {
            gltfOptions.useLongIndices = UseLongIndicesOptions::AUTO;
        } else if (choice == "always") {
            gltfOptions.useLongIndices = UseLongIndicesOptions::ALWAYS;
        } else {
            fmt::printf("Unknown --long-indices: %s\n", choice);
            fmt::printf(options.help());
            return 1;
        }
    }

    if (options.count("compute-normals") > 0) {
        for (const std::string &choice : options["compute-normals"].as<std::vector<std::string>>()) {
            if (choice == "never") {
                gltfOptions.computeNormals = ComputeNormalsOption::NEVER;
            } else if (choice == "broken") {
                gltfOptions.computeNormals = ComputeNormalsOption::BROKEN;
            } else if (choice == "missing") {
                gltfOptions.computeNormals = ComputeNormalsOption::MISSING;
            } else if (choice == "always") {
                gltfOptions.computeNormals = ComputeNormalsOption::ALWAYS;
            } else {
                fmt::printf("Unknown --compute-normals: %s\n", choice);
                fmt::printf(options.help());
                return 1;
            }
        }
    }

    if (options.count("keep-attribute") > 0) {
        gltfOptions.keepAttribs = RAW_VERTEX_ATTRIBUTE_JOINT_INDICES | RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS;
        for (std::string attribute : options["keep-attribute"].as<std::vector<std::string>>()) {
            if (attribute == "position") { gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_POSITION; }
            else if (attribute == "normal") { gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_NORMAL; }
            else if (attribute == "tangent") { gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_TANGENT; }
            else if (attribute == "binormal") { gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_BINORMAL; }
            else if (attribute == "color") { gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_COLOR; }
            else if (attribute == "uv0") { gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_UV0; }
            else if (attribute == "uv1") { gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_UV1; }
            else if (attribute == "auto") { gltfOptions.keepAttribs |= RAW_VERTEX_ATTRIBUTE_AUTO; }
            else {
                fmt::printf("Unknown --keep-attribute: %s\n", attribute);
                fmt::printf("Valid choices are: position, normal, tangent, binormial, color, uv0, uv1, auto,\n");
                return 1;
            }
        }
    }

    if (gltfOptions.embedResources && gltfOptions.outputBinary) {
        fmt::printf("Note: Ignoring --embed; it's meaningless with --binary.\n");
    }

    if (options.count("output") == 0) {
        // if -o is not given, default to the basename of the .fbx
        outputPath = fmt::format(".{}{}", (const char)StringUtils::GetPathSeparator(), StringUtils::GetFileBaseString(inputPath));

        fmt::printf("outputPath = %s\n", outputPath);
    }
    std::string outputFolder; // the output folder in .gltf mode, not used for .glb
    std::string modelPath; // the path of the actual .glb or .gltf file
    if (gltfOptions.outputBinary) {
        // in binary mode, we write precisely where we're asked
        modelPath = outputPath + ".glb";

    } else {
        // in gltf mode, we create a folder and write into that
        outputFolder = fmt::format("{}_out{}", outputPath.c_str(), (const char)StringUtils::GetPathSeparator());
        modelPath = outputFolder + StringUtils::GetFileNameString(outputPath) + ".gltf";
    }
    if (!FileUtils::CreatePath(modelPath.c_str())) {
        fmt::fprintf(stderr, "ERROR: Failed to create folder: %s'\n", outputFolder.c_str());
        return 1;
    }

    ModelData *data_render_model = nullptr;
    RawModel  raw;

    if (verboseOutput) {
        fmt::printf("Loading FBX File: %s\n", inputPath);
    }
    if (!LoadFBXFile(raw, inputPath.c_str(), "png;jpg;jpeg")) {
        fmt::fprintf(stderr, "ERROR:: Failed to parse FBX: %s\n", inputPath);
        return 1;
    }

    if (!texturesTransforms.empty()) {
        raw.TransformTextures(texturesTransforms);
    }
    raw.Condense();
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
            (unsigned long) (outStream.tellp() - streamStart), modelPath);
        delete data_render_model;
        return 0;
    }

    fmt::printf(
        "Wrote %lu bytes of glTF to %s.\n",
        (unsigned long) (outStream.tellp() - streamStart), modelPath);

    if (gltfOptions.embedResources) {
        // we're done: everything was inlined into the glTF JSON
        delete data_render_model;
        return 0;
    }

    assert(!outputFolder.empty());

    const std::string binaryPath = outputFolder + extBufferFilename;
    FILE *fp = fopen(binaryPath.c_str(), "wb");
    if (fp == nullptr) {
        fmt::fprintf(stderr, "ERROR:: Couldn't open file '%s' for writing.\n", binaryPath);
        return 1;
    }

    if (data_render_model->binary->empty() == false)
    {
        const unsigned char *binaryData = &(*data_render_model->binary)[0];
        unsigned long       binarySize  = data_render_model->binary->size();
        if (fwrite(binaryData, binarySize, 1, fp) != 1) {
            fmt::fprintf(stderr, "ERROR: Failed to write %lu bytes to file '%s'.\n", binarySize, binaryPath);
            fclose(fp);
            return 1;
        }
        fclose(fp);
        fmt::printf("Wrote %lu bytes of binary data to %s.\n", binarySize, binaryPath);
    }

    delete data_render_model;
    return 0;
}
