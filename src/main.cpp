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
#include "utils/String_Utils.h"
#include "utils/File_Utils.h"
#include "Fbx2Raw.h"
#include "RawModel.h"
#include "Raw2Gltf.h"

bool verboseOutput = false;

int main(int argc, char *argv[])
{
    cxxopts::Options options(
        "FBX2glTF",
        "FBX2glTF 2.0: Generate a glTF 2.0 representation of an FBX model.");

    std::string inputPath;
    std::string outputPath;

    std::vector<std::function<Vec2f(Vec2f)>> texturesTransforms;

    GltfOptions gltfOptions{
        -1,            // keepAttribs
        false,         // outputBinary
        false,         // embedResources
        false,         // useDraco
        false,         // useKHRMatCom
        false,         // useKHRMatUnlit
        false,         // usePBRMetRough
        false,         // usePBRSpecGloss
        false,         // useBlendShapeNormals
        false,         // useBlendShapeTangents
    };

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
                   "d,draco", "Apply Draco mesh compression to geometries.",
                   cxxopts::value<bool>(gltfOptions.useDraco))
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
                   "khr-materials-common", "(WIP) Use KHR_materials_common extensions to specify Lambert/Blinn/Phong shaders.",
                   cxxopts::value<bool>(gltfOptions.useKHRMatCom))
               (
                   "pbr-specular-glossiness", "(WIP) Experimentally fill in the KHR_materials_pbrSpecularGlossiness extension.",
                   cxxopts::value<bool>(gltfOptions.usePBRSpecGloss))
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
        fmt::printf(
            R"(
FBX2glTF version 2.0
Copyright (c) 2016-2017 Oculus VR, LLC.
)");
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

    if (!gltfOptions.useKHRMatUnlit && !gltfOptions.useKHRMatCom && !gltfOptions.usePBRSpecGloss && !gltfOptions.usePBRMetRough) {
        if (verboseOutput) {
            fmt::printf("Defaulting to --pbr-metallic-roughness material support.\n");
        }
        gltfOptions.usePBRMetRough = true;
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

    if (options.count("keep-attribute") > 0) {
        gltfOptions.keepAttribs = RAW_VERTEX_ATTRIBUTE_JOINT_INDICES | RAW_VERTEX_ATTRIBUTE_JOINT_WEIGHTS;
        for (const auto &attribute : options["keep-attribute"].as<std::vector<std::string>>()) {
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
        outputPath = "./" + Gltf::StringUtils::GetFileBaseString(inputPath);
    }
    std::string outputFolder; // the output folder in .gltf mode, not used for .glb
    std::string modelPath; // the path of the actual .glb or .gltf file
    if (gltfOptions.outputBinary) {
        // in binary mode, we write precisely where we're asked
        modelPath = outputPath + ".glb";

    } else {
        // in gltf mode, we create a folder and write into that
        outputFolder = outputPath + "_out/";
        modelPath = outputFolder + Gltf::StringUtils::GetFileNameString(outputPath) + ".gltf";
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

    const unsigned char *binaryData = &(*data_render_model->binary)[0];
    unsigned long       binarySize  = data_render_model->binary->size();
    if (fwrite(binaryData, binarySize, 1, fp) != 1) {
        fmt::fprintf(stderr, "ERROR: Failed to write %lu bytes to file '%s'.\n", binarySize, binaryPath);
        fclose(fp);
        return 1;
    }
    fclose(fp);
    fmt::printf("Wrote %lu bytes of binary data to %s.\n", binarySize, binaryPath);

    delete data_render_model;
    return 0;
}
