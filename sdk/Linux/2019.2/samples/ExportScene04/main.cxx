/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The scene created in this example is a group of lights, a marker and
// a camera. An animation stack rotates the lights and moves the camera 
// around.
//
// The example illustrates how to:
//        1) create a light and it assign a gobo
//        2) set global light settings
//        3) create a marker
//        4) create a camera and link it to a point of interest
//        5) create an animation stack
//        6) export a scene in a .FBX file
//
/////////////////////////////////////////////////////////////////////////

#include <math.h>

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME "ExportScene04.fbx"


// Function prototypes.
bool CreateScene(FbxScene* pScene);

FbxNode* CreateLightGroup(FbxScene* pScene, const char* pName);
FbxNode* CreateLight(FbxScene* pScene, const char* pName);
FbxNode* CreateMarker(FbxScene* pScene, const char* pName);
FbxNode* CreateCamera(FbxScene* pScene, const char* pName);

void SetCameraPointOfInterest(FbxNode* pCamera, FbxNode* pPointOfInterest);

void SetLightGroupDefaultPosition(FbxNode* pLightGroup);
void SetLightDefaultPosition(FbxNode* pLight, int pIndex);
void SetMarkerDefaultPosition(FbxNode* pMarker);
void SetCamera1DefaultPosition(FbxNode* pCamera);
void SetCamera2DefaultPosition(FbxNode* pCamera);

void AnimateLightGroup(FbxNode* pLightGroup, FbxAnimLayer* pAnimLayer);
void AnimateLight(FbxNode* pLight, int pIndex, FbxAnimLayer* pAnimLayer);
void AnimateCamera(FbxNode* pLightGroup, FbxAnimLayer* pAnimLayer);


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
    FbxNode* lLightGroup = CreateLightGroup(pScene, "LightGroup");
    FbxNode* lMarker = CreateMarker(pScene, "Marker");
    FbxNode* lCamera1 = CreateCamera(pScene, "Camera1");
    FbxNode* lCamera2 = CreateCamera(pScene, "Camera2");

    pScene->GetGlobalSettings().SetAmbientColor(FbxColor(1.0, 0.5, 0.2));

    SetCameraPointOfInterest(lCamera1, lMarker);
    SetCameraPointOfInterest(lCamera2, lCamera1);

    SetLightGroupDefaultPosition(lLightGroup);
    SetMarkerDefaultPosition(lMarker);
    SetCamera1DefaultPosition(lCamera1);
    SetCamera2DefaultPosition(lCamera2);

    // Create the Animation Stack
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, "Rotating lights");

        // The animation nodes can only exist on AnimLayers therefore it is mandatory to
        // add at least one AnimLayer to the AnimStack. And for the purpose of this example,
        // one layer is all we need.
        FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
        lAnimStack->AddMember(lAnimLayer);

    // Build the scene graph.
    FbxNode* lRootNode = pScene->GetRootNode();
    lRootNode->AddChild(lLightGroup);
    lRootNode->AddChild(lMarker);
    lRootNode->AddChild(lCamera1);
    lCamera1->AddChild(lCamera2);


    // Set perspective camera as the default camera.
    pScene->GetGlobalSettings().SetDefaultCamera(FBXSDK_CAMERA_PERSPECTIVE);

    AnimateLightGroup(lLightGroup, lAnimLayer);
    AnimateCamera(lCamera1, lAnimLayer);

    return true;
}

// Create 6 lights and set global light settings.
FbxNode* CreateLightGroup(FbxScene* pScene, const char* pName)
{
    FbxString lLightName;
    FbxNode* lGroup = NULL;
    FbxNode* lNode = NULL;
    FbxLight* lLight = NULL;
    int i;

    lGroup = FbxNode::Create(pScene,pName);

    for(i = 0; i < 6; i++)
    {
        lLightName = pName;
        lLightName += "-Light";
        lLightName += i;

        lNode = CreateLight(pScene, lLightName.Buffer());
        lGroup->AddChild(lNode);
    }

    for (i = 0; i < 6; i++)
    {
        lLight = (FbxLight*) lGroup->GetChild(i)->GetNodeAttribute();
        lLight->FileName.Set("gobo.tif");// Resource file is in current directory.
        lLight->DrawGroundProjection.Set(true);
        lLight->DrawVolumetricLight.Set(true);
        lLight->DrawFrontFacingVolumetricLight.Set(false);
    }

    return lGroup;
}

