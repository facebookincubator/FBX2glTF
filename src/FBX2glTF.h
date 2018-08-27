/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <string>

#if defined ( _WIN32 )
// Tell Windows not to define min() and max() macros
#define NOMINMAX
#include <Windows.h>
#endif

#define FBX2GLTF_VERSION std::string("0.9.5")

#include <fmt/printf.h>
#include <fbxsdk.h>

#if defined ( _WIN32 )
// this is defined in fbxmath.h
#undef isnan
#endif

#include "mathfu.hpp"

extern bool verboseOutput;
