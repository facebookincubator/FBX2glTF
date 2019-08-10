/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <climits>
#include <string>

#if defined(_WIN32)
// Tell Windows not to define min() and max() macros
#define NOMINMAX
#include <Windows.h>
#endif

#define FBX2GLTF_VERSION std::string("0.9.7")

#include <fmt/printf.h>

#include <fbxsdk.h>

#if defined(_WIN32)
// this is defined in fbxmath.h
#undef isnan
#undef snprintf
#endif

#include "mathfu.hpp"

// give all modules access to our tweaked JSON
#include <fifo_map.hpp>
#include <json.hpp>

template <class K, class V, class ignore, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;

using json = nlohmann::basic_json<workaround_fifo_map>;

extern bool verboseOutput;

/**
 * Centralises all the laborious downcasting from your OS' 64-bit
 * index variables down to the uint32s that glTF is built out of.
 */
inline uint32_t to_uint32(size_t n) {
  assert(n < UINT_MAX);
  return static_cast<uint32_t>(n);
}

/**
 * The variuos situations in which the user may wish for us to (re-)compute normals for our
 * vertices.
 */
enum class ComputeNormalsOption {
  NEVER, // do not ever compute any normals (results in broken glTF for some sources)
  BROKEN, // replace zero-length normals in any mesh that has a normal layer
  MISSING, // if a mesh lacks normals, compute them all
  ALWAYS // compute a new normal for every vertex, obliterating whatever may have been there before
};

enum class UseLongIndicesOptions {
  NEVER, // only ever use 16-bit indices
  AUTO, // use shorts or longs depending on vertex count
  ALWAYS, // only ever use 32-bit indices
};

enum class AnimationFramerateOptions {
  BAKE24, // bake animations at 24 fps
  BAKE30, // bake animations at 30 fps
  BAKE60, // bake animations at 60 fps
};

/**
 * User-supplied options that dictate the nature of the glTF being generated.
 */
struct GltfOptions {
  /**
   * If negative, disabled. Otherwise, a bitfield of RawVertexAttributes that
   * specify the largest set of attributes that'll ever be kept for a vertex.
   * The special bit RAW_VERTEX_ATTRIBUTE_AUTO triggers smart mode, where the
   * attributes to keep are inferred from which textures are supplied.
   */
  int keepAttribs{-1};
  /** Whether to output a .glb file, the binary format of glTF. */
  bool outputBinary{false};
  /** If non-binary, whether to inline all resources, for a single (large) .glTF file. */
  bool embedResources{false};

  /** Whether and how to use KHR_draco_mesh_compression to minimize static geometry size. */
  struct {
    bool enabled = false;
    int compressionLevel = 7;
    int quantBitsPosition = 14;
    int quantBitsTexCoord = 10;
    int quantBitsNormal = 10;
    int quantBitsColor = 8;
    int quantBitsGeneric = 8;
  } draco;

  /** Whether to include FBX User Properties as 'extras' metadata in glTF nodes. */
  bool enableUserProperties{false};

  /** Whether to use KHR_materials_unlit to extend materials definitions. */
  bool useKHRMatUnlit{false};
  /** Whether to populate the pbrMetallicRoughness substruct in materials. */
  bool usePBRMetRough{false};

  /** Whether to include lights through the KHR_punctual_lights extension. */
  bool useKHRLightsPunctual{true};

  /** Whether to include blend shape normals, if present according to the SDK. */
  bool useBlendShapeNormals{false};
  /** Whether to include blend shape tangents, if present according to the SDK. */
  bool useBlendShapeTangents{false};
  /** When to compute vertex normals from geometry. */
  ComputeNormalsOption computeNormals = ComputeNormalsOption::BROKEN;
  /** When to use 32-bit indices. */
  UseLongIndicesOptions useLongIndices = UseLongIndicesOptions::AUTO;
  /** Select baked animation framerate. */
  AnimationFramerateOptions animationFramerate = AnimationFramerateOptions::BAKE24;

  /** Temporary directory used by FBX SDK. */
  std::string fbxTempDir;
};
