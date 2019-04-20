/****************************************************************************************

Copyright (C) 2015 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _SCENE_CACHE_H
#define _SCENE_CACHE_H

#include "GlFunctions.h"

// Save mesh vertices, normals, UVs and indices in GPU with OpenGL Vertex Buffer Objects
class VBOMesh
{
public:
    VBOMesh();
    ~VBOMesh();

    // Save up data into GPU buffers.
    bool Initialize(const FbxMesh * pMesh);

    // Update vertex positions for deformed meshes.
    void UpdateVertexPosition(const FbxMesh * pMesh, const FbxVector4 * pVertices) const;

    // Bind buffers, set vertex arrays, turn on lighting and texture.
    void BeginDraw(ShadingMode pShadingMode) const;
    // Draw all the faces with specific material with given shading mode.
    void Draw(int pMaterialIndex, ShadingMode pShadingMode) const;
    // Unbind buffers, reset vertex arrays, turn off lighting and texture.
    void EndDraw() const;

    // Get the count of material groups
    int GetSubMeshCount() const { return mSubMeshes.GetCount(); }

private:
    enum
    {
        VERTEX_VBO,
        NORMAL_VBO,
        UV_VBO,
        INDEX_VBO,
        VBO_COUNT,
    };

    // For every material, record the offsets in every VBO and triangle counts
    struct SubMesh
    {
        SubMesh() : IndexOffset(0), TriangleCount(0) {}

        int IndexOffset;
        int TriangleCount;
    };

    GLuint mVBONames[VBO_COUNT];
    FbxArray<SubMesh*> mSubMeshes;
    bool mHasNormal;
    bool mHasUV;
    bool mAllByControlPoint; // Save data in VBO by control point or by polygon vertex.
};

// Cache for FBX material
class MaterialCache
{
public:
    MaterialCache();
    ~MaterialCache();

    bool Initialize(const FbxSurfaceMaterial * pMaterial);
    
    // Set material colors and binding diffuse texture if exists.
    void SetCurrentMaterial() const;

    bool HasTexture() const { return mDiffuse.mTextureName != 0; }

    // Set default green color.
    static void SetDefaultMaterial();

private:
    struct ColorChannel
    {
        ColorChannel() : mTextureName(0)
        {
            mColor[0] = 0.0f;
            mColor[1] = 0.0f;
            mColor[2] = 0.0f;
            mColor[3] = 1.0f;
        }

        GLuint mTextureName;
        GLfloat mColor[4];
    };
    ColorChannel mEmissive;
    ColorChannel mAmbient;
    ColorChannel mDiffuse;
    ColorChannel mSpecular;
    GLfloat mShinness;
};

// Property cache, value and animation curve.
struct PropertyChannel
{
    PropertyChannel() : mAnimCurve(NULL), mValue(0.0f) {}
    // Query the channel value at specific time.
    GLfloat Get(const FbxTime & pTime) const
    {
        if (mAnimCurve)
        {
            return mAnimCurve->Evaluate(pTime);
        }
        else
        {
            return mValue;
        }
    }

    FbxAnimCurve * mAnimCurve;
    GLfloat mValue;
};

// Cache for FBX lights
class LightCache
{
public:
    LightCache();
    ~LightCache();

    // Set ambient light. Turn on light0 and set its attributes to default (white directional light in Z axis).
    // If the scene contains at least one light, the attributes of light0 will be overridden.
    static void IntializeEnvironment(const FbxColor & pAmbientLight);

    bool Initialize(const FbxLight * pLight, FbxAnimLayer * pAnimLayer);

    // Draw a geometry (sphere for point and directional light, cone for spot light).
    // And set light attributes.
    void SetLight(const FbxTime & pTime) const;

private:
    static int sLightCount;         // How many lights in this scene.

    GLuint mLightIndex;
    FbxLight::EType mType;
    PropertyChannel mColorRed;
    PropertyChannel mColorGreen;
    PropertyChannel mColorBlue;
    PropertyChannel mConeAngle;
};

#endif // _SCENE_CACHE_H
