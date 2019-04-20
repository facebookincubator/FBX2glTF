/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/****************************************************************************/
/* This example shows how to instanciate meshes and curves in FBX.          */
/*  - Create a cube                                                         */
/*  - Create instances of this cube (new nodes that point to the same mesh) */
/*  - Apply the same materials to the polygons of these cubes               */
/*  - Create an animation curve                                             */
/*  - Animate all cubes using the same animation curve.                     */
/****************************************************************************/

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME			"Instances.fbx"
#define SAMPLE_INSTANCE_COUNT	10

// Globals
int gNbCubes = -1;
FbxDouble3 gRed(1.0, 0.0, 0.0);
FbxDouble3 gGreen(0.0, 1.0, 0.0);
FbxDouble3 gBlue(0.0, 0.0, 1.0);
FbxDouble3 gGray(0.5, 0.5, 0.5);
FbxDouble3 gWhite(1.0, 1.0, 1.0);

FbxSurfacePhong* gMatWhite;
FbxSurfacePhong* gMatGray;
FbxSurfacePhong* gMatRed;
FbxSurfacePhong* gMatGreen;
FbxSurfacePhong* gMatBlue;

// Function prototypes.
bool CreateScene(FbxManager* pSdkManager, FbxScene* pScene);
FbxNode* CreateCube(FbxScene* pScene, const char* pName);
FbxNode* CreateCubeInstance(FbxScene* pScene, const char* pName, FbxMesh* pFirstCube);
FbxSurfacePhong* CreateMaterial(FbxScene* pScene, FbxDouble3 pColor);
FbxAnimCurve* CreateAnimCurve(FbxScene* pScene);
void AnimateCube(FbxNode* pCube, FbxAnimLayer* pAnimLayer, FbxAnimCurve* pAnimCurve, int pRotAxis);


int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // The example can take an output file name as an argument, and a cube number.
	const char* lSampleFileName = NULL;
	for( int i = 1; i < argc; ++i )
	{
		if( FBXSDK_stricmp(argv[i], "-test") == 0 ) continue;
		else
		{
			if( !lSampleFileName ) lSampleFileName = argv[i];
			else if ( gNbCubes < 1 ) gNbCubes = atoi(argv[i]);
		}
	}
	if( !lSampleFileName ) lSampleFileName = SAMPLE_FILENAME;
	if( gNbCubes < 1 ) gNbCubes = SAMPLE_INSTANCE_COUNT;

    // Create the scene.
    lResult = CreateScene(lSdkManager, lScene);

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

bool CreateScene(FbxManager* /*pSdkManager*/, FbxScene* pScene)
{
	// Initial cube position
    double lX = 0.0;
    double lY = 20.0;
    double lZ = 0.0;

	// Create global materials
	gMatWhite = CreateMaterial(pScene, gWhite);
	gMatGray  = CreateMaterial(pScene, gGray);
	gMatRed	  = CreateMaterial(pScene, gRed);
	gMatGreen = CreateMaterial(pScene, gGreen);
	gMatBlue  = CreateMaterial(pScene, gBlue);

	FbxMesh* lCubeMesh = NULL;
	FbxNode* lCubeNode = NULL;

	FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, "Cube Animation Stack");
	FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
	lAnimStack->AddMember(lAnimLayer);

	for (int i = 0; i < gNbCubes; i++)
	{
		FbxString lCubeName = "Cube ";
		lCubeName += (i+1);

		if (i == 0)
		{
			// Create first cube
			lCubeNode = CreateCube(pScene, lCubeName.Buffer());
			lCubeMesh = lCubeNode->GetMesh();
		}
		else
		{
			// Create cube instance
			lCubeNode = CreateCubeInstance(pScene, lCubeName.Buffer(), lCubeMesh);
		}

    	// set the cube position
    	lCubeNode->LclTranslation.Set(FbxVector4(lX, lY, lZ));

		// Animate cube
		// Create an animation curve for each node.
		FbxAnimCurve* lAnimCurve = CreateAnimCurve(pScene);
		AnimateCube(lCubeNode, lAnimLayer, lAnimCurve, i%3);
		// alternate sides of X
		if (lX >= 0)
		{
			lX += 50;
		}
		else
		{
			lX -= 50;
		}
		lX *= -1.0;
    	lY += 30.0;
	}

    return true;
}


