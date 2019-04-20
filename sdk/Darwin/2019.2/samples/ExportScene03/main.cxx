/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The scene created in this example is a textured cube, a pyramid 
// with materials mapped on it's faces and triangle deformed by vertex cache. 
// An animation stack displays 6 different angles of all models.
//
// The example illustrates how to:
//        1) create a cube in mesh
//        2) map textures on diffuse, ambient and emissive channel of the cube
//        3) create an UV set for each channel with a texture for the cube
//        4) create a pyramid in mesh
//        5) create and map materials on the diffuse channel for each faces of the pyramid
//        6) create a cube with our custom mesh type
//        7) create and map materials on the diffuse channel for our custom mesh cube
//        8) create and add to our custom meshed cube a User Data Layer
//        9) create vertex cache deformer (by default maya caches are created in 64bits (mcx extension))
//       10) create an animation stack
//       11) animate vertex
//       12) export a scene in a .FBX file
//
//
// 1. To test vertex (3 doubles) cache (default) :
//
// ./ExportScene03 cacheFileName
//
// 2. To test int32 cache :
//
// ./ExportScene03 cacheFileName 1
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"
#include "MyKFbxMesh.h"

#define SAMPLE_FILENAME_MC		"ExportScene03_MC.fbx"
#define SAMPLE_FILENAME_PC2		"ExportScene03_PC2.fbx"
#define SAMPLE_CACHE_TYPE		2

#define PID_MY_GEOMETRY_LEMENT	0

// Function prototypes.
bool CreateScene(FbxScene* pScene, char* pSampleFileName);

FbxNode* CreateCubeWithTexture(FbxScene* pScene, const char* pName);
FbxNode* CreatePyramidWithMaterials(FbxScene* pScene, const char* pName);
FbxNode* CreateTriangle(FbxScene* pScene, const char* pName);
FbxNode* CreateCubeWithMaterialAndMyKFbxMesh(FbxScene* pScene, const char* pName);

void CreateTexture(FbxScene* pScene, FbxMesh* pMesh);
void CreateMaterials(FbxScene* pScene, FbxMesh* pMesh);
void CreateMaterialsWithMyKFbxMesh(FbxScene* pScene, MyKFbxMesh* pMyKFbxMesh);

void MapShapeOnPyramid(FbxScene* pScene, FbxNode* pPyramid);
void MapVertexCacheOnTriangle(FbxScene* pScene, FbxNode* pTriangle, char* pSampleFileName);

void SetCubeDefaultPosition(FbxNode* pCube);
void SetPyramidDefaultPosition(FbxNode* pPyramid);
void SetTriangleDefaultPosition(FbxNode* pTriangle);
void SetMyKFbxMeshCubeDefaultPosition(FbxNode* pMyKFbxCube);

void Animate(FbxNode* pNode, FbxAnimLayer* pAnimLayer);
void AnimateVertexCacheOnTriangleDoubleVertex(FbxNode* pNode, double pFrameRate);
void AnimateVertexCacheOnTriangleInt32(FbxNode* pNode, double pFrameRate);
void AnimateVertexCacheOnTriangleFloat(FbxNode* pNode, double pFrameRate);

bool gExportVertexCacheMCFormat = true;

// Declare the UV names globally so we can create them on the mesh and then assign them properly
// to our textures when we create them
static const char* gDiffuseElementName = "DiffuseUV";
static const char* gAmbientElementName = "AmbientUV";
static const char* gEmissiveElementName = "EmissiveUV";

// gCacheType == 0 (default)  - double vertex array
//            == 1            - int32 array
//            == 2            - float array
int gCacheType = -1;

int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    char* lSampleFileName = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    //Add the new class we have created to the Sdk Manager
    //Our class MyKFbxMesh is derived from FbxMesh
    lSdkManager->RegisterFbxClass("MyKFbxMesh", FBX_TYPE(MyKFbxMesh), FBX_TYPE(FbxMesh));
    //Now, our class MyKFbxMesh is ready to be used

    lSdkManager->RegisterFbxClass("MyFbxObject", FBX_TYPE(MyFbxObject), FBX_TYPE(FbxObject), "MyFbxObjectType", "MyFbxObjectSubType");

    //The example can take an output file name as an argument, and a cache format
	for( int i = 1; i < argc; ++i )
	{
		if( FBXSDK_stricmp(argv[i], "-test") == 0 ) continue;
		else
		{
			if( !lSampleFileName ) lSampleFileName = argv[i];
			else if( gCacheType == -1 ) gCacheType = atoi(argv[i]);
		}
	}
	if( !lSampleFileName ) lSampleFileName = gExportVertexCacheMCFormat ? (char *)SAMPLE_FILENAME_MC : (char *)SAMPLE_FILENAME_PC2;
	if( gCacheType == -1 ) gCacheType = SAMPLE_CACHE_TYPE;

    // Create the scene.
    lResult = CreateScene(lScene, lSampleFileName);

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while creating the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return 0;
    }

    // Save the scene.
    lResult = SaveScene(lSdkManager, lScene, lSampleFileName);

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

bool CreateScene(FbxScene* pScene, char* pSampleFileName)
{
    FbxNode* lCube = CreateCubeWithTexture(pScene, "Cube");
    FbxNode* lPyramid = CreatePyramidWithMaterials(pScene, "Pyramid");
    FbxNode* lTriangle = CreateTriangle(pScene, "Triangle");
    FbxNode* lMyKFbxMeshCube = CreateCubeWithMaterialAndMyKFbxMesh(pScene, "CubeMyKFbxMesh");
    MyFbxObject* lMyFbxObject = MyFbxObject::Create(pScene, "MyFbxObject 1");

    MapShapeOnPyramid(pScene, lPyramid);
    MapVertexCacheOnTriangle(pScene, lTriangle, pSampleFileName);

    SetCubeDefaultPosition(lCube);
    SetPyramidDefaultPosition(lPyramid);
    SetTriangleDefaultPosition(lTriangle);
    SetMyKFbxMeshCubeDefaultPosition(lMyKFbxMeshCube);

    // Build the node tree.
    FbxNode* lRootNode = pScene->GetRootNode();
    lRootNode->AddChild(lCube);
    lRootNode->AddChild(lPyramid);
    lRootNode->AddChild(lMyKFbxMeshCube);
    lRootNode->AddChild(lTriangle);
    lRootNode->ConnectSrcObject(lMyFbxObject);

    // Create the Animation Stack
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, "Show all faces");

    // The animation nodes can only exist on AnimLayers therefore it is mandatory to
    // add at least one AnimLayer to the AnimStack. And for the purpose of this example,
    // one layer is all we need.
    FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
    lAnimStack->AddMember(lAnimLayer);

    //Create a simple animated fcurve
    FbxProperty lMyProperty = lMyFbxObject->FindProperty("MyAnimatedPropertyName");
    if( lMyProperty.IsValid() )
    {
        lMyProperty.Set(0.0);   //Default value
        lMyProperty.ModifyFlag(FbxPropertyFlags::eAnimatable, true);
        lMyProperty.CreateCurveNode(lAnimLayer);
        FbxAnimCurve* lMyFCurve = lMyProperty.GetCurve(lAnimLayer, true);
        if( lMyFCurve )
        {
            FbxAnimCurveKey key;

            key.Set(FBXSDK_TIME_ZERO, -100); lMyFCurve->KeyAdd(key.GetTime(), key);
            key.Set(FbxTime(100), 0)   ; lMyFCurve->KeyAdd(key.GetTime(), key);
            key.Set(FbxTime(200), 100) ; lMyFCurve->KeyAdd(key.GetTime(), key);
        }
    }

    Animate(lCube, lAnimLayer);
    Animate(lPyramid, lAnimLayer);
    Animate(lMyKFbxMeshCube, lAnimLayer);
    FbxGlobalSettings& lGlobalSettings = pScene->GetGlobalSettings();

    switch(gCacheType) 
    {
    case 0:
    default:
	    AnimateVertexCacheOnTriangleDoubleVertex(lTriangle, FbxTime::GetFrameRate(lGlobalSettings.GetTimeMode()));
	    break;
    case 1:
	    AnimateVertexCacheOnTriangleInt32(lTriangle, FbxTime::GetFrameRate(lGlobalSettings.GetTimeMode()));
	    break;
    case 2:
	    AnimateVertexCacheOnTriangleFloat(lTriangle, FbxTime::GetFrameRate(lGlobalSettings.GetTimeMode()));
	    break;
    }

    return true;
}

