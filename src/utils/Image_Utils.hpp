/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace ImageUtils {

enum ImageOcclusion { IMAGE_OPAQUE, IMAGE_TRANSPARENT };

struct ImageProperties {
  int width;
  int height;
  ImageOcclusion occlusion;
};

ImageProperties GetImageProperties(char const* filePath);

/**
 * Very simple method for mapping filename suffix to mime type. The glTF 2.0 spec only accepts
 * values "image/jpeg" and "image/png" so we don't need to get too fancy.
 */
std::string suffixToMimeType(std::string suffix);

} // namespace ImageUtils
