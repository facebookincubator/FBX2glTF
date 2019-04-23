/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The scene created in this example is two cubes. 
// One is linked to a CGFX shader. Another is linked to a HLSL shader.
//
// The example illustrates how to:
//        1) create a CGFX shader
//        2) create a HLSL shader
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"
#include "../Common/GeometryUtility.h"

#define SAMPLE_FILENAME "ExportShader.fbx"
#define CGFX_SHADERFILE "xfSkyBox.cgfx" 
#define HLSL_SHADERFILE "glass.fx"
#define TEXTURE_FILE    "Default_reflection.dds"


// Function prototypes.
bool CreateScene(FbxManager* pSdkManager, FbxScene* pScene);
FbxSurfaceMaterial* CreateCgfxShader(FbxScene* pScene);
FbxSurfaceMaterial* CreateHlslShader(FbxScene* pScene);
void CreateTableEntry(FbxBindingTable* pTable, FbxProperty& pProp);

int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene.
    lResult = CreateScene(lSdkManager, lScene);

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while creating the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return 0;
    }

    // Save the scene.

    // The example can take an output file name as an argument.
	const char* lSampleFileName = NULL;
	for( int i = 1; i < argc; ++i )
	{
		if( FBXSDK_stricmp(argv[i], "-test") == 0 ) continue;
		else if( !lSampleFileName ) lSampleFileName = argv[i];
	}
	if( !lSampleFileName ) lSampleFileName = SAMPLE_FILENAME;

    lResult = SaveScene(lSdkManager, lScene, lSampleFileName, -1, true);
    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return 0;
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

    return 0;
}


bool CreateScene(FbxManager *pSdkManager, FbxScene* pScene)
{
    // create scene info
    FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(pSdkManager,"SceneInfo");
    sceneInfo->mTitle = "Example scene";
    sceneInfo->mSubject = "Illustrates the creation of shader.";
    sceneInfo->mAuthor = "ExportShader sample program.";
    sceneInfo->mRevision = "rev. 1.0";
    sceneInfo->mKeywords = "shader";
    sceneInfo->mComment = "no particular comments required.";

    pScene->SetSceneInfo(sceneInfo);

    // Cube with CgFX shader
    FbxDouble3 lCgfxTranslation(0, 0, 0);
    FbxNode* lCgfxNode = CreateCube(pScene, "CgfxCube", lCgfxTranslation);

    FbxSurfaceMaterial* lCgfxMat = CreateCgfxShader(pScene);
    lCgfxNode->AddMaterial(lCgfxMat);

    // Cube with HLSL shader
    FbxDouble3 lHlslTranslation(20, 0, 0);
    FbxNode* lHlslNode = CreateCube(pScene, "HlslCube", lHlslTranslation);

    FbxSurfaceMaterial* lHlslMat = CreateHlslShader(pScene);
    lHlslNode->AddMaterial(lHlslMat);

    // Build the node tree.
    FbxNode* lRootNode = pScene->GetRootNode();
    lRootNode->AddChild(lCgfxNode);
    lRootNode->AddChild(lHlslNode);	

    return true;
}

FbxSurfaceMaterial* CreateCgfxShader(FbxScene* pScene)
{
    FbxSurfaceMaterial* lCgfxMat = FbxSurfaceMaterial::Create(pScene, "CgfxShader" );

    FbxImplementation* lImpl = FbxImplementation::Create(pScene, FbxString("Cgfx_Implementation"));

    lCgfxMat->AddImplementation( lImpl );
    lCgfxMat->SetDefaultImplementation( lImpl );
    lImpl->RenderAPI = FBXSDK_RENDERING_API_OPENGL;
    lImpl->RenderAPIVersion = "1.5"; 

    lImpl->Language = FBXSDK_SHADING_LANGUAGE_CGFX;
    lImpl->LanguageVersion = "1.5";

    FbxBindingTable* lTable = lTable = lImpl->AddNewTable("root", "shader");
    lImpl->RootBindingName = "root";

    // shader file
    lTable->DescAbsoluteURL = CGFX_SHADERFILE;
    // technique name
    lTable->DescTAG =  "user";

    FbxProperty lRootProp = FbxProperty::Create( lCgfxMat, FbxCompoundDT, "ShaderExample");
    // Property "SurfColor"
    FbxProperty lProp = FbxProperty::Create( lRootProp, FbxDouble3DT, "SurfColor" );
    FbxDouble3 lSurfVal(0, 1, 0);
    lProp.Set(lSurfVal);
    CreateTableEntry(lTable, lProp); // for every property, create a table entry in binding table
    
    // Property MasterAlpha
    lProp = FbxProperty::Create(lRootProp, FbxFloatDT, "MasterAlpha");
    lProp.Set(0.35);
    CreateTableEntry(lTable, lProp);

    // Property EnvSample, the property type is sample, so connect a texture to it
    lProp = FbxProperty::Create(lRootProp, FbxDouble3DT, "EnvSampler", "EnvSampler");
    FbxDouble3 lSampleVal(0, 0, 0);
    lProp.Set(lSampleVal);
    CreateTableEntry(lTable, lProp);

    // create texture and connect to property
    FbxFileTexture* lTexture = FbxFileTexture::Create(pScene, "EnvSamplerTex");
    lTexture->SetFileName(TEXTURE_FILE);
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);

    lTexture->ConnectDstProperty(lProp);
    return lCgfxMat;
}