// The cube rotates around X, Y or Z.
void AnimateCube(FbxNode* pCube, FbxAnimLayer* pAnimLayer, FbxAnimCurve* pAnimCurve, int pRotAxis)
{
    FbxAnimCurveNode *lCurveNode = pCube->LclRotation.GetCurveNode(pAnimLayer, true);
	
	// Find out which channel to animate: rotate around X axis, Y axis, or Z axis.
	if(pRotAxis == 0) 
	{
		lCurveNode->ConnectToChannel(pAnimCurve, FBXSDK_CURVENODE_COMPONENT_X);
	}
    else if(pRotAxis == 1)
	{
		lCurveNode->ConnectToChannel(pAnimCurve, FBXSDK_CURVENODE_COMPONENT_Y);
	}
    else if(pRotAxis == 2)
	{
		lCurveNode->ConnectToChannel(pAnimCurve, FBXSDK_CURVENODE_COMPONENT_Z);
	}
}

// Create a cube with a fresh new mesh, and add it to the scene.
FbxNode* CreateCube(FbxScene* pScene, const char* pName)
{
    int i, j;
    FbxMesh* lMesh = FbxMesh::Create(pScene,pName);

    FbxVector4 lControlPoint0(-50, 0,   50);
    FbxVector4 lControlPoint1(50,  0,   50);
    FbxVector4 lControlPoint2(50,  100, 50);
    FbxVector4 lControlPoint3(-50, 100, 50);
    FbxVector4 lControlPoint4(-50, 0,   -50);
    FbxVector4 lControlPoint5(50,  0,   -50);
    FbxVector4 lControlPoint6(50,  100, -50);
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

    lControlPoints[0]  = lControlPoint0;
    lControlPoints[1]  = lControlPoint1;
    lControlPoints[2]  = lControlPoint2;
    lControlPoints[3]  = lControlPoint3;
    lControlPoints[4]  = lControlPoint1;
    lControlPoints[5]  = lControlPoint5;
    lControlPoints[6]  = lControlPoint6;
    lControlPoints[7]  = lControlPoint2;
    lControlPoints[8]  = lControlPoint5;
    lControlPoints[9]  = lControlPoint4;
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

    // Set the normals on Layer 0.
    FbxLayer* lLayer = lMesh->GetLayer(0);
    if (lLayer == NULL)
    {
        lMesh->CreateLayer();
        lLayer = lMesh->GetLayer(0);
    }

    // We want to have one normal for each vertex (or control point),
    // so we set the mapping mode to eByControlPoint.
    FbxLayerElementNormal* lLayerElementNormal= FbxLayerElementNormal::Create(lMesh, "");

    lLayerElementNormal->SetMappingMode(FbxLayerElement::eByControlPoint);

    // Set the normal values for every control point.
    lLayerElementNormal->SetReferenceMode(FbxLayerElement::eDirect);

    lLayerElementNormal->GetDirectArray().Add(lNormalZPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalZPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalZPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalZPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalXPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalXPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalXPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalXPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalZNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalZNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalZNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalZNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalXNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalXNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalXNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalXNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalYPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalYPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalYPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalYPos);
    lLayerElementNormal->GetDirectArray().Add(lNormalYNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalYNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalYNeg);
    lLayerElementNormal->GetDirectArray().Add(lNormalYNeg);

    lLayer->SetNormals(lLayerElementNormal);

    // Array of polygon vertices.
    int lPolygonVertices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,12, 13, 
        14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };


    // Create UV for Diffuse channel.
    FbxLayerElementUV* lUVDiffuseLayer = FbxLayerElementUV::Create(lMesh, "DiffuseUV");
    lUVDiffuseLayer->SetMappingMode(FbxLayerElement::eByPolygonVertex);
    lUVDiffuseLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);
    lLayer->SetUVs(lUVDiffuseLayer, FbxLayerElement::eTextureDiffuse);

    FbxVector2 lVectors0(0, 0);
    FbxVector2 lVectors1(1, 0);
    FbxVector2 lVectors2(1, 1);
    FbxVector2 lVectors3(0, 1);

    lUVDiffuseLayer->GetDirectArray().Add(lVectors0);
    lUVDiffuseLayer->GetDirectArray().Add(lVectors1);
    lUVDiffuseLayer->GetDirectArray().Add(lVectors2);
    lUVDiffuseLayer->GetDirectArray().Add(lVectors3);

    //Now we have set the UVs as eIndexToDirect reference and in eByPolygonVertex  mapping mode
    //we must update the size of the index array.
    lUVDiffuseLayer->GetIndexArray().SetCount(24);

    // Create polygons. Assign texture and texture UV indices.
    for(i = 0; i < 6; i++)
    {
        // all faces of the cube have the same texture
        lMesh->BeginPolygon(-1, -1, -1, false);

        for(j = 0; j < 4; j++)
        {
            // Control point index
            lMesh->AddPolygon(lPolygonVertices[i*4 + j]);  

            // update the index array of the UVs that map the texture to the face
            lUVDiffuseLayer->GetIndexArray().SetAt(i*4+j, j);
        }

        lMesh->EndPolygon ();
    }

    // Set material indices
    FbxLayerElementMaterial* lMaterialLayer = FbxLayerElementMaterial::Create(lMesh, "MaterialIndices");
    lMaterialLayer->SetMappingMode(FbxLayerElement::eByPolygon);
    lMaterialLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);
    lLayer->SetMaterials(lMaterialLayer);

    for (i = 0; i < 6; i++)
    {
        // the i-th material on FbxNode is applied to the i-th polygon
        lMaterialLayer->GetIndexArray().Add(i);
    }

    // create a FbxNode
    FbxNode* lNode = FbxNode::Create(pScene,pName);

    // set the node attribute
    lNode->SetNodeAttribute(lMesh);

    // set the shading mode to view texture
    lNode->SetShadingMode(FbxNode::eTextureShading);

    // apply materials to the polygons of this node
    lNode->AddMaterial(gMatRed);
    lNode->AddMaterial(gMatGreen);
    lNode->AddMaterial(gMatBlue);
    lNode->AddMaterial(gMatRed);
    lNode->AddMaterial(gMatGreen);
    lNode->AddMaterial(gMatBlue);

    // rescale the cube
    lNode->LclScaling.Set(FbxVector4(0.3, 0.3, 0.3));

	// Add node to the scene
    pScene->GetRootNode()->AddChild(lNode);

    // return the FbxNode
    return lNode;
}

