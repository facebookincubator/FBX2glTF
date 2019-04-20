/****************************************************************************************

Copyright (C) 2015 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _DRAW_TEXT_H
#define _DRAW_TEXT_H

#include "GlFunctions.h"

// Utility classes for display text in OpenGL.

class DrawText
{
public:
    DrawText();
    ~DrawText();

    // Set the size of glyphs.
    void SetPointSize(float pPointSize) { mPointSize = pPointSize; }

    // Set the extra horizontal size between two consecutive glyphs.
    void SetGap(float pGap) { mGap = pGap; }

    // Display a string (ASCII only).
    void Display(const char * pText);

private:
    // Initialize with pre-generated texture, containing glyph coordinates and bitmaps.
    void Initialize();

    struct Glyph
    {
        float advance;          // horizontal distance from the origin of this glyph to next origin.
        float texture_left;     // texture coordinates of this glyph, range in [0, 1]
        float texture_right;
        float texture_bottom;
        float texture_top;
        float vertex_left;      // vertex coordinates of this glyph
        float vertex_right;     // range almost in [0, 1], except some glyph with descend like 'g' or 'p'
        float vertex_bottom;
        float vertex_top;
    };

    Glyph * mGlyph;
    GLuint mTextureName;

    float mPointSize;
    float mGap;
};

#endif // _DRAW_TEXT_H