// Create a cube with a texture. 
FbxNode* CreateCubeWithTexture(FbxScene* pScene, const char* pName)
{
    int i, j;
    FbxMesh* lMesh = FbxMesh::Create(pScene,pName);

    FbxVector4 lControlPoint0(-50, 0, 50);
    FbxVector4 lControlPoint1(50, 0, 50);
    FbxVector4 lControlPoint2(50, 100, 50);
    FbxVector4 lControlPoint3(-50, 100, 50);
    FbxVector4 lControlPoint4(-50, 0, -50);
    FbxVector4 lControlPoint5(50, 0, -50);
    FbxVector4 lControlPoint6(50, 100, -50);
    FbxVector4 lControlPoint7(-50, 100, -50);

    FbxVector4 lNormalXPos(1, 0, 0);
    FbxVector4 lNormalXNeg(-1, 0, 0);
    FbxVector4 lNormalYPos(0, 1, 0);
    FbxVector4 lNormalYNeg(0, -1, 0);
    FbxVector4 lNormalZPos(0, 0, 1);
    FbxVector4 lNormalZNeg(0, 0, -1);

    // Create control points.
    lMesh->InitControlPoints(24);
    FbxVector4* lControlPoints = lMesh->GetControlPoints();

    lControlPoints[0] = lControlPoint0;
    lControlPoints[1] = lControlPoint1;
    lControlPoints[2] = lControlPoint2;
    lControlPoints[3] = lControlPoint3;
    lControlPoints[4] = lControlPoint1;
    lControlPoints[5] = lControlPoint5;
    lControlPoints[6] = lControlPoint6;
    lControlPoints[7] = lControlPoint2;
    lControlPoints[8] = lControlPoint5;
    lControlPoints[9] = lControlPoint4;
    lControlPoints[10] = lControlPoint7;
    lControlPoints[11] = lControlPoint6;
    lControlPoints[12] = lControlPoint4;
    lControlPoints[13] = lControlPoint0;
    lControlPoints[14] = lControlPoint3;
    lControlPoints[15] = lControlPoint7;
    lControlPoints[16] = lControlPoint3;
    lControlPoints[17] = lControlPoint2;
    lControlPoints[18] = lControlPoint6;
    lControlPoints[19] = lControlPoint7;
    lControlPoints[20] = lControlPoint1;
    lControlPoints[21] = lControlPoint0;
    lControlPoints[22] = lControlPoint4;
    lControlPoints[23] = lControlPoint5;


    // We want to have one normal for each vertex (or control point),
    // so we set the mapping mode to eByControlPoint.
    FbxGeometryElementNormal* lGeometryElementNormal= lMesh->CreateElementNormal();

    lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);

    // Here are two different ways to set the normal values.
    bool firstWayNormalCalculations=true;
    if (firstWayNormalCalculations)
    {    
        // The first method is to set the actual normal value
        // for every control point.
        lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);

        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
    }
    else
    {
        // The second method is to the possible values of the normals
        // in the direct array, and set the index of that value
        // in the index array for every control point.
        lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

        // Add the 6 different normals to the direct array
        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);

        // Now for each control point, we need to specify which normal to use
        lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
    }

    // Array of polygon vertices.
    int lPolygonVertices[] = { 0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15,
        16, 17, 18, 19,
        20, 21, 22, 23 };

    // Create UV for Diffuse channel
    FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV( gDiffuseElementName);
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


    // Create UV for Ambient channel
    FbxGeometryElementUV* lUVAmbientElement = lMesh->CreateElementUV(gAmbientElementName);

    lUVAmbientElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    lUVAmbientElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    lVectors0.Set(0, 0);
    lVectors1.Set(1, 0);
    lVectors2.Set(0, 0.418586879968643);
    lVectors3.Set(1, 0.418586879968643);

    lUVAmbientElement->GetDirectArray().Add(lVectors0);
    lUVAmbientElement->GetDirectArray().Add(lVectors1);
    lUVAmbientElement->GetDirectArray().Add(lVectors2);
    lUVAmbientElement->GetDirectArray().Add(lVectors3);

    // Create UV for Emissive channel
    FbxGeometryElementUV* lUVEmissiveElement = lMesh->CreateElementUV(gEmissiveElementName);

    lUVEmissiveElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    lUVEmissiveElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    lVectors0.Set(0.2343, 0);
    lVectors1.Set(1, 0.555);
    lVectors2.Set(0.333, 0.999);
    lVectors3.Set(0.555, 0.666);

    lUVEmissiveElement->GetDirectArray().Add(lVectors0);
    lUVEmissiveElement->GetDirectArray().Add(lVectors1);
    lUVEmissiveElement->GetDirectArray().Add(lVectors2);
    lUVEmissiveElement->GetDirectArray().Add(lVectors3);

    //Now we have set the UVs as eIndexToDirect reference and in eByPolygonVertex  mapping mode
    //we must update the size of the index array.
    lUVDiffuseElement->GetIndexArray().SetCount(24);
    lUVAmbientElement->GetIndexArray().SetCount(24);
    lUVEmissiveElement->GetIndexArray().SetCount(24);



    // Create polygons. Assign texture and texture UV indices.
    for(i = 0; i < 6; i++)
    {
        //we won't use the default way of assigning textures, as we have
        //textures on more than just the default (diffuse) channel.
        lMesh->BeginPolygon(-1, -1, false);



        for(j = 0; j < 4; j++)
        {
            //this function points 
            lMesh->AddPolygon(lPolygonVertices[i*4 + j] // Control point index. 
            );
            //Now we have to update the index array of the UVs for diffuse, ambient and emissive
            lUVDiffuseElement->GetIndexArray().SetAt(i*4+j, j);
            lUVAmbientElement->GetIndexArray().SetAt(i*4+j, j);
            lUVEmissiveElement->GetIndexArray().SetAt(i*4+j, j);

        }

        lMesh->EndPolygon ();
    }

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lMesh);
    lNode->SetShadingMode(FbxNode::eTextureShading);

    CreateTexture(pScene, lMesh);

    return lNode;
}