// Create a cube instance with the given mesh as node attribute, and add it to the scene.
FbxNode* CreateCubeInstance(FbxScene* pScene, const char* pName, FbxMesh* pFirstCube)
{
    // create a FbxNode
    FbxNode* lNode = FbxNode::Create(pScene,pName);

    // set the node attribute
    lNode->SetNodeAttribute(pFirstCube);

    // set the shading mode to view texture
    lNode->SetShadingMode(FbxNode::eTextureShading);

    // apply materials to the polygons of this node
    if (pFirstCube->GetNodeCount() % 2 == 0)
    {
        lNode->AddMaterial(gMatWhite);
        lNode->AddMaterial(gMatWhite);
        lNode->AddMaterial(gMatWhite);
        lNode->AddMaterial(gMatGray);
        lNode->AddMaterial(gMatGray);
        lNode->AddMaterial(gMatGray);
    }
    else
    {
        lNode->AddMaterial(gMatRed);
        lNode->AddMaterial(gMatGreen); 
        lNode->AddMaterial(gMatBlue);
        lNode->AddMaterial(gMatRed);
        lNode->AddMaterial(gMatGreen);
        lNode->AddMaterial(gMatBlue);
    }

    // rescale the cube
    lNode->LclScaling.Set(FbxVector4(0.3, 0.3, 0.3));

	// Add node to the scene
    pScene->GetRootNode()->AddChild(lNode);

    // return the FbxNode
    return lNode;
}

// Create a material that will be applied to a polygon
FbxSurfacePhong* CreateMaterial(FbxScene* pScene, FbxDouble3 pColor)
{
    // Create material
    FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene, "Polygon Material");

    // Set its diffuse color
    lMaterial->Diffuse.Set(pColor);

    return lMaterial;
}

// Create a single animation curve that will be used by all cubes.
FbxAnimCurve* CreateAnimCurve(FbxScene* pScene)
{
    FbxTime lTime;
    int lKeyIndex = 0;

	// Create curve
	FbxAnimCurve* lAnimCurve = FbxAnimCurve::Create(pScene, "Cube Animation");

	// Add keys to the curve
    lAnimCurve->KeyModifyBegin();

	// First key: time 0, value 0
    lTime.SetSecondDouble(0.0);
    lKeyIndex = lAnimCurve->KeyAdd(lTime);
    lAnimCurve->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationLinear);

	// Second key: time 20s, value -3600
	// Since this curve will describe rotation, each cube will rotate 10 times around itself during 20 seconds.
    lTime.SetSecondDouble(20.0);
    lKeyIndex = lAnimCurve->KeyAdd(lTime);
    lAnimCurve->KeySet(lKeyIndex, lTime, -3600, FbxAnimCurveDef::eInterpolationLinear);

	// Done adding keys.
    lAnimCurve->KeyModifyEnd();

	return lAnimCurve;
}