// Create a spotlight. 
FbxNode* CreateLight(FbxScene* pScene, const char* pName)
{
    FbxLight* lLight = FbxLight::Create(pScene,pName);

    lLight->LightType.Set(FbxLight::eSpot);
    lLight->CastLight.Set(true);

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lLight);

    return lNode;
}

// Create a marker to use a point of interest for the camera. 
FbxNode* CreateMarker(FbxScene* pScene, const char* pName)
{
    FbxMarker* lMarker = FbxMarker::Create(pScene,pName);

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lMarker);

    return lNode;
}

FbxNode* CreateCamera(FbxScene* pScene, const char* pName)
{
    FbxCamera* lCamera = FbxCamera::Create(pScene,pName);

    // Modify some camera default settings.
    lCamera->SetApertureMode(FbxCamera::eVertical);
    lCamera->SetApertureWidth(0.816);
    lCamera->SetApertureHeight(0.612);
    lCamera->SetSqueezeRatio(0.5);

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lCamera);

    return lNode;
}

void SetCameraPointOfInterest(FbxNode* pCamera, FbxNode* pPointOfInterest)
{
    // Set the camera to always point at this node.
    pCamera->SetTarget(pPointOfInterest);
}

// The light group is just over the XZ plane.
void SetLightGroupDefaultPosition(FbxNode* pLightGroup)
{
    int i;

    for (i = 0; i < pLightGroup->GetChildCount(); i++)
    {
        SetLightDefaultPosition(pLightGroup->GetChild(i), i);
    }

    pLightGroup->LclTranslation.Set(FbxVector4(0.0, 15.0, 0.0));
    pLightGroup->LclRotation.Set(FbxVector4(0.0, 0.0, 0.0));
    pLightGroup->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));
}

void SetLightDefaultPosition(FbxNode* pLight, int pIndex)
{
    // Set light location depending of it's index.
    pLight->LclTranslation.Set(FbxVector4((cos((double)pIndex) * 40.0), 0.0, (sin((double)pIndex) * 40.0)));
    pLight->LclRotation.Set(FbxVector4(20.0, (90.0 - pIndex * 60.0), 0.0));
    pLight->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));

    // Set light attributes depending of it's index.
    FbxDouble3 lColor[6] = 
    {
        FbxDouble3(1.0, 0.0, 0.0), 
        FbxDouble3(1.0, 1.0, 0.0), 
        FbxDouble3(0.0, 1.0, 0.0), 
        FbxDouble3(0.0, 1.0, 1.0), 
        FbxDouble3(0.0, 0.0, 1.0), 
        FbxDouble3(1.0, 0.0, 1.0)
    };

    FbxLight* light = pLight->GetLight();
    if (light)
    {
        light->Color.Set(lColor[pIndex % 6]);
        light->Intensity.Set(33.0);
        light->OuterAngle.Set(90.0);
        light->Fog.Set(100.0);
    }
}

void SetMarkerDefaultPosition(FbxNode* pMarker)
{
    // The marker is at the origin.
    pMarker->LclTranslation.Set(FbxVector4(0.0, 0.0, 0.0));
    pMarker->LclRotation.Set(FbxVector4(0.0, 0.0, 0.0));
    pMarker->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));
}

// The code below shows how to compute the camera rotation.
// In the present case, it wouldn't be necessary since the
// camera is set to point to the marker. 
void SetCamera1DefaultPosition(FbxNode* pCamera)
{
    FbxVector4 lCameraLocation(0.0, 100.0, -300.0);
    FbxVector4 lDefaultPointOfInterest(1.0, 100.0, -300.0);
    FbxVector4 lNewPointOfInterest(0, 0, 0);
    FbxVector4 lRotation;
    FbxVector4 lScaling(1.0, 1.0, 1.0);

    FbxVector4::AxisAlignmentInEulerAngle(lCameraLocation, lDefaultPointOfInterest, lNewPointOfInterest, lRotation);

    pCamera->LclTranslation.Set(lCameraLocation);
    pCamera->LclRotation.Set(lRotation);
    pCamera->LclScaling.Set(lScaling);
}

