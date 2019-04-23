/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
// Switch binding from one cylinder to the other one.
// Please check out the animation to see difference.
// Illustrates how to:
// 1. Get skin deformer and cluster;
// 2. Detach skin;
// 3. Bind and create corresponding bindpose;
//
// Steps:
//  1. Initialize FBX SDK Manager and FBX Scene
//  2. Load the input file to scene
//  3. Access the two cylinders
//  4. Get the skin deformer and all clusters from the first cylinder
//  5. Remove clusters, skin deformer and bind pose from the first cylinder
//  6. Move joints to proper position for cylinder2
//  7. Update clusters, skin deformer and create a bind pose for the second cylinder
//  8. Save the scene to output file
//  9. Destroy the FBX SDK Manager and FBX Scene
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>
#include "../Common/Common.h"

const char * SAMPLE_FILENAME_BEFORE_SWITCH = "Bind_Before_Switch.fbx";
const char * SAMPLE_FILENAME_AFTER_SWITCH = "Bind_After_Switch.fbx";

void SwitchBinding( FbxScene* pScene );

int main(int /*argc*/, char** /*argv*/)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;

	// Prepare the FBX SDK.
	InitializeSdkObjects(lSdkManager, lScene);	

	// Load the scene. 
	bool lResult = LoadScene(lSdkManager, lScene, SAMPLE_FILENAME_BEFORE_SWITCH);
	if( lResult )
	{
		//Switch binding from the first cylinder to the second one.
		SwitchBinding( lScene );

		//Save the scene after switching binding.
		lResult = SaveScene(lSdkManager, lScene, SAMPLE_FILENAME_AFTER_SWITCH);
	}

	// Destroy all objects created by the FBX SDK.
	DestroySdkObjects(lSdkManager, lResult);
    return 0;
}

void SwitchBinding(FbxScene * pScene)
{
	//Get the two cylinders.
	FbxMesh* lCylinder01 = pScene->GetSrcObject<FbxMesh>(0);
	FbxMesh* lCylinder02 = pScene->GetSrcObject<FbxMesh>(1);

	//Get the skin deformer, which will be switched from the first cylinder to the the second one.
	FbxSkin* lSkin = (FbxSkin *) lCylinder01->GetDeformer(0, FbxDeformer::eSkin);
	
	// Get all clusters through the skin deformer, later they will be re-used for the second cylinder.
	FbxArray<FbxCluster*> lClusterArray;
	for (int lClusterIndex = 0; lClusterIndex < lSkin->GetClusterCount(); ++lClusterIndex)
	{
		FbxCluster* lCluster=lSkin->GetCluster(lClusterIndex);
		lClusterArray.Add(lCluster);
	}

	// Detach the first cylinder.
	// Remove clusters from the skin deformer.
	for (int lClusterIndex = 0; lClusterIndex < lClusterArray.GetCount(); ++lClusterIndex)
		lSkin->RemoveCluster(lClusterArray[lClusterIndex]);
	// Remove the skin deformer from the first cylinder.
	// Currently, the only deformer connected to lCylinder01 is the skin deformer.
	lCylinder01->RemoveDeformer(0);
	// Remove the corresponding bindpose.
	pScene->RemovePose(0) ;

    // Bind the second cylinder and create the corresponding bind pose.
	// Move the joints into the second cylinder. Only need to move the root joint.
	lClusterArray[0]->GetLink()->LclTranslation.Set(FbxDouble3 ( -11.0322688253132,5.3883395780739,0) );

	// Prepare an array to collect nodes for bind pose, which is combined of the geometry and joints. 
	FbxArray<FbxNode*> lPoseNodeArray;

    // Add the second cylinder to node array of bind pose.
	lPoseNodeArray.Add(lCylinder02->GetNode());

	// Set proper transformation for each cluster and add them to the skin deformer.
	// ThansformMatrix is the global transformation of the mesh when the binding happens.
	// TransformLinkMatrix is the global transformation of the joint(Link) when the binding happens.
	FbxAMatrix lTransformMatrix, lTransformLinkMatrix;
	lTransformMatrix = lCylinder02->GetNode()->EvaluateGlobalTransform();
	for(int lClusterIndex = 0; lClusterIndex < lClusterArray.GetCount(); lClusterIndex++)
	{
		// All joints have the same TransformMatrix.
		lClusterArray[lClusterIndex]->SetTransformMatrix(lTransformMatrix);

		// Compute global transformation of each joint and set it as TransformLinkMatrix.
		lTransformLinkMatrix = lClusterArray[lClusterIndex]->GetLink()->EvaluateGlobalTransform();
		lClusterArray[lClusterIndex]->SetTransformLinkMatrix(lTransformLinkMatrix);

		// Add cluster to the skin deformer.
		lSkin->AddCluster(lClusterArray[lClusterIndex]);

		// Add each joint(Link) to node array of bindpose.
		lPoseNodeArray.Add(lClusterArray[lClusterIndex]->GetLink());
	}

	// Add the skin deformer to the second cylinder.
	lCylinder02->AddDeformer(lSkin);

	// Create a pose by the node array and set it as bindpose.
	FbxPose* lPose = FbxPose::Create(pScene,lCylinder02->GetNode()->GetName());
	lPose->SetIsBindPose(true);
	for(int lPoseNodeIndex = 0; lPoseNodeIndex<lPoseNodeArray.GetCount(); ++lPoseNodeIndex)
	{
		FbxMatrix lBindMatrix = lPoseNodeArray[lPoseNodeIndex]->EvaluateGlobalTransform();
		lPose->Add(lPoseNodeArray[lPoseNodeIndex], lBindMatrix);
	}

	// Add the bindpose to the scene.
	pScene->AddPose(lPose);
}