// Create a pyramid with materials. 
FbxNode* CreatePyramidWithMaterials(FbxScene* pScene, const char* pName)
{
    int i, j;
    FbxMesh* lMesh = FbxMesh::Create(pScene, pName);

    FbxVector4 lControlPoint0(-50, 0, 50);
    FbxVector4 lControlPoint1(50, 0, 50);
    FbxVector4 lControlPoint2(50, 0, -50);
    FbxVector4 lControlPoint3(-50, 0, -50);
    FbxVector4 lControlPoint4(0, 100, 0);

    FbxVector4 lNormalP0(0, 1, 0);
    FbxVector4 lNormalP1(0, 0.447, 0.894);
    FbxVector4 lNormalP2(0.894, 0.447, 0);
    FbxVector4 lNormalP3(0, 0.447, -0.894);
    FbxVector4 lNormalP4(-0.894, 0.447, 0);

    // Create control points.
    lMesh->InitControlPoints(16);
    FbxVector4* lControlPoints = lMesh->GetControlPoints();

    lControlPoints[0] = lControlPoint0;
    lControlPoints[1] = lControlPoint1;
    lControlPoints[2] = lControlPoint2;
    lControlPoints[3] = lControlPoint3;
    lControlPoints[4] = lControlPoint0;
    lControlPoints[5] = lControlPoint1;
    lControlPoints[6] = lControlPoint4;
    lControlPoints[7] = lControlPoint1;
    lControlPoints[8] = lControlPoint2;
    lControlPoints[9] = lControlPoint4;
    lControlPoints[10] = lControlPoint2;
    lControlPoints[11] = lControlPoint3;
    lControlPoints[12] = lControlPoint4;
    lControlPoints[13] = lControlPoint3;
    lControlPoints[14] = lControlPoint0;
    lControlPoints[15] = lControlPoint4;

    // specify normals per control point.

    FbxGeometryElementNormal* lNormalElement= lMesh->CreateElementNormal();
    lNormalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
    lNormalElement->SetReferenceMode(FbxGeometryElement::eDirect);

    lNormalElement->GetDirectArray().Add(lNormalP0);
    lNormalElement->GetDirectArray().Add(lNormalP0);
    lNormalElement->GetDirectArray().Add(lNormalP0);
    lNormalElement->GetDirectArray().Add(lNormalP0);
    lNormalElement->GetDirectArray().Add(lNormalP1);
    lNormalElement->GetDirectArray().Add(lNormalP1);
    lNormalElement->GetDirectArray().Add(lNormalP1);
    lNormalElement->GetDirectArray().Add(lNormalP2);
    lNormalElement->GetDirectArray().Add(lNormalP2);
    lNormalElement->GetDirectArray().Add(lNormalP2);
    lNormalElement->GetDirectArray().Add(lNormalP3);
    lNormalElement->GetDirectArray().Add(lNormalP3);
    lNormalElement->GetDirectArray().Add(lNormalP3);
    lNormalElement->GetDirectArray().Add(lNormalP4);
    lNormalElement->GetDirectArray().Add(lNormalP4);
    lNormalElement->GetDirectArray().Add(lNormalP4);


    // Array of polygon vertices.
    int lPolygonVertices[] = { 0, 3, 2, 1,
        4, 5, 6,
        7, 8, 9,
        10, 11, 12,
        13, 14, 15 };

    // Set material mapping.
    FbxGeometryElementMaterial* lMaterialElement = lMesh->CreateElementMaterial();
    lMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
    lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    // Create polygons. Assign material indices.

    // Pyramid base.
    lMesh->BeginPolygon(0); // Material index.

    for(j = 0; j < 4; j++)
    {
        lMesh->AddPolygon(lPolygonVertices[j]); // Control point index.
    }

    lMesh->EndPolygon ();

    // Pyramid sides.
    for(i = 1; i < 5; i++)
    {
        lMesh->BeginPolygon(i); // Material index.

        for(j = 0; j < 3; j++)
        {
            lMesh->AddPolygon(lPolygonVertices[4 + 3*(i - 1) + j]); // Control point index.
        }

        lMesh->EndPolygon ();
    }


    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lMesh);

    CreateMaterials(pScene, lMesh);

    return lNode;
}

FbxNode* CreateTriangle(FbxScene* pScene, const char* pName)
{
    FbxMesh* lMesh = FbxMesh::Create(pScene, pName);

    // The three vertices
    FbxVector4 lControlPoint0(-50, 0, 50);
    FbxVector4 lControlPoint1(50, 0, 50);
    FbxVector4 lControlPoint2(0, 50, -50);

    // Create control points.
    lMesh->InitControlPoints(3);
    FbxVector4* lControlPoints = lMesh->GetControlPoints();

    lControlPoints[0] = lControlPoint0;
    lControlPoints[1] = lControlPoint1;
    lControlPoints[2] = lControlPoint2;

    // Create the triangle's polygon
    lMesh->BeginPolygon();
    lMesh->AddPolygon(0); // Control point 0
    lMesh->AddPolygon(1); // Control point 1
    lMesh->AddPolygon(2); // Control point 2
    lMesh->EndPolygon();

    FbxNode* lNode = FbxNode::Create(pScene,pName);
    lNode->SetNodeAttribute(lMesh);

    return lNode;
}

