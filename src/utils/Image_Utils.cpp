/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <stdint.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

#include "File_Utils.h"
#include "Image_Utils.h"

// https://www.w3.org/TR/PNG/#11IHDR
const int PNG_IHDR_CHUNK_START = 8;
const int PNG_HEADER_SIZE      = PNG_IHDR_CHUNK_START + 8 + 13;

enum PNG_ColorType
{
    Grayscale      = 0,
    RGB            = 2,
    Palette        = 3,
    GrayscaleAlpha = 4,
    RGBA           = 6
};

static bool ImageIsPNG(char const *fileName, unsigned char const *buffer)
{
    if ((buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4E && buffer[3] == 0x47)) {
        // first chunk must be an IHDR
        return (
            buffer[PNG_IHDR_CHUNK_START + 4] == 'I' &&
            buffer[PNG_IHDR_CHUNK_START + 5] == 'H' &&
            buffer[PNG_IHDR_CHUNK_START + 6] == 'D' &&
            buffer[PNG_IHDR_CHUNK_START + 7] == 'R');
    }
    return false;
}

static ImageProperties PNGProperties(unsigned char const *buffer)
{
    // Extract (big-endian) properties from the PNG IHDR
    ImageProperties properties;
    properties.width  =
        (buffer[PNG_IHDR_CHUNK_START + 8] & 0xFF) << 24 | (buffer[PNG_IHDR_CHUNK_START + 9] & 0xFF) << 16 |
        (buffer[PNG_IHDR_CHUNK_START + 10] & 0xFF) << 8 | (buffer[PNG_IHDR_CHUNK_START + 11] & 0xFF);
    properties.height =
        (buffer[PNG_IHDR_CHUNK_START + 12] & 0xFF) << 24 | (buffer[PNG_IHDR_CHUNK_START + 13] & 0xFF) << 16 |
        (buffer[PNG_IHDR_CHUNK_START + 14] & 0xFF) << 8 | (buffer[PNG_IHDR_CHUNK_START + 15] & 0xFF);
    properties.occlusion = (buffer[PNG_IHDR_CHUNK_START + 17] == RGBA) ? IMAGE_TRANSPARENT : IMAGE_OPAQUE;

    return properties;
}

// header is broken into multiple structs because TGA headers are packed
struct TGA_HeaderStart_t
{
    char IDLength;
    char ColorMapType;
    char DataTypeCode;
};

struct TGA_HeaderColor_t
{
    short int ColorMapOrigin;
    short int ColorMapLength;
    char      ColorMapDepth;
};

struct TGA_HeaderOrigin_t
{
    short int XOrigin;
    short int YOrigin;
    short     Width;
    short     Height;
    char      BitsPerPixel;
    char      ImageDescriptor;
};

const int TGA_HEADER_SIZE = 8 + sizeof(TGA_HeaderOrigin_t);

static bool ImageIsTGA(char const *fileName, unsigned char const *buffer)
{
    // TGA's have pretty ambiguous header so we simply check their file extension
    size_t len = strlen(fileName);
    if (len < 4) {
        return false;
    }
#if defined( __unix__ ) || defined( __APPLE__ )
    return strcasecmp(fileName + len - 4, ".tga") == 0;
#else
    return _stricmp( fileName + len - 4, ".tga" ) == 0;
#endif
}

static ImageProperties TGAProperties(unsigned char const *buffer)
{
    const TGA_HeaderOrigin_t *header = reinterpret_cast< const TGA_HeaderOrigin_t * >( &(buffer[8]));

    ImageProperties properties;
    properties.width     = header->Width;
    properties.height    = header->Height;
    properties.occlusion = (header->BitsPerPixel == 32) ? IMAGE_TRANSPARENT : IMAGE_OPAQUE;

    return properties;
}

ImageProperties GetImageProperties(char const *filePath)
{
    ImageProperties defaultProperties;
    defaultProperties.width     = 1;
    defaultProperties.height    = 1;
    defaultProperties.occlusion = IMAGE_OPAQUE;

    FILE *f = fopen(filePath, "rb");
    if (f == nullptr) {
        return defaultProperties;
    }

    // This assumes every image file is at least as large as the largest header.
    const int maxHeaderSize = std::max(PNG_HEADER_SIZE, TGA_HEADER_SIZE);
    unsigned char buffer[maxHeaderSize];
    if (fread(buffer, (size_t) maxHeaderSize, 1, f) == 1) {
        if (ImageIsPNG(filePath, buffer)) {
            return PNGProperties(buffer);
        } else if (ImageIsTGA(filePath, buffer)) {
            return TGAProperties(buffer);
        }
    }
    return defaultProperties;
}
