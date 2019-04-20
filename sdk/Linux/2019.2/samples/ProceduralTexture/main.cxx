/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The scene created in this example is a plane with a procedural texture applied.
//
// The example illustrates how to:
//        1) Create a procedural texture
//        2) Set the blob property of a procedural texture
//        3) Get the blob property of a procedural texture and dump it on disk
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>
#include <fstream>
#include "../Common/Common.h"

#define SAMPLE_FILENAME "ProceduralTexture.fbx"
#define TEXTURE_FILENAME "a_texture.jpg"
#define FILENAME_PROP "Filename"

// Function prototypes.
bool CreateSceneAndSaveFile(int argc, char** argv);
bool ReadFileAndDumpProceduralTextureBlobOnDisk(int argc, char** argv);
bool DumpProceduralTextureBlobOnDisk(FbxScene* pScene);
bool CreateScene(FbxScene* pScene);
FbxNode* CreatePlane(FbxScene* pScene, const char* pName);
FbxSurfacePhong* CreatePhongMaterial(FbxScene* pScene, const char* pName);
FbxProceduralTexture* CreateProceduralTexture(FbxScene* pScene, const char* pName);
void MapPhong(FbxSurfacePhong* pPhong, FbxNode* pNode);
void MapProceduralTexure(FbxProceduralTexture* pProcTex, FbxNode* pNode);

int main(int argc, char** argv)
{
	bool lResult;
	
	lResult = CreateSceneAndSaveFile(argc, argv);
	if(lResult == false)
    {
		return 1;
	}

	lResult = ReadFileAndDumpProceduralTextureBlobOnDisk(argc, argv);
	if(lResult == false)
    {
		return 1;
	}

	return 0;
}

bool CreateSceneAndSaveFile(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene.

    lResult = CreateScene(lScene);

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while creating the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return false;
    }

    // Save the scene.
    // The example can take an output file name as an argument.
	FbxString lFilePath("");
	for( int i = 1, c = argc; i < c; ++i )
	{
		if( FbxString(argv[i]) == "-test" ) continue;
		else if( lFilePath.IsEmpty() ) lFilePath = argv[i];
	}
	if( lFilePath.IsEmpty() ) lFilePath = SAMPLE_FILENAME;

	FBXSDK_printf("Saving the file...\n");
	lResult = SaveScene(lSdkManager, lScene, lFilePath.Buffer());

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return false;
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

    return true;
}

bool ReadFileAndDumpProceduralTextureBlobOnDisk(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // The example can take an input file name as an argument.
	FbxString lFilePath("");
	for( int i = 1, c = argc; i < c; ++i )
	{
		if( FbxString(argv[i]) == "-test" ) continue;
		else if( lFilePath.IsEmpty() ) lFilePath = argv[i];
	}
	if( lFilePath.IsEmpty() ) lFilePath = SAMPLE_FILENAME;

	FBXSDK_printf("Reading the FBX file...\n");
	lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while loading the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return false;
    }

	// Create a new file for each procedural texture found in the file.
	lResult = DumpProceduralTextureBlobOnDisk(lScene);
    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while dumping procedural texture blobs on disk...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return false;
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

    return true;
}

