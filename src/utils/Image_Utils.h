/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __IMAGE_UTILS_H__
#define __IMAGE_UTILS_H__

enum ImageOcclusion
{
    IMAGE_OPAQUE,
    IMAGE_PERFORATED,
    IMAGE_TRANSPARENT
};

struct ImageProperties
{
    int            width     = 0;
    int            height    = 0;
    ImageOcclusion occlusion = IMAGE_OPAQUE;
};

ImageProperties GetImageProperties(char const *filePath);

#endif // !__IMAGE_UTILS_H__