FbxNode* CreateCubeWithMaterialAndMyKFbxMesh(FbxScene* pScene, const char* pName)
{
    int i, j;

    //create a cube with our newly created class
    MyKFbxMesh* lMyKFbxMesh = MyKFbxMesh::Create(pScene,pName);
    FbxDouble3 lVector3(0.1, 0.2, 0.3);
    FbxDouble4 lVector4(0.1, 0.2, 0.3, 0.4);
    FbxDouble4 lVector41(1.1, 1.2, 1.3, 1.4);
    FbxDouble4 lVector42(2.1, 2.2, 2.3, 2.4);
    FbxDouble4 lVector43(3.1, 3.2, 3.3, 3.4);
    FbxDouble4x4 lMatrix(lVector4,lVector41,lVector42,lVector43);

    FbxColor lGreen(0.0, 0.0, 1.0);

    FbxTime lTime(333);
    //Set user-specific properties of our classes
    FbxString lString = "My Property 5 Value";
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY1).Set(true);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY2).Set((int) 1);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY3).Set((float)2.2);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY4).Set((double)3.3);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY5).Set(lString);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY6).Set(lVector3);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY6).Set(lGreen);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY8).Set(lVector4);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY9).Set(lMatrix);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY10).Set(3);
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY10).AddEnumValue("AAA");
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY10).AddEnumValue("BBB");
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY10).AddEnumValue("CCC");
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY10).AddEnumValue("DDD");
    lMyKFbxMesh->GetProperty((int) MyKFbxMesh::eMY_PROPERTY11).Set(lTime);

    FbxVector4 lControlPoint0(-25, 0, 25);
    FbxVector4 lControlPoint1(25, 0, 25);
    FbxVector4 lControlPoint2(25, 50, 25);
    FbxVector4 lControlPoint3(-25, 50, 25);
    FbxVector4 lControlPoint4(-25, 0, -25);
    FbxVector4 lControlPoint5(25, 0, -25);
    FbxVector4 lControlPoint6(25, 50, -25);
    FbxVector4 lControlPoint7(-25, 50, -25);

    FbxVector4 lNormalXPos(1, 0, 0);
    FbxVector4 lNormalXNeg(-1, 0, 0);
    FbxVector4 lNormalYPos(0, 1, 0);
    FbxVector4 lNormalYNeg(0, -1, 0);
    FbxVector4 lNormalZPos(0, 0, 1);
    FbxVector4 lNormalZNeg(0, 0, -1);

    // Create control points.
    lMyKFbxMesh->InitControlPoints(24);
    FbxVector4* lControlPoints = lMyKFbxMesh->GetControlPoints();

    lControlPoints[0] = lControlPoint0;
    lControlPoints[1] = lControlPoint1;
    lControlPoints[2] = lControlPoint2;
    lControlPoints[3] = lControlPoint3;
    lControlPoints[4] = lControlPoint1;
    lControlPoints[5] = lControlPoint5;
    lControlPoints[6] = lControlPoint6;
    lControlPoints[7] = lControlPoint2;
    lControlPoints[8] = lControlPoint5;
    lControlPoints[9] = lControlPoint4;
    lControlPoints[10] = lControlPoint7;
    lControlPoints[11] = lControlPoint6;
    lControlPoints[12] = lControlPoint4;
    lControlPoints[13] = lControlPoint0;
    lControlPoints[14] = lControlPoint3;
    lControlPoints[15] = lControlPoint7;
    lControlPoints[16] = lControlPoint3;
    lControlPoints[17] = lControlPoint2;
    lControlPoints[18] = lControlPoint6;
    lControlPoints[19] = lControlPoint7;
    lControlPoints[20] = lControlPoint1;
    lControlPoints[21] = lControlPoint0;
    lControlPoints[22] = lControlPoint4;
    lControlPoints[23] = lControlPoint5;


    // We want to have one normal for each vertex (or control point),
    // so we set the mapping mode to eByControlPoint.
    FbxGeometryElementNormal* lGeometryElementNormal = lMyKFbxMesh->CreateElementNormal();
    lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);


    // The second method is to the possible values of the normals
    // in the direct array, and set the index of that value
    // in the index array for every control point.
    lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    // Add the 6 different normals to the direct array
    lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
    lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
    lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
    lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
    lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
    lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);

    // Now for each control point, we need to specify which normal to use
    lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
    lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.


    // Array of polygon vertices.
    int lPolygonVertices[] = { 0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15,
        16, 17, 18, 19,
        20, 21, 22, 23 };

    // Set material mapping.
    FbxGeometryElementMaterial* lMaterialElement = lMyKFbxMesh->CreateElementMaterial();
    lMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
    lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    // Create UV coordinates.
    FbxGeometryElementUV* lUVElement = lMyKFbxMesh->CreateElementUV( "");
	FBX_ASSERT( lUVElement != NULL);
    lUVElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    lUVElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    FbxVector2 lVectors0(0, 0);
    FbxVector2 lVectors1(1, 0);
    FbxVector2 lVectors2(1, 1);
    FbxVector2 lVectors3(0, 1);

    lUVElement->GetDirectArray().Add(lVectors0);
    lUVElement->GetDirectArray().Add(lVectors1);
    lUVElement->GetDirectArray().Add(lVectors2);
    lUVElement->GetDirectArray().Add(lVectors3);

    for(i = 0; i < 6; i++)
    {
        //we created 6 lambert materials in the MyKFbxMesh
        //make each face use a different one
        lMyKFbxMesh->BeginPolygon(i);

        for(j = 0; j < 4; j++)
        {
            lMyKFbxMesh->AddPolygon(lPolygonVertices[i*4 + j], // Control point index. 
                j); // Valid texture UV index since texture UV mapping is by polygon vertex.
        }

        lMyKFbxMesh->EndPolygon ();
    }


    //Add a User Data Element
    //As of now, the types supported by a User Data Element are: FbxBoolDT, FbxIntDT, FbxFloatDT and FbxDoubleDT

    //For this example, we will create a element which possess 1 float and 1 bool

    //create a template array of KFbxDataTypes
    FbxArray<FbxDataType> lArrayType;

    //Create a template array of const char*
    FbxArray<const char*> lArrayNames;

    //let's add our types and the names of each of the added types
    lArrayType.Add(FbxFloatDT);
    lArrayNames.Add("My Float");

    lArrayType.Add(FbxBoolDT);
    lArrayNames.Add("My Bool");


    //Now we are ready to create the User Data Element
	FbxGeometryElementUserData* lFbxGeometryElementUserData = FbxGeometryElementUserData::Create(lMyKFbxMesh, "My Geometry Element",PID_MY_GEOMETRY_LEMENT,lArrayType, lArrayNames);
	//And UserData create function is still in implementing


    //For this example we will set the mapping mode to POLYGON_VERTEX
    lFbxGeometryElementUserData->SetMappingMode(FbxGeometryElement::eByPolygonVertex);

    //As we are using the eDirect Reference mode, and we are using polygon vertex Mapping mode
    //we have to resize the direct array to the number of polygon vertex we have in this mesh
    lFbxGeometryElementUserData->ResizeAllDirectArrays(lMyKFbxMesh->GetPolygonVertexCount());


    //To change the values in the direct array, we simply get the array and modify what we need to
    FbxLayerElementArrayTemplate<void*>* directArrayF = lFbxGeometryElementUserData->GetDirectArrayVoid("My Float");
    float *lDirectArrayFloat = NULL;
    lDirectArrayFloat = directArrayF->GetLocked(lDirectArrayFloat);

    FbxLayerElementArrayTemplate<void*>* directArrayB = lFbxGeometryElementUserData->GetDirectArrayVoid("My Bool");
    bool *lDirectArrayBool = NULL;
    directArrayB->GetLocked(lDirectArrayBool);

    //Modify every data for each polygon vertex on our mesh with some value
    for(i=0; i<lMyKFbxMesh->GetPolygonVertexCount(); ++i)
    {
        if(lDirectArrayFloat)
            lDirectArrayFloat[i]=(float)(i+0.5);
        if(lDirectArrayBool)
            lDirectArrayBool[i]= (i%2==0);
    }

    directArrayF->Release((void**)&lDirectArrayFloat);
    directArrayB->Release((void**)&lDirectArrayBool);

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lMyKFbxMesh);
    lNode->SetShadingMode(FbxNode::eTextureShading);

    //let's create the materials
    //6 materials, 1 for each face of the cube
    CreateMaterialsWithMyKFbxMesh(pScene, lMyKFbxMesh);

    return lNode;
}


