/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The example illustrates how to:
//        1) get a stereo camera from a scene
//        2) evaluate left/right camera
//        3) get local/global matrix of left and right camera
//        4) create stereo camera set
//        5) connect left/right and stereo
//
//steps:
// 1. initialize FBX sdk object.
// 2. load fbx scene form the specified file.
// 3. Get root node of the scene.
// 4. Recursively traverse each node in the scene.
// 5. Detect and get stereo camera by from node attribute type.
// 6. Get left and right camera.
// 7. Evaluate left/right camera.
// 8. Get local/global matrix of left and right camera.
// 9. Create stereo camera set after extracting stereo camera.
// 10. Connect left/right and stereo.
// 11. Get the updated values via connections.
// 12. Destroy all objects.
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME "StereoCamera.fbx"

void PrintMatrix(const FbxAMatrix& pMatrix);

void GetStereoCameraInfo(FbxNode* pNode);

void CreateStereoCamera(FbxScene* pScene);

static bool gVerbose = true;

int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Load the scene.
    // The example can take a FBX file as an argument.
	FbxString lFilePath("");
	for( int i = 1, c = argc; i < c; ++i )
	{
		if( FbxString(argv[i]) == "-test" ) gVerbose = false;
		else if( lFilePath.IsEmpty() ) lFilePath = argv[i];
	}
	if( lFilePath.IsEmpty() ) lFilePath = SAMPLE_FILENAME;

	FBXSDK_printf("\n\nFile: %s\n\n", lFilePath.Buffer());
	lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while loading the scene...");
    }
    else 
    {
        if(!lScene)
        {
            FBX_ASSERT_NOW("null scene");
        }

        //get root node of the fbx scene
        FbxNode* lRootNode = lScene->GetRootNode();

        //This function illustrates how to get stereo camera info from scene.
        GetStereoCameraInfo(lRootNode);

        //create your own stereo camera set
        CreateStereoCamera(lScene);
    }

    //Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);
    return 0;
}

//This function illustrates how to get stereo camera info from scene.
void GetStereoCameraInfo(FbxNode* pNode)
{
    if(!pNode)
        return;

    //detect stereo camera by node attribute type
    if(pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCameraStereo)
    {
        //get stereo camera
        FbxCameraStereo* lStereoCamera = (FbxCameraStereo*)pNode->GetNodeAttribute();

        //once we get the FbxCameraStereo, its connection has already been done.
        //it's easy to get its left and right camera
        //get left camera
        FbxCamera* lLeftCamera = lStereoCamera->GetLeftCamera();
        FbxString lLeftName = lLeftCamera->GetName();
        FBXSDK_printf("left camera: %s\n", lLeftName.Buffer());
        //get right camera
        FbxCamera* lRightCamera = lStereoCamera->GetRightCamera();
        FbxString lRightName = lRightCamera->GetName();
        FBXSDK_printf("right camera: %s\n", lRightName.Buffer());

        //FBX SDK support reevaluation functionality
        //reevaluate left/right camera
        double lLeftOffsetX = lStereoCamera->ReevaluateLeftCameraFilmOffsetX();
        FBXSDK_printf("reevaluated left camera film offset: %f\n", lLeftOffsetX);
        double lLeftFbxOffsetX = lLeftCamera->FilmOffsetX.Get();
        FBXSDK_printf("The original FBX left camera film offset: %f\n", lLeftFbxOffsetX);
        double lRightOffsetX = lStereoCamera->ReevaluateRightCameraFilmOffsetX();
        FBXSDK_printf("reevaluated right camera film offset: %f\n", lRightOffsetX);
        double lRightFbxOffsetX = lRightCamera->FilmOffsetX.Get();
        FBXSDK_printf("The original FBX right camera film offset: %f\n", lRightFbxOffsetX);

        //get local/global matrix of left and right camera
        //Then you can get their Translate, Rotate and Scale info.
        FbxAMatrix lLeft_localMatrix = lStereoCamera->GetLeftCameraLocalMatrix();
        FbxAMatrix lRight_localMatrix = lStereoCamera->GetRightCameraLocalMatrix();
        FbxAMatrix lLeft_globalMatrix = lStereoCamera->GetLeftCameraGlobalMatrix();
        FbxAMatrix lRight_globalMatrix = lStereoCamera->GetRightCameraGlobalMatrix();

        //print the local and global TRS for left camera
        FBXSDK_printf("===local TRS of left camera===\n");
        PrintMatrix(lLeft_localMatrix);
        FBXSDK_printf("===global TRS of left camera===\n");
        PrintMatrix(lLeft_globalMatrix);

        //print the local and global TRS for right camera
        FBXSDK_printf("===local TRS of right camera===\n");
        PrintMatrix(lRight_localMatrix);
        FBXSDK_printf("===global TRS of right camera===\n");
        PrintMatrix(lRight_globalMatrix);
    }// end if pNode->GetNodeAttribute()


    //recursively traverse each node in the scene
    int i, lCount = pNode->GetChildCount();
    for (i = 0; i < lCount; i++)
    {
        GetStereoCameraInfo(pNode->GetChild(i));
    }
}

