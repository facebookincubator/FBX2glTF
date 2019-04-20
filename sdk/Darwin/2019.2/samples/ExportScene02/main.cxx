/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The scene created in this example is a sphere morphed by 2 shapes. A 
// animation stack shows the influence of the shapes over the sphere.
//
// The example illustrates how to:
//        1) create a nurbs
//        2) map a shape over a nurbs
//        3) map a texture over a nurbs on material channel Diffuse 
//           and Ambient
//        4) map a material over a nurbs
//        5) create an animation stack
//        6) export a scene in a .FBX file
//
/////////////////////////////////////////////////////////////////////////

#include <math.h>

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME "ExportScene02.fbx"


// Function prototypes.
bool CreateScene(FbxScene* pScene);

FbxNode* CreateNurbs(FbxScene* pScene, const char* pName);

void MapStretchedShape(FbxScene* pScene, FbxNode* pNurbs);
void MapBoxShape(FbxScene* pScene, FbxNode* pNurbs);
void MapShapesOnNurbs(FbxScene* pScene, FbxNode* pNurbs);
void MapTexture(FbxScene* pScene, FbxNode* pNurbs);
void MapMaterial(FbxScene* pScene, FbxNode* pNurbs);

void AnimateNurbs(FbxNode* pNurbs, FbxScene* pScene);


int main(int argc, char** argv)
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

bool CreateScene(FbxScene* pScene)
{
    FbxNode* lNurbs = CreateNurbs(pScene, "Nurbs");

	MapShapesOnNurbs(pScene, lNurbs);
    MapMaterial(pScene, lNurbs);
    MapTexture(pScene, lNurbs);

    // Build the node tree.
    FbxNode* lRootNode = pScene->GetRootNode();
    lRootNode->AddChild(lNurbs);

    AnimateNurbs(lNurbs, pScene);
    return true;
}

// Create a sphere. 
FbxNode* CreateNurbs(FbxScene* pScene, const char* pName)
{
    FbxNurbs* lNurbs = FbxNurbs::Create(pScene,pName);

    // Set nurbs properties.
    lNurbs->SetOrder(4, 4);
    lNurbs->SetStep(2, 2);
    lNurbs->InitControlPoints(8, FbxNurbs::ePeriodic, 7, FbxNurbs::eOpen);

    double lUKnotVector[] = { -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0 };
    memcpy(lNurbs->GetUKnotVector(), lUKnotVector, lNurbs->GetUKnotCount()*sizeof(double));

    double lVKnotVector[] = { 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 4.0, 4.0, 4.0 };
    memcpy(lNurbs->GetVKnotVector(), lVKnotVector, lNurbs->GetVKnotCount()*sizeof(double));

    FbxVector4* lVector4 = lNurbs->GetControlPoints();
    int i, j;
    double lScale = 20.0;
    double lPi = 3.14159;
    double lYAngle[] = { 90.0, 90.0, 52.0, 0.0, -52.0, -90.0, -90.0 };
    double lRadius[] = { 0.0, 0.283, 0.872, 1.226, 0.872, 0.283, 0.0}; 

    for (i = 0; i < 7; i++) 
    {
        for (j = 0; j < 8; j++)
        {
            double lX = lScale * lRadius[i] * cos(lPi/4*j);
            double lY = lScale * sin(2*lPi/360*lYAngle[i]);
            double lZ = lScale * lRadius[i] * sin(lPi/4*j);
            double lWeight = 1.0;

            lVector4[8*i + j].Set(lX, lY, lZ, lWeight);
        }
    }


    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lNurbs);

    return lNode;
}

// Map nurbs control points onto a stretched shape.
void MapStretchedShape(FbxScene* pScene, FbxBlendShapeChannel* lBlendShapeChannel)
{
    FbxShape* lShape = FbxShape::Create(pScene,"StretchedShape");

    FbxVector4 lExtremeRight(-250.0, 0.0, 0.0);
    FbxVector4 lExtremeLeft(250.0, 0.0, 0.0);

    lShape->InitControlPoints(8*7);

    FbxVector4* lVector4 = lShape->GetControlPoints();

    int i, j;

    for (i = 0; i < 7; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (j < 3 || j > 6)
            {
                lVector4[8*i + j] = lExtremeLeft;
            }
            else
            {
                lVector4[8*i + j] = lExtremeRight;
            }
        }
    }

	lBlendShapeChannel->AddTargetShape(lShape);
}

// Map nurbs control points onto a box shape.
void MapBoxShape(FbxScene* pScene, FbxBlendShapeChannel* lBlendShapeChannel)
{
    FbxShape* lShape = FbxShape::Create(pScene,"BoxShape");

    lShape->InitControlPoints(8*7);

    FbxVector4* lVector4 = lShape->GetControlPoints();

    int i, j;
    double lScale = 20.0;
    double lWeight = 1.0;
    double lX[] = { 0.9, 1.1, 0.0, -1.1, -0.9, -1.1, 0.0, 1.1 };
    double lZ[] = { 0.0, 1.1, 0.9, 1.1, 0.0, -1.1, -0.9, -1.1 };

    // Top control points.
    for (i = 0; i < 8; i++)
    {
        lVector4[i].Set(0.0, lScale, 0.0, lWeight);
    }

    // Middle control points.
    for (i = 1; i < 6; i++)
    {
        double lY = 1.0 - 0.5 * (i - 1);

        for (j = 0; j < 8; j++)
        {
            lVector4[8*i + j].Set(lScale*lX[j], lScale*lY, lScale*lZ[j], lWeight);
        }
    }

    // Bottom control points.
    for (i = 48; i < 56; i++)
    {
        lVector4[i].Set(0.0, -lScale, 0.0, lWeight);
    }

	lBlendShapeChannel->AddTargetShape(lShape);
}