bool DumpProceduralTextureBlobOnDisk(FbxScene* pScene)
{
	FBXSDK_printf("Writing the blob on disk...\n");
    // Collect all the procedural textures proxy objects in scene
	int lNbProcTex = pScene->GetSrcObjectCount<FbxProceduralTexture>();

	if (!lNbProcTex)
	{
		return true;
	}

	bool lWroteBlob = false; // Bool: Wrote at least one blob on disk

	// Directory for blob extraction
	FbxString lDirPath = FbxGetCurrentWorkPath() + "/Blobs/";

	for(int lIndex = 0; lIndex < lNbProcTex; lIndex++)
    {
        FbxProceduralTexture* lProcTex = pScene->GetSrcObject<FbxProceduralTexture>(lIndex);
        if(!lProcTex)
		{
            continue;
		}

		FbxProperty lFilenameProp = lProcTex->RootProperty.Find(FILENAME_PROP);
		if (!lFilenameProp.IsValid())
		{
			continue;
		}

		// Read binary blob
		void* lBlobBegin = NULL;
		size_t lBlobSize = 0;
		FbxBlob lBinaryBlob = lProcTex->GetBlob();
		lBlobSize = lBinaryBlob.Size();
		lBlobBegin = const_cast<void*>(lBinaryBlob.Access());
		
		// Get file name to dump the blob to.
		FbxString lFilename = lFilenameProp.Get<FbxString>();
		FbxString lFilePath = lDirPath + FbxPathUtils::GetFileName(lFilename, false) + lIndex + "." + FbxPathUtils::GetExtensionName(lFilename);

		bool lIsWritable = FbxPathUtils::Create(FbxPathUtils::GetFolderName(lFilePath));
		if (lIsWritable)
		{
			std::ofstream lDataStreamOut(lFilePath.Buffer(), std::ofstream::binary);
			lDataStreamOut.write((const char *)lBlobBegin, lBlobSize);
			lDataStreamOut.close();
			// So now we wrote the file!
			lWroteBlob = true;
			FBXSDK_printf("Blob is written on disk! File: %s\n", lFilePath.Buffer());
		}
	}

	return lWroteBlob;
}

bool CreateScene(FbxScene* pScene)
{
	FBXSDK_printf("Creating the scene...\n");
    FbxNode* lPlane = CreatePlane(pScene, "Plane");

	FbxSurfacePhong* lPhong = CreatePhongMaterial(pScene, "Phong");
	MapPhong(lPhong, lPlane);
	FbxProceduralTexture* lProcTex = CreateProceduralTexture(pScene, "ProcTex");
	MapProceduralTexure(lProcTex, lPlane);

    // Build the node tree.
    FbxNode* lRootNode = pScene->GetRootNode();
    lRootNode->AddChild(lPlane);

    return true;
}

// Create a plane. 
FbxNode* CreatePlane(FbxScene* pScene, const char* pName)
{
    FbxMesh* lMesh = FbxMesh::Create(pScene,pName);
    FbxVector4 lControlPoint0(-50, 0, 50);
    FbxVector4 lControlPoint1(50, 0, 50);
    FbxVector4 lControlPoint2(50, 100, 50);
    FbxVector4 lControlPoint3(-50, 100, 50);
	FbxVector4 lNormal(0, 0, 1);

	// Create control points.
	lMesh->InitControlPoints(4);
	FbxVector4* lControlPoints = lMesh->GetControlPoints();

    lControlPoints[0] = lControlPoint0;
    lControlPoints[1] = lControlPoint1;
    lControlPoints[2] = lControlPoint2;
    lControlPoints[3] = lControlPoint3;

    // Set the normals.
    // We want to have one normal for each vertex (or control point),
    // so we set the mapping mode to eByControlPoint.
    FbxGeometryElementNormal* lGeometryElementNormal = lMesh->CreateElementNormal();;

    lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);

    // Set the actual normal value for all 4 control points.
    lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);
	lGeometryElementNormal->GetDirectArray().Add(lNormal);
	lGeometryElementNormal->GetDirectArray().Add(lNormal);
	lGeometryElementNormal->GetDirectArray().Add(lNormal);
	lGeometryElementNormal->GetDirectArray().Add(lNormal);

    // Array of polygon vertices.
    int lPolygonVertices[] = { 0, 1, 2, 3 };

    // Create UV for Diffuse channel
    FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV( "DiffuseUV");
	FBX_ASSERT( lUVDiffuseElement != NULL);
    lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    FbxVector2 lVectors0(0, 0);
    FbxVector2 lVectors1(1, 0);
    FbxVector2 lVectors2(1, 1);
    FbxVector2 lVectors3(0, 1);

    lUVDiffuseElement->GetDirectArray().Add(lVectors0);
    lUVDiffuseElement->GetDirectArray().Add(lVectors1);
    lUVDiffuseElement->GetDirectArray().Add(lVectors2);
    lUVDiffuseElement->GetDirectArray().Add(lVectors3);

    //Now we have set the UVs as eIndexToDirect reference and in eByPolygonVertex  mapping mode
    //we must update the size of the index array.
    lUVDiffuseElement->GetIndexArray().SetCount(4);

	// Create polygon
	lMesh->BeginPolygon(-1, -1, false);
	for (int j = 0; j < 4; j++)
	{
		//this function points 
		lMesh->AddPolygon(lPolygonVertices[j]);
		//Now we have to update the index array of the UVs for diffuse
		lUVDiffuseElement->GetIndexArray().SetAt(j, j);
	}
    lMesh->EndPolygon();

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lMesh);

    return lNode;
}