//print the TRS for the given matrix
void PrintMatrix(const FbxAMatrix& pMatrix)
{
	if( !gVerbose ) return;

    //print the TRS
    FBXSDK_printf(" T : %f %f %f %f\n", 
        pMatrix.GetT()[0], 
        pMatrix.GetT()[1], 
        pMatrix.GetT()[2], 
        pMatrix.GetT()[3] );

    FBXSDK_printf(" R : %f %f %f %f\n", 
        pMatrix.GetR()[0], 
        pMatrix.GetR()[1], 
        pMatrix.GetR()[2], 
        pMatrix.GetR()[3] );

    FBXSDK_printf(" S : %f %f %f %f\n", 
        pMatrix.GetS()[0], 
        pMatrix.GetS()[1], 
        pMatrix.GetS()[2], 
        pMatrix.GetS()[3] );
}

//This function illustrates how to create and connect stereo camera.
void CreateStereoCamera(FbxScene* pScene)
{
    if(!pScene)
        return;

    //create a fbx node for stereo camera
    FbxNode* lMyStereoNode = FbxNode::Create(pScene,"myStereoNode");
    //create a cameraStereo, it's a node attribute of stereo camera node.
    FbxCameraStereo* lMyStereoCamera = FbxCameraStereo::Create(pScene,"myStereoCamera");
    //set stereoCamera as a node attribute of the FBX node.
    lMyStereoNode->SetNodeAttribute (lMyStereoCamera);
    //create a camera(node attribute), it will be left camera of stereo
    FbxCamera* lLeftCamera = FbxCamera::Create(pScene, "leftCamera");
    //create a camera(node attribute), it will be right camera of stereo
    FbxCamera* lRightCamera = FbxCamera::Create(pScene, "rightCamera");
    //add left camera to stereo
    lMyStereoCamera->SetLeftCamera(lLeftCamera);
    //add right camera to stereo
    lMyStereoCamera->SetRightCamera(lRightCamera);

    //During FBXSDK reevaluating, if ConnectProperties() is called, 
    //left and right camera property will be connected to stereo camera.
    //It's used to connect the left/right camera property [FocalLength, FarPlane, NearPlane, FilmWidth,
    //FilmHeight, FilmSqueezeRatio] to stereo camera.
    //If these properties of stereo camera have been modified by SDK, 
    //FBX will not automatically sync and update the corresponding properties of left/right camera.
    //However, you could get the newest property of left/right camera since ConnectProperties() is called.
    //To get the newest property value, please use lLeft_Camera->FocalLength.GetSrcProperty().Get(&lNewValue, ...);
    //Then you can update your left/right camera properties, for example, lLeftCamera->FocalLength.Set(lNewValue);
    lMyStereoCamera->ConnectProperties();

    //test the connection
    //get the focal length value of left camera.
    double lFocalLength_Left = lLeftCamera->FocalLength.Get();
    FBXSDK_printf("FocalLength of left camera: %f\n", lFocalLength_Left);
    double lFocalLength_Left_src = 0;
    //get source property of left focal length, it should be stereo focal length.
    FbxProperty lLeftSrcLengthProperty = lLeftCamera->FocalLength.GetSrcProperty();
    if(lLeftSrcLengthProperty.IsValid())
    {
        lFocalLength_Left_src = lLeftSrcLengthProperty.Get<FbxDouble>();
        FBXSDK_printf("Initialized FocalLength of left camera source:   %f\n", lFocalLength_Left_src);
        //modify the FocalLength of stereo camera
        lMyStereoCamera->FocalLength.Set(3.333);
        //get the FocalLength of left camera. It's 34.89 now. 
        //But it should be updated to 3.33 since stereo FocalLength has changed.
        //FBX doesn't sync it from stereo camera
        lFocalLength_Left = lLeftCamera->FocalLength.Get();
        FBXSDK_printf("FocalLength of left camera:   %f\n", lFocalLength_Left);
        //get the newest value from connected property.
        lFocalLength_Left_src = lLeftSrcLengthProperty.Get<FbxDouble>();
        FBXSDK_printf("FocalLength of left camera source:   %f\n", lFocalLength_Left_src);
    }
}


