/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// Illustrates how to get/set pivots, how to convert pivots
// and how to query local and global transform.
// 
//Steps:
//  1. Initialize FBX SDK object.
//  2. Create default animation stack and animation layer.
//  3. Create a pyramid mesh and attach it to a node.
//  4. Set pivots.
//  5. Add animation to the pyramid node.
//  6. Evaluate the local and global transform.
//  7. Save the scene before pivot converting.
//  8. Convert the animation to reset pivots.
//  9. Save the scene after pivot converting.
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>
#include "../Common/Common.h"
#include "../Common/AnimationUtility.h"
#include "../Common/GeometryUtility.h"

const char * SAMPLE_FILENAME_BEFORE_CONVECTION = "pivot_before_convection.fbx";
const char * SAMPLE_FILENAME_AFTER_CONVECTION = "pivot_after_convection.fbx";

const char * PYRAMID_NAME = "Pyramid";

const double KEY_TIME[] = {0.0, 0.5, 1.0};
const float KEY_VALUE[] = {0.0, 90.0, 180.0};

int main(int /*argc*/, char** /*argv*/)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create a animation stack and layer for the scene
    FbxAnimStack * lAnimStack;
    FbxAnimLayer * lAnimLayer = CreateDefaultAnimStackAndLayer(lScene, lAnimStack);

    // Create a pyramid mesh whose bottom width is 4 and height is 4.
    // Attach this pyramid to a node and as a child of the root node of the scene
    FbxNode * lPyramidNode = CreatePyramid(lScene, PYRAMID_NAME, 4, 4);

    // Enable pivot
    lPyramidNode->SetRotationActive(true);
    // Set the rotation pivot at the center of the pyramid
    lPyramidNode->SetRotationPivot(FbxNode::eSourcePivot, FbxVector4(0, 2, 0));
    // Add a post rotation for the pyramid
    lPyramidNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, -90));

    // Animate the Y channel of the local rotation
    FbxAnimCurve * lAnimCurve = lPyramidNode->LclRotation.GetCurve(lAnimLayer, "Y", true);
    if (lAnimCurve)
    {
        const int lKeyCount = sizeof(KEY_TIME)/sizeof(double);
        for (int lKeyIndex = 0; lKeyIndex < lKeyCount; ++lKeyIndex)
        {
            FbxTime lTime;
            FbxAnimCurveKey lKey;
            lTime.SetSecondDouble(KEY_TIME[lKeyIndex]);
            lKey.Set(lTime, KEY_VALUE[lKeyIndex]);
            lAnimCurve->KeyAdd(lTime, lKey);
        }
    }

    // Query the local transform and global transform of the pyramid node at 0.5 second
    FbxTime lTime;
    lTime.SetSecondDouble(0.5);
    FbxAMatrix lLocalTransform = lPyramidNode->EvaluateLocalTransform(lTime);
    FbxAMatrix lGlobalTransform = lPyramidNode->EvaluateGlobalTransform(lTime);

    // Save the scene before pivot converting
    bool lResult = SaveScene(lSdkManager, lScene, SAMPLE_FILENAME_BEFORE_CONVECTION);

    // Set the target of pivot converting
    // Reset the rotation pivot and post rotation, and maintain the animation
    lPyramidNode->SetPivotState(FbxNode::eSourcePivot, FbxNode::ePivotActive);
    lPyramidNode->SetPivotState(FbxNode::eDestinationPivot, FbxNode::ePivotActive);
    lPyramidNode->SetPostRotation(FbxNode::eDestinationPivot, FbxVector4(0, 0, 0));
    lPyramidNode->SetRotationPivot(FbxNode::eDestinationPivot, FbxVector4(0, 0, 0));

    // Convert the animation between source pivot set and destination pivot set with a frame rate of 30 per second
	lScene->GetRootNode()->ConvertPivotAnimationRecursive(lAnimStack, FbxNode::eDestinationPivot, 30.0);

    // Copy the rotation pivot and post rotation from destination set to source set in order to save them in file
    lPyramidNode->SetRotationPivot(FbxNode::eSourcePivot, FbxVector4(0, 0, 0));
    lPyramidNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, 0));

    // Save the scene after pivot converting
    lResult = SaveScene(lSdkManager, lScene, SAMPLE_FILENAME_AFTER_CONVECTION);

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);
    return 0;
}