// Create a procedural texture.
FbxProceduralTexture* CreateProceduralTexture(FbxScene* pScene, const char* pName)
{
	FbxProceduralTexture* lProceduralTexture = FbxProceduralTexture::Create(pScene, pName);

	// For this example we simply dump the binary content of a jpg to the 
	// procedural texture's blob property.
	// In the general case, dump whatever information needed for the procedural
	// texture to the blob property.
	
	FbxString lFilename = TEXTURE_FILENAME;

	//check whether the file is readable
	bool lIsReadable = FbxFileUtils::Exist(lFilename);

	if (lIsReadable)
	{
		// create binary blob from the texture file
		FbxBlob lBinaryBlob;
		std::ifstream lDataStreamIn(lFilename, std::ifstream::binary);
		char* lBlobBegin = (char*)malloc(4096);
		char* lBlobEnd = lBlobBegin;
		long long lBlobSize = 0;
		long long lBlobPointerSize = 4096;
		std::streamsize lNbRead = 0;
		while(!lDataStreamIn.eof())
		{
			lBlobEnd = lBlobBegin + lBlobSize;
			lDataStreamIn.read(lBlobEnd, 4096);
			lNbRead = lDataStreamIn.gcount();
			lBlobPointerSize += 4096;
			lBlobBegin = (char *)realloc(lBlobBegin, size_t(lBlobPointerSize));
			lBlobSize += lNbRead;
		}
		lDataStreamIn.close();
		lBinaryBlob.Assign(lBlobBegin, (int)lBlobSize);
		free(lBlobBegin);

		lProceduralTexture->SetBlob(lBinaryBlob); 
	}

	// Add a property to retain file name
	FbxProperty lFilenameProp = FbxProperty::Create(lProceduralTexture, FbxStringDT, FILENAME_PROP);
	if (lFilenameProp.IsValid())
	{
		lFilenameProp.Set(lFilename);
	}

	return lProceduralTexture;
}


FbxSurfacePhong* CreatePhongMaterial(FbxScene* pScene, const char* pName)
{
	FbxSurfacePhong* lPhong = FbxSurfacePhong::Create(pScene, pName);
	
	return lPhong;
}

// Map procedural texture over plane.
void MapProceduralTexure(FbxProceduralTexture* pProceduralTexture, FbxNode* pNode)
{
    // The note shading mode has to be set to FbxNode::eTextureShading for the texture to be displayed.
    pNode->SetShadingMode(FbxNode::eTextureShading);

    // we have to connect the texture to the material DiffuseColor property
    FbxSurfacePhong* lMaterial = pNode->GetSrcObject<FbxSurfacePhong>(0);
    if (lMaterial)
	{
        lMaterial->Diffuse.ConnectSrcObject(pProceduralTexture);
	}

}

// Map material over mesh.
void MapPhong(FbxSurfacePhong* pPhong, FbxNode* pNode)
{
    // Create MaterialElement in the mesh
    FbxMesh*						lMesh					= pNode->GetMesh();
    FbxGeometryElementMaterial*	lGeometryElementMaterial	= lMesh->CreateElementMaterial();

    // The material is mapped to the whole mesh
    lGeometryElementMaterial->SetMappingMode(FbxGeometryElement::eAllSame);

    // And the material is avalible in the Direct array
    lGeometryElementMaterial->SetReferenceMode(FbxGeometryElement::eDirect);
    pNode->AddMaterial(pPhong);
}