void MapShapesOnNurbs(FbxScene* pScene, FbxNode* pNurbs)
{
	FbxBlendShape* lBlendShape = FbxBlendShape::Create(pScene,"MyBlendShape");
	FbxBlendShapeChannel* lBlendShapeChannel01 = FbxBlendShapeChannel::Create(pScene,"MyBlendShapeChannel01");
	FbxBlendShapeChannel* lBlendShapeChannel02 = FbxBlendShapeChannel::Create(pScene,"MyBlendShapeChannel02");

	//Create and add two target shapes on the lBlendShapeChannel.
	MapStretchedShape(pScene, lBlendShapeChannel01);
	MapBoxShape(pScene, lBlendShapeChannel01);	
	
	MapBoxShape(pScene, lBlendShapeChannel02);	

	//Set the lBlendShapeChannel on lBlendShape.
	lBlendShape->AddBlendShapeChannel(lBlendShapeChannel01);
	lBlendShape->AddBlendShapeChannel(lBlendShapeChannel02);

	//Set the lBlendShape on pNurbs.
	FbxGeometry* lGeometry = pNurbs->GetGeometry();
	lGeometry->AddDeformer(lBlendShape);
};

// Map texture over sphere.
void MapTexture(FbxScene* pScene, FbxNode* pNurbs)
{
    FbxFileTexture* lTexture = FbxFileTexture::Create(pScene,"scene02.jpg");

    // The texture won't be displayed if node shading mode isn't set to FbxNode::eTextureShading.
    pNurbs->SetShadingMode(FbxNode::eTextureShading);

    // Set texture properties.
    lTexture->SetFileName("scene02.jpg"); // Resource file is in current directory.
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eCylindrical);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.45, -0.05);
    lTexture->SetScale(4.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);

    // we connect the texture to the material DiffuseColor property
    FbxSurfacePhong* lMaterial = pNurbs->GetSrcObject<FbxSurfacePhong>(0);
    if (lMaterial)
        lMaterial->Diffuse.ConnectSrcObject(lTexture);

    //now, we can try to map a texture on the AMBIENT channel of a material.

    //It is important to create a NEW texture and not to simply change the
    //properties of lTexture.

    //Set the Texture properties
    lTexture=FbxFileTexture::Create(pScene, "grandient.jpg");
    lTexture->SetFileName("gradient.jpg");
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eCylindrical);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);

    // we connect the texture to the material Ambient property
    if (lMaterial)
        lMaterial->Ambient.ConnectSrcObject(lTexture);

}

// Map material over sphere.
void MapMaterial(FbxScene* pScene, FbxNode* pNurbs)
{
    FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene,"scene02");
    FbxDouble3 lBlue(0.0, 0.0, 1.0);
    FbxDouble3 lBlack(0.0, 0.0, 0.0);

    lMaterial->Emissive.Set(lBlue);
    lMaterial->Ambient.Set(lBlack);
    lMaterial->Specular.Set(lBlack);
    lMaterial->TransparencyFactor.Set(0.0);
    lMaterial->Shininess.Set(0.0);
    lMaterial->ReflectionFactor.Set(0.0);

    // Create GeometryElementMaterial
    FbxNurbs*						lNurbs					= pNurbs->GetNurbs();
    FbxGeometryElementMaterial*	lGeometryElementMaterial	= lNurbs->GetElementMaterial( 0);

    if (!lGeometryElementMaterial)
    {
        lGeometryElementMaterial = lNurbs->CreateElementMaterial();
    }

    // The material is mapped to the whole Nurbs
    lGeometryElementMaterial->SetMappingMode(FbxGeometryElement::eAllSame);

    // And the material is avalible in the Direct array
    lGeometryElementMaterial->SetReferenceMode(FbxGeometryElement::eDirect);
    pNurbs->AddMaterial(lMaterial);
}

// Morph sphere into box shape.
void AnimateNurbs(FbxNode* pNurbs, FbxScene* pScene)
{
    FbxString lAnimStackName;
    FbxTime lTime;
    int lKeyIndex = 0;

    // First animation stack.
    lAnimStackName = "Morph sphere into box";
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, lAnimStackName);

        // The animation nodes can only exist on AnimLayers therefore it is mandatory to
        // add at least one AnimLayer to the AnimStack. And for the purpose of this example,
        // one layer is all we need.
        FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
        lAnimStack->AddMember(lAnimLayer);

    FbxGeometry* lNurbsAttribute = (FbxGeometry*) pNurbs->GetNodeAttribute();

    // The stretched shape is at index 0 because it was added first to the nurbs.
    FbxAnimCurve* lCurve = lNurbsAttribute->GetShapeChannel(0, 0, lAnimLayer, true);
    if (lCurve)
    {
		lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 75.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.25);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lCurve->KeyModifyEnd();
    }

    // The box shape is at index 1 because it was added second to the nurbs.
    lCurve = lNurbsAttribute->GetShapeChannel(0, 1, lAnimLayer, true);
    if (lCurve)
    {
		lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(1.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.25);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 100.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lCurve->KeyModifyEnd();
    }
}