void SetCamera2DefaultPosition(FbxNode* pCamera)
{
    pCamera->LclTranslation.Set(FbxVector4(-150.0, 0.0, 75.0));
}

// The light group rises and rotates.
void AnimateLightGroup(FbxNode* pLightGroup, FbxAnimLayer* pAnimLayer)
{
    FbxAnimCurve* lCurve = NULL;
    FbxTime lTime;
    int i;
    int lKeyIndex = 0;

    for (i = 0; i < pLightGroup->GetChildCount(); i++)
    {
        AnimateLight(pLightGroup->GetChild(i), i, pAnimLayer);
    }

    // Create the CurveNodes (they are necessary for the GetCurve to successfully allocate the Animation curve)
    pLightGroup->LclRotation.GetCurveNode(pAnimLayer, true);
    pLightGroup->LclTranslation.GetCurveNode(pAnimLayer, true);

    // Y axis rotation.
    lCurve = pLightGroup->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);

        lTime.SetSecondDouble(10.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 5*360.0);

        lCurve->KeyModifyEnd();
    }

    // Y axis translation.
    lCurve = pLightGroup->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 15.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(5.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 200.0);

        lCurve->KeyModifyEnd();
    }
}

// The lights are changing color, intensity, orientation and cone angle.
void AnimateLight(FbxNode* pLight, int pIndex, FbxAnimLayer* pAnimLayer)
{
    FbxAnimCurve* lCurve = NULL;
    FbxTime lTime;
    int i, j;
    int lKeyIndex = 0;

    FbxLight* light = pLight->GetLight();

    // Intensity fade in/out.
    // Create the CurveNode (it is necessary for the GetCurve to successfully allocate the Animation curve)
    light->Intensity.GetCurveNode(pAnimLayer, true);
    lCurve = light->Intensity.GetCurve(pAnimLayer, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(3.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 33.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);

        lTime.SetSecondDouble(7.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 33.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(10.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);

        lCurve->KeyModifyEnd();
    }

    // Fog fade in/out
    // Create the CurveNode (it is necessary for the GetCurve to successfully allocate the Animation curve)
    light->Fog.GetCurveNode(pAnimLayer, true);
    lCurve = light->Fog.GetCurve(pAnimLayer, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(3.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 33.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);

        lTime.SetSecondDouble(7.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 33.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(10.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);

        lCurve->KeyModifyEnd();
    }

    // X rotation swoops & cone angle woobles.
    {
        // Create the CurveNodes (they are necessary for the GetCurve to successfully allocate the Animation curve)
        pLight->LclRotation.GetCurveNode(pAnimLayer, true);
        light->OuterAngle.GetCurveNode(pAnimLayer, true);

        lCurve = pLight->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
        FbxAnimCurve* lConeCurve = light->OuterAngle.GetCurve(pAnimLayer,true);
        double lValue;

        lCurve->KeyModifyBegin();
        lConeCurve->KeyModifyBegin();

        for (i = 0; i < 8; i++)
        {
            lTime.SetSecondDouble((double)i * 0.833333);
            lValue = cos((((double)i) + (((double)pIndex) * 60.0)) * 72.0);

            lKeyIndex = lCurve->KeyAdd(lTime);
            lCurve->KeySetValue(lKeyIndex, float((lValue - 0.4) * 30.0));
            lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
            lKeyIndex = lConeCurve->KeyAdd(lTime);
            lConeCurve->KeySetValue(lKeyIndex, float((2.0 - (lValue + 1.0)) * 45.0));
            lConeCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
        }

        // Finally, have the lights spread out and lose focus.
        lTime.SetSecondDouble(10.0);

        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, -90.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lKeyIndex = lConeCurve->KeyAdd(lTime);
        lConeCurve->KeySetValue(lKeyIndex, 180.0);
        lConeCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);

        lCurve->KeyModifyEnd();
        lConeCurve->KeyModifyEnd();
    }

    // Color cycling.
    {
        FbxDouble3 lColor[6] = 
        {
            FbxDouble3(1.0, 0.0, 0.0), 
            FbxDouble3(1.0, 1.0, 0.0), 
            FbxDouble3(0.0, 1.0, 0.0), 
            FbxDouble3(0.0, 1.0, 1.0), 
            FbxDouble3(0.0, 0.0, 1.0), 
            FbxDouble3(1.0, 0.0, 1.0)
        };

        FbxAnimCurve* lCurveA[3];
        // Create the CurveNodes (they are necessary for the GetCurve to successfully allocate the Animation curve)
        light->Color.GetCurveNode(pAnimLayer, true);
        lCurveA[0] = light->Color.GetCurve(pAnimLayer,FBXSDK_CURVENODE_COLOR_RED, true);
        lCurveA[1] = light->Color.GetCurve(pAnimLayer,FBXSDK_CURVENODE_COLOR_GREEN, true);
        lCurveA[2] = light->Color.GetCurve(pAnimLayer,FBXSDK_CURVENODE_COLOR_BLUE, true);

        if (lCurveA[0] && lCurveA[1] && lCurveA[2])
        {
            lCurveA[0]->KeyModifyBegin();
            lCurveA[1]->KeyModifyBegin();
            lCurveA[2]->KeyModifyBegin();

            for (i = 0; i < 24; i++)
            {
                j = i + pIndex;

                while (j > 5)
                {
                    j -= 6;
                }

                lTime.SetSecondDouble((double)i * 0.4166666);

                lKeyIndex = lCurveA[0]->KeyAdd(lTime);
                lCurveA[0]->KeySetValue(lKeyIndex, (float)lColor[j][0]);
                lCurveA[0]->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

                lKeyIndex = lCurveA[1]->KeyAdd(lTime);
                lCurveA[1]->KeySetValue(lKeyIndex, (float)lColor[j][1]);
                lCurveA[1]->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

                lKeyIndex = lCurveA[2]->KeyAdd(lTime);
                lCurveA[2]->KeySetValue(lKeyIndex, (float)lColor[j][2]);
                lCurveA[2]->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
            }

            lCurveA[0]->KeyModifyEnd();
            lCurveA[1]->KeyModifyEnd();
            lCurveA[2]->KeyModifyEnd();
        }
    }
}

// The camera is rising and rolling twice.
void AnimateCamera(FbxNode* pCamera, FbxAnimLayer* pAnimLayer)
{
    FbxAnimCurve* lCurve = NULL;
    FbxTime lTime;
    int lKeyIndex = 0;

    // Create the CurveNode (it is necessary for the GetCurve to successfully allocate the Animation curve)
    pCamera->LclTranslation.GetCurveNode(pAnimLayer, true);

    // X translation.
    lCurve = pCamera->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationLinear);

        lTime.SetSecondDouble(10.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySet(lKeyIndex, lTime, 200.0);

        lCurve->KeyModifyEnd();
    }

    // Y translation.
    lCurve = pCamera->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationLinear);

        lTime.SetSecondDouble(10.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySet(lKeyIndex, lTime, 300.0);

        lCurve->KeyModifyEnd();
    }

    // Camera roll.
    FbxCamera* cam = pCamera->GetCamera();
    // Create the CurveNode (it is necessary for the GetCurve to successfully allocate the Animation curve)
    cam->Roll.GetCurveNode(pAnimLayer, true);
    lCurve = cam->Roll.GetCurve(pAnimLayer, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();

        lTime.SetSecondDouble (0.0); 
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationLinear);

        lTime.SetSecondDouble(10.0); 
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySet(lKeyIndex, lTime, 2*360.0);

        lCurve->KeyModifyEnd();
    }    
}