FbxSurfaceMaterial* CreateHlslShader(FbxScene* pScene)
{
    FbxSurfaceMaterial* lHlslMat = FbxSurfaceMaterial::Create(pScene, "HlslShader" );

    FbxImplementation* lImpl = FbxImplementation::Create(pScene, FbxString("Hlsl_Implementation"));

    lHlslMat->AddImplementation( lImpl );
    lHlslMat->SetDefaultImplementation( lImpl );
    lImpl->RenderAPI = FBXSDK_RENDERING_API_DIRECTX;
    lImpl->RenderAPIVersion = "9.0"; 

    lImpl->Language = FBXSDK_SHADING_LANGUAGE_HLSL;
    lImpl->LanguageVersion = "1.0";

    FbxBindingTable* lTable = lTable = lImpl->AddNewTable("root", "shader");
    lImpl->RootBindingName = "root";

    // shader file
    lTable->DescAbsoluteURL = HLSL_SHADERFILE;
    // technique name
    lTable->DescTAG =  "dx9";

    
    // Property Script
    FbxProperty lProp = FbxProperty::Create(lHlslMat, FbxFloatDT, "Script", "Script"); 
    lProp.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    lProp.Set(0.8);
    CreateTableEntry(lTable, lProp);
        
    // Property reflectStrength
    lProp = FbxProperty::Create(lHlslMat, FbxFloatDT, "reflectStrength", "reflectStrength");
    lProp.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    lProp.Set(0.35);
    CreateTableEntry(lTable, lProp);

    // Property refractStrength
    lProp = FbxProperty::Create(lHlslMat, FbxFloatDT, "refractStrength", "refractStrength");
    lProp.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    lProp.Set(0.75);
    CreateTableEntry(lTable, lProp);

    // Property etas
    lProp = FbxProperty::Create(lHlslMat, FbxDouble3DT, "etas", "etas");
    lProp.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    FbxDouble3 lEtasVal(0.8, 0.6, 0.4);
    lProp.Set(lEtasVal);
    CreateTableEntry(lTable, lProp);

    // Property cubeMap, the property type is sample, not connect a texture to it, just skip it
    lProp = FbxProperty::Create(lHlslMat, FbxDouble3DT, "fresnelTex", "fresnelTex");
    lProp.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    FbxDouble3 lTexVal(0, 0, 0);
    lProp.Set(lTexVal);
    CreateTableEntry(lTable, lProp);

    // Property cubeMap, the property type is sample, so connect a texture to it
    lProp = FbxProperty::Create(lHlslMat, FbxDouble3DT, "cubeMap", "cubeMap");
    lProp.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    FbxDouble3 lMapVal(0, 1, 0);
    lProp.Set(lMapVal);
    CreateTableEntry(lTable, lProp);

    FbxFileTexture* lTexture = FbxFileTexture::Create(pScene, "cubeMapTex");
    lTexture->SetFileName(TEXTURE_FILE);
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);

    lTexture->ConnectDstProperty(lProp);

    return lHlslMat;
}

void CreateTableEntry(FbxBindingTable* pTable, FbxProperty& pProp)
{
    // create entry
    FbxBindingTableEntry& lEntry = pTable->AddNewEntry();
    // set src to the fbx property, in this sample, fbx properties have the same name with shader parameters
    FbxPropertyEntryView lSrc( &lEntry, true, true );
    // Because CgFX uses compound property, so do not use pProp.GetName()
    lSrc.SetProperty( pProp.GetHierarchicalName()); 

    // set dst to the shader parameter
    FbxSemanticEntryView lDst( &lEntry, false, true );
    lDst.SetSemantic( pProp.GetName() );
}
