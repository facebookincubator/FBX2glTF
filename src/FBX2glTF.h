/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __FBX2GLTF_H__
#define __FBX2GLTF_H__

#if defined ( _WIN32 )
// This can be a macro under Windows, confusing FMT
#undef isnan
// Tell Windows not to define min() and max() macros
#define NOMINMAX
#include <Windows.h>
#endif

#include <fmt/printf.h>
#include <fbxsdk.h>

#include "mathfu.h"

#endif // !__FBX2GLTF_H__