// Create texture for cube.
void CreateTexture(FbxScene* pScene, FbxMesh* pMesh)
{
    // A texture need to be connected to a property on the material,
    // so let's use the material (if it exists) or create a new one
    FbxSurfacePhong* lMaterial = NULL;

    //get the node of mesh, add material for it.
    FbxNode* lNode = pMesh->GetNode();
    if(lNode)
    {
        lMaterial = lNode->GetSrcObject<FbxSurfacePhong>(0);
        if (lMaterial == NULL)
        {      
            FbxString lMaterialName = "toto";
            FbxString lShadingName  = "Phong";
            FbxDouble3 lBlack(0.0, 0.0, 0.0);
            FbxDouble3 lRed(1.0, 0.0, 0.0);
            FbxDouble3 lDiffuseColor(0.75, 0.75, 0.0);

            FbxLayer* lLayer = pMesh->GetLayer(0);     

            // Create a layer element material to handle proper mapping.
            FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create(pMesh, lMaterialName.Buffer());

            // This allows us to control where the materials are mapped.  Using eAllSame
            // means that all faces/polygons of the mesh will be assigned the same material.
            lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
            lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);
            
            // Save the material on the layer
            lLayer->SetMaterials(lLayerElementMaterial);

            // Add an index to the lLayerElementMaterial.  Since we have only one, and are using eAllSame mapping mode,
            // we only need to add one.
            lLayerElementMaterial->GetIndexArray().Add(0);

            lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

            // Generate primary and secondary colors.
            lMaterial->Emissive           .Set(lBlack);
            lMaterial->Ambient            .Set(lRed);
            lMaterial->AmbientFactor      .Set(1.);
            // Add texture for diffuse channel
            lMaterial->Diffuse           .Set(lDiffuseColor);
            lMaterial->DiffuseFactor     .Set(1.);
            lMaterial->TransparencyFactor.Set(0.4);
            lMaterial->ShadingModel      .Set(lShadingName);
            lMaterial->Shininess         .Set(0.5);
            lMaterial->Specular          .Set(lBlack);
            lMaterial->SpecularFactor    .Set(0.3);
            lNode->AddMaterial(lMaterial);
        }
    }

    FbxFileTexture* lTexture = FbxFileTexture::Create(pScene,"Diffuse Texture");

    // Set texture properties.
    lTexture->SetFileName("scene03.jpg"); // Resource file is in current directory.
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);
    lTexture->UVSet.Set(FbxString(gDiffuseElementName)); // Connect texture to the proper UV

    
    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Diffuse.ConnectSrcObject(lTexture);

    lTexture = FbxFileTexture::Create(pScene,"Ambient Texture");

    // Set texture properties.
    lTexture->SetFileName("gradient.jpg"); // Resource file is in current directory.
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);
    lTexture->UVSet.Set(FbxString(gAmbientElementName)); // Connect texture to the proper UV

    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Ambient.ConnectSrcObject(lTexture);

    lTexture = FbxFileTexture::Create(pScene,"Emissive Texture");

    // Set texture properties.
    lTexture->SetFileName("spotty.jpg"); // Resource file is in current directory.
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);
    lTexture->UVSet.Set(FbxString(gEmissiveElementName)); // Connect texture to the proper UV

    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Emissive.ConnectSrcObject(lTexture);
}

// Create materials for pyramid.
void CreateMaterials(FbxScene* pScene, FbxMesh* pMesh)
{
    int i;

    for (i = 0; i < 5; i++ )
    {
        FbxString lMaterialName = "material";
        FbxString lShadingName = "Phong";
        lMaterialName += i;
        FbxDouble3 lBlack(0.0, 0.0, 0.0);
        FbxDouble3 lRed(1.0, 0.0, 0.0);
        FbxDouble3 lColor;
        FbxSurfacePhong *lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());


        // Generate primary and secondary colors.
        lMaterial->Emissive.Set(lBlack);
        lMaterial->Ambient.Set(lRed);
        lColor = FbxDouble3(i > 2   ? 1.0 : 0.0, 
            i > 0 && i < 4 ? 1.0 : 0.0, 
            i % 2   ? 0.0 : 1.0);
        lMaterial->Diffuse.Set(lColor);
        lMaterial->TransparencyFactor.Set(0.0);
        lMaterial->ShadingModel.Set(lShadingName);
        lMaterial->Shininess.Set(0.5);

        //get the node of mesh, add material for it.
        FbxNode* lNode = pMesh->GetNode();
        if(lNode)             
            lNode->AddMaterial(lMaterial);
    }  
}

void CreateMaterialsWithMyKFbxMesh(FbxScene* pScene, MyKFbxMesh* pMyKFbxMesh)
{
    int i;
    for (i = 0; i != 6; ++i )
    {
        FbxString lMaterialName = "material";
        FbxString lShadingModelName = i%2==0 ? "Lambert" : "Phong";
        lMaterialName += i;
        FbxDouble3 lBlack(0.0, 0.0, 0.0);
        FbxDouble3 lRed(1.0, 0.0, 0.0);
        FbxDouble3 lColor;
        FbxSurfaceLambert *lMaterial = FbxSurfaceLambert::Create(pScene, lMaterialName.Buffer());


        // Generate primary and secondary colors.

        lMaterial->Emissive.Set(lBlack);
        lMaterial->Ambient.Set(lRed);
        lColor = FbxDouble3(i > 2   ? 1.0 : 0.0, 
            i > 0 && i < 4 ? 1.0 : 0.0, 
            i % 2   ? 0.0 : 1.0);
        lMaterial->Diffuse.Set(lColor);
        lMaterial->TransparencyFactor.Set(0.0);
        lMaterial->ShadingModel.Set(lShadingModelName);

        //get the node of mesh, add material for it. 
        FbxNode* lNode = pMyKFbxMesh->GetNode();
        if(lNode)
            lNode->AddMaterial(lMaterial);

    }
}

// Map pyramid control points onto an upside down shape.
void MapShapeOnPyramid(FbxScene* pScene, FbxNode* pPyramid)
{
    FbxShape* lShape = FbxShape::Create(pScene,"Upside Down");

    FbxVector4 lControlPoint0(-50, 100, 50);
    FbxVector4 lControlPoint1(50, 100, 50);
    FbxVector4 lControlPoint2(50, 100, -50);
    FbxVector4 lControlPoint3(-50, 100, -50);
    FbxVector4 lControlPoint4(0, 0, 0);

    FbxVector4 lNormalP0(0, 1, 0);
    FbxVector4 lNormalP1(0, -0.447, 0.894);
    FbxVector4 lNormalP2(0.894, -0.447, 0);
    FbxVector4 lNormalP3(0, -0.447, -0.894);
    FbxVector4 lNormalP4(-0.894, -0.447, 0);

    // Create control points.
    lShape->InitControlPoints(16);
    FbxVector4* lControlPoints = lShape->GetControlPoints();

    lControlPoints[0] = lControlPoint0;
    lControlPoints[1] = lControlPoint1;
    lControlPoints[2] = lControlPoint2;
    lControlPoints[3] = lControlPoint3;
    lControlPoints[4] = lControlPoint0;
    lControlPoints[5] = lControlPoint1;
    lControlPoints[6] = lControlPoint4;
    lControlPoints[7] = lControlPoint1;
    lControlPoints[8] = lControlPoint2;
    lControlPoints[9] = lControlPoint4;
    lControlPoints[10] = lControlPoint2;
    lControlPoints[11] = lControlPoint3;
    lControlPoints[12] = lControlPoint4;
    lControlPoints[13] = lControlPoint3;
    lControlPoints[14] = lControlPoint0;
    lControlPoints[15] = lControlPoint4;


    FbxGeometryElementNormal* lNormalElement = lShape->CreateElementNormal();
    lNormalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
    lNormalElement->SetReferenceMode(FbxGeometryElement::eDirect);

    lNormalElement->GetDirectArray().Add(lNormalP0);
    lNormalElement->GetDirectArray().Add(lNormalP0);
    lNormalElement->GetDirectArray().Add(lNormalP0);
    lNormalElement->GetDirectArray().Add(lNormalP0);
    lNormalElement->GetDirectArray().Add(lNormalP1);
    lNormalElement->GetDirectArray().Add(lNormalP1);
    lNormalElement->GetDirectArray().Add(lNormalP1);
    lNormalElement->GetDirectArray().Add(lNormalP2);
    lNormalElement->GetDirectArray().Add(lNormalP2);
    lNormalElement->GetDirectArray().Add(lNormalP2);
    lNormalElement->GetDirectArray().Add(lNormalP3);
    lNormalElement->GetDirectArray().Add(lNormalP3);
    lNormalElement->GetDirectArray().Add(lNormalP3);
    lNormalElement->GetDirectArray().Add(lNormalP4);
    lNormalElement->GetDirectArray().Add(lNormalP4);
    lNormalElement->GetDirectArray().Add(lNormalP4);


	FbxBlendShape* lBlendShape = FbxBlendShape::Create(pScene,"");
	FbxBlendShapeChannel* lBlendShapeChannel = FbxBlendShapeChannel::Create(pScene, "");
	pPyramid->GetMesh()->AddDeformer(lBlendShape);
	lBlendShape->AddBlendShapeChannel(lBlendShapeChannel);
    lBlendShapeChannel->AddTargetShape(lShape);
}

void MapVertexCacheOnTriangle(FbxScene* pScene, FbxNode* pTriangle, char* pSampleFileName)
{
    // By convention, all cache files are created in a _fpc folder located at the same
    // place as the .fbx file. 
    FbxString lFBXAbsolutePath = FbxPathUtils::Resolve(pSampleFileName);

    // Create a cache directory with the same name as the fbx file
    FbxString lFPCAbsoluteDirectory;

    lFPCAbsoluteDirectory  = FbxPathUtils::GetFolderName(lFBXAbsolutePath);
    lFPCAbsoluteDirectory += "/";
    lFPCAbsoluteDirectory += FbxPathUtils::GetFileName(pSampleFileName, false);
    lFPCAbsoluteDirectory += "_fpc";

    // Make this path the shortest possible
    lFPCAbsoluteDirectory = FbxPathUtils::Clean(lFPCAbsoluteDirectory);

    // Now get the point cache absolute and relative file name
    FbxString lAbsolutePCFileName = lFPCAbsoluteDirectory + FbxString("/") + pTriangle->GetName();
    lAbsolutePCFileName += gExportVertexCacheMCFormat ? ".xml" : ".pc2";

    FbxString lRelativePCFileName = FbxPathUtils::GetRelativeFilePath(FbxPathUtils::GetFolderName(lFBXAbsolutePath)+"/", lAbsolutePCFileName);

    // Make sure the direcotry exist.
    if (!FbxPathUtils::Create(FbxPathUtils::GetFolderName(lAbsolutePCFileName)))
    {
        // Cannot create this directory. So do not create the point cache
        return;
    }

    //
    // Create the cache file
    //
    FbxCache* lCache = FbxCache::Create(pScene, pTriangle->GetName());

    lCache->SetCacheFileName(lRelativePCFileName, lAbsolutePCFileName);
    lCache->SetCacheFileFormat(gExportVertexCacheMCFormat ? FbxCache::eMayaCache : FbxCache::eMaxPointCacheV2);

    //
    // Create the vertex deformer
    //
    FbxVertexCacheDeformer* lDeformer = FbxVertexCacheDeformer::Create(pScene, pTriangle->GetName());

    lDeformer->SetCache(lCache);
    lDeformer->Channel = pTriangle->GetName();
    lDeformer->Active = true;

    // Apply the deformer on the mesh
    pTriangle->GetGeometry()->AddDeformer(lDeformer);

	if (gExportVertexCacheMCFormat && gCacheType != 1)
	{
		//
		// Create the second deformer for normal data
		//
		FbxString channelName(pTriangle->GetName());
		channelName += "_normals";

		lDeformer = FbxVertexCacheDeformer::Create(pScene, channelName);
		// normal cache data is stored with the points data in the same cache file,
		// so two deformers are connnected to the same cache file.
		lDeformer->SetCache(lCache);		
		lDeformer->Channel = channelName;
		lDeformer->Active = true;

		pTriangle->GetGeometry()->AddDeformer(lDeformer);
	}
}

// Cube is translated to the left.
void SetCubeDefaultPosition(FbxNode* pCube) 
{
    pCube->LclTranslation.Set(FbxVector4(-75.0, -50.0, 0.0));
    pCube->LclRotation.Set(FbxVector4(0.0, 0.0, 0.0));
    pCube->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));
}

// Pyramid is translated to the right.
void SetPyramidDefaultPosition(FbxNode* pPyramid) 
{
    pPyramid->LclTranslation.Set(FbxVector4(75.0, -50.0, 0.0));
    pPyramid->LclRotation.Set(FbxVector4(0.0, 0.0, 0.0));
    pPyramid->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));
}

void SetTriangleDefaultPosition(FbxNode* pTriangle)
{
    pTriangle->LclTranslation.Set(FbxVector4(200.0, -50.0, 0.0));
    pTriangle->LclRotation.Set(FbxVector4(0.0, 0.0, 0.0));
    pTriangle->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));
}

void SetMyKFbxMeshCubeDefaultPosition(FbxNode* pMyKFbxCube)
{
    pMyKFbxCube->LclTranslation.Set(FbxVector4(-200.0, -25.0, 0.0));
    pMyKFbxCube->LclRotation.Set(FbxVector4(0.0, 0.0, 0.0));
    pMyKFbxCube->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));
}

// Displays 6 different angles.
void Animate(FbxNode* pNode, FbxAnimLayer* pAnimLayer)
{
    FbxAnimCurve* lCurve = NULL;
    FbxTime lTime;
    int lKeyIndex = 0;

    lCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
    if (lCurve)
    {
		lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(0.5);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 90.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 180.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.5);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 270.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 360.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lCurve->KeyModifyEnd();
    }

    lCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
    if (lCurve)
    {
		lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(2.5);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 90.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(3.5);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, -90.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(4.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lCurve->KeyModifyEnd();
    }


    // The upside down shape is at index 0 because it is the only one.
    // The cube has no shape so the function returns NULL is this case.
    FbxGeometry* lGeometry = (FbxGeometry*) pNode->GetNodeAttribute();
    lCurve = lGeometry->GetShapeChannel(0, 0, pAnimLayer, true);
    if (lCurve)
    {
		lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 100.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(4.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lCurve->KeyModifyEnd();
    }
}

void AnimateVertexCacheOnTriangleDoubleVertex(FbxNode* pTriangle, double pFrameRate)
{
    //
    // Move the vertices from their original position to the center.
    //
    FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pTriangle->GetGeometry()->GetDeformer(0, FbxDeformer::eVertexCache));
    FbxCache*               lCache = lDeformer->GetCache();
    bool                     lRet;

    // Write samples for 4 seconds
    FbxTime lTimeIncrement, lCurrentTime, lStopTime;
    lTimeIncrement.SetTime(0, 0, 0, 1); // 1 frame @ current frame rate
    lStopTime.SetTime(0, 0, 4);         // 4 seconds

    unsigned int lFrameCount = (unsigned int)(lStopTime.Get()/lTimeIncrement.Get());
    FbxStatus lStatus;
	unsigned int lNormalChannelIndex;

    // Open the file for writing
    if (gExportVertexCacheMCFormat)
    {
        // The default maya cache is created in 64bits (mcx extension). To use the legacy 32bit format (mc) replace
        // FbxCache::eMCX with FBXCache::eMCC
        lRet = lCache->OpenFileForWrite(FbxCache::eMCOneFile, pFrameRate, pTriangle->GetName(), FbxCache::eMCX, FbxCache::eDoubleVectorArray, "positions", &lStatus);
		if (lRet)
		{
			FbxString channelName(pTriangle->GetName());
			channelName += "_normals";
			lRet = lCache->AddChannel(channelName, FbxCache::eFloatVectorArray, "normals", lNormalChannelIndex, &lStatus); 
		}
    }
    else
    {
        lRet = lCache->OpenFileForWrite(0.0, pFrameRate, lFrameCount, 3, &lStatus);  
    }

    if (!lRet)
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
        return;
    }

    int lChannelIndex = lCache->GetChannelIndex(pTriangle->GetName());
    unsigned int lCurrentFrame = 0;

    while (lCurrentTime <= lStopTime)
    {
        double lVertices[3][3];
        double lScaleFactor = 1.0-double(lCurrentTime.GetSecondDouble()/lStopTime.GetSecondDouble());

        lVertices[0][0] = -50.0 * lScaleFactor;  // X
        lVertices[0][1] = 0.0;                   // Y
        lVertices[0][2] = 50.0  * lScaleFactor;  // Z

        lVertices[1][0] = 50.0  * lScaleFactor;  // X
        lVertices[1][1] = 0.0;                   // Y
        lVertices[1][2] = 50.0  * lScaleFactor;  // Z

        lVertices[2][0] = 0.0   * lScaleFactor;  // X
        lVertices[2][1] = 50.0  * lScaleFactor;  // Y
        lVertices[2][2] = -50.0 * lScaleFactor;  // Z

		lCache->BeginWriteAt(lCurrentTime);
        if (gExportVertexCacheMCFormat)
        {
			float lNormals[3][3];

			for (int i = 0; i < 3; i++)
			{
				lNormals[i][0] = 0.0f;
				lNormals[i][1] = 1.0f;
				lNormals[i][2] = 0.0f;
			} 
			
            lCache->Write(lChannelIndex, lCurrentTime, &lVertices[0][0], 3);
			lCache->Write(lNormalChannelIndex, lCurrentTime, &lNormals[0][0], 3);
        }
        else
        {
            lCache->Write(lCurrentFrame, &lVertices[0][0]);
        }
		lCache->EndWriteAt();

        lCurrentTime += lTimeIncrement;
        lCurrentFrame++;
    }

    if (!lCache->CloseFile(&lStatus))
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
    }
}

void AnimateVertexCacheOnTriangleInt32(FbxNode* pTriangle, double pFrameRate)
{
    //
    // Move the vertices from their original position to the center.
    //
    FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pTriangle->GetGeometry()->GetDeformer(0, FbxDeformer::eVertexCache));
    FbxCache*               lCache = lDeformer->GetCache();
    bool                     lRet = false;

    // Write samples for 4 seconds
    FbxTime lTimeIncrement, lCurrentTime, lStopTime;
    lTimeIncrement.SetTime(0, 0, 0, 1); // 1 frame @ current frame rate
    lStopTime.SetTime(0, 0, 4);         // 4 seconds
    FbxStatus lStatus;

    // Open the file for writing int32 array
    if (gExportVertexCacheMCFormat)
    {
        // The default maya cache is created in 64bits (mcx extension). To use the legacy 32bit format (mc) replace
        // FbxCache::eMCX with FBXCache::eMCC
        lRet = lCache->OpenFileForWrite(FbxCache::eMCOneFile, pFrameRate, pTriangle->GetName(), FbxCache::eMCX, FbxCache::eInt32Array, "positions", &lStatus);
    }

    if (!lRet)
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
        return;
    }

    int lChannelIndex = lCache->GetChannelIndex(pTriangle->GetName());
    int lCurrentFrame = 0;

    while (lCurrentTime <= lStopTime)
    {
        int v[2];

	    v[0] = -10 + lCurrentFrame;
	    v[1] = v[0]+1;

        if (gExportVertexCacheMCFormat)
        {
            lCache->Write(lChannelIndex, lCurrentTime, &v[0], 2);
        }

        lCurrentTime += lTimeIncrement;
        lCurrentFrame++;
    }

    if (!lCache->CloseFile(&lStatus))
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
    }

    // Open the file for reading int32 array
    if (gExportVertexCacheMCFormat)
    {
        lRet = lCache->OpenFileForRead(&lStatus);
    }

    if (!lRet)
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
        return;
    }

    FbxTime lCurrentTime2;
    lCurrentFrame = 0;

    FBXSDK_printf("Testing awCache int32 array read and write\n");
    bool passTest = true;
    // FBXSDK_printf("Should print out -10 .. 110\n");
    while (lCurrentTime2 <= lStopTime)
    {
        int v[2];
        if (gExportVertexCacheMCFormat)
        {
            lCache->Read(lChannelIndex, lCurrentTime2, &v[0], 2);
	        if ((v[0] != -10 + lCurrentFrame) || (v[0]+1 != v[1]) ) 
	        {
		        FBXSDK_printf("awCache int32 array read/write mismatch\n");
		        passTest = false;
		        break;
	        }
	        // FBXSDK_printf("%d ",v[0]);
        }

        lCurrentTime2 += lTimeIncrement;
        lCurrentFrame++;
    }

    if (!lCache->CloseFile(&lStatus))
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
    }

    if (passTest)
    {
		FBXSDK_printf("awCache int32 array read and write test passed\n");
    }
}


void AnimateVertexCacheOnTriangleFloat(FbxNode* pTriangle, double pFrameRate)
{
    //
    // Move the vertices from their original position to the center.
    //
    FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pTriangle->GetGeometry()->GetDeformer(0, FbxDeformer::eVertexCache));
    FbxCache*               lCache = lDeformer->GetCache();
    bool                     lRet;

    // Write samples for 4 seconds
    FbxTime lTimeIncrement, lCurrentTime, lStopTime;
    lTimeIncrement.SetTime(0, 0, 0, 1); // 1 frame @ current frame rate
    lStopTime.SetTime(0, 0, 4);         // 4 seconds
    FbxStatus lStatus;
	unsigned int lNormalChannelIndex;

    // Open the file for writing
    if (gExportVertexCacheMCFormat)
    {
        // The default maya cache is created in 64bits (mcx extension). To use the legacy 32bit format (mc) replace
        // FbxCache::eMCX with FBXCache::eMCC
        lRet = lCache->OpenFileForWrite(FbxCache::eMCOneFile, pFrameRate, pTriangle->GetName(), FbxCache::eMCX, FbxCache::eFloatVectorArray, "positions", &lStatus);
		if (lRet)
		{
			FbxString channelName(pTriangle->GetName());
			channelName += "_normals";
			lRet = lCache->AddChannel(channelName, FbxCache::eFloatVectorArray, "normals", lNormalChannelIndex, &lStatus); 
		}
    }
    else
    {
        lRet = false;
    }

    if (!lRet)
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
        return;
    }

    int lChannelIndex = lCache->GetChannelIndex(pTriangle->GetName());
    unsigned int lCurrentFrame = 0;
	
	while (lCurrentTime <= lStopTime)
    {
        float lVertices[3][3];
        float lScaleFactor = 1.0f-float(lCurrentTime.GetSecondDouble()/lStopTime.GetSecondDouble());

        lVertices[0][0] = -50.0f * lScaleFactor;  // X
        lVertices[0][1] = 0.0f;                   // Y
        lVertices[0][2] = 50.0f  * lScaleFactor;  // Z

        lVertices[1][0] = 50.0f  * lScaleFactor;  // X
        lVertices[1][1] = 0.0f;                   // Y
        lVertices[1][2] = 50.0f  * lScaleFactor;  // Z

        lVertices[2][0] = 0.0f   * lScaleFactor;  // X
        lVertices[2][1] = 50.0f  * lScaleFactor;  // Y
        lVertices[2][2] = -50.0f * lScaleFactor;  // Z

		float lNormals[3][3];

		for (int i = 0; i < 3; i++)
		{
			lNormals[i][0] = 0.0f;
			lNormals[i][1] = 1.0f;
			lNormals[i][2] = 0.0f;
		}

        if (gExportVertexCacheMCFormat)
        {
			lCache->BeginWriteAt(lCurrentTime);

            lCache->Write(lChannelIndex, lCurrentTime, &lVertices[0][0], 3);
			lCache->Write(lNormalChannelIndex, lCurrentTime, &lNormals[0][0], 3);

			lCache->EndWriteAt();
        }

        lCurrentTime += lTimeIncrement;
        lCurrentFrame++;
    }


    if (!lCache->CloseFile(&lStatus))
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
    } 

    // Open the file for reading int32 array
    if (gExportVertexCacheMCFormat)
    {
        lRet = lCache->OpenFileForRead(&lStatus);
    }

    if (!lRet)
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
        return;
    }

    FbxTime lCurrentTime2;
    lCurrentFrame = 0;
    #define AbsFlt(a)      (((a) < 0) ? -(a) : (a))
    #define CmpFlt(a,b)    (AbsFlt((a)-(b)) > 1e-5)

    FBXSDK_printf("Testing awCache Float3 array read and write\n");
    bool passTest = true;
    while (lCurrentTime2 <= lStopTime && passTest)
    {
        float lVertices[3][3];
        float lNormals[3][3];
        float lScaleFactor = 1.0f-float(lCurrentTime2.GetSecondDouble()/lStopTime.GetSecondDouble());

        if (gExportVertexCacheMCFormat)
        {
            lCache->Read(lChannelIndex, lCurrentTime2, &lVertices[0][0], 3);
            lCache->Read(lNormalChannelIndex, lCurrentTime2, &lNormals[0][0], 3);
            
            if ((CmpFlt(lVertices[0][0], -50.0f * lScaleFactor) || CmpFlt(lVertices[0][1], 0.0f                ) || CmpFlt(lVertices[0][2], 50.0f * lScaleFactor)) ||
                (CmpFlt(lVertices[1][0],  50.0f * lScaleFactor) || CmpFlt(lVertices[1][1], 0.0f                ) || CmpFlt(lVertices[1][2], 50.0f * lScaleFactor)) ||
                (CmpFlt(lVertices[2][0],   0.0f * lScaleFactor) || CmpFlt(lVertices[2][1], 50.0f * lScaleFactor) || CmpFlt(lVertices[2][2],-50.0f * lScaleFactor)))
	        {
		        FBXSDK_printf("awCache Float3 vertex array read/write mismatch\n");
		        passTest = false;
		        break;
            }

			for (int i = 0; i < 3; i++)
			{
				if ((CmpFlt(lNormals[i][0], 0.0f) || CmpFlt(lNormals[i][1], 1.0f ) || CmpFlt(lNormals[i][2], 0.0f))) 
				{
					FBXSDK_printf("awCache Float3 normal array read/write mismatch\n");
					passTest = false;
					break;
				}
			}
	    }   

        lCurrentTime2 += lTimeIncrement;
        lCurrentFrame++;
    }

    #undef AbsFlt
    #undef CmpFlt
    
    if (!lCache->CloseFile(&lStatus))
    {
        // print out the error
        FBXSDK_printf("File open error: %s\n", lStatus.GetErrorString());
    }

    if (passTest)
    {
	    FBXSDK_printf("awCache float3 array read and write test passed\n");
    }

}


