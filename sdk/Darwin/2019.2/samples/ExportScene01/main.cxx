/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The scene created in this example is a cylinder linked to a skeleton
// made of 2 segments. Two animation stacks show the influence of the
// skeleton segments over the cylinder.
//
// The example illustrates how to:
//        1) create a patch
//        2) create a skeleton segment
//        3) create a link
//        4) store the bind pose
//        5) store one arbitrary rest pose
//        6) create multiple animation stacks
//        7) create meta-data and add a thumbnail
//        8) export a scene in a .FBX file (ASCII mode)
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"
#include "Thumbnail.h"

#define SAMPLE_FILENAME "ExportScene01.fbx"


// Function prototypes.
bool CreateScene(FbxManager* pSdkManager, FbxScene* pScene);

FbxNode* CreatePatch(FbxScene* pScene, const char* pName);
FbxNode* CreateSkeleton(FbxScene* pScene, const char* pName);

void LinkPatchToSkeleton(FbxScene* pScene, FbxNode* pPatch, FbxNode* pSkeletonRoot);
void StoreBindPose(FbxScene* pScene, FbxNode* pPatch);
void StoreRestPose(FbxScene* pScene, FbxNode* pSkeletonRoot);
void AnimateSkeleton(FbxScene* pScene, FbxNode* pSkeletonRoot);
void AddThumbnailToScene(FbxScene* pScene);
void AddNodeRecursively(FbxArray<FbxNode*>& pNodeArray, FbxNode* pNode);

void SetXMatrix(FbxAMatrix& pXMatrix, const FbxMatrix& pMatrix);

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

bool CreateScene(FbxManager *pSdkManager, FbxScene* pScene)
{
    // create scene info
    FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(pSdkManager,"SceneInfo");
    sceneInfo->mTitle = "Example scene";
    sceneInfo->mSubject = "Illustrates the creation and animation of a deformed cylinder.";
    sceneInfo->mAuthor = "ExportScene01.exe sample program.";
    sceneInfo->mRevision = "rev. 1.0";
    sceneInfo->mKeywords = "deformed cylinder";
    sceneInfo->mComment = "no particular comments required.";

    // we need to add the sceneInfo before calling AddThumbNailToScene because
    // that function is asking the scene for the sceneInfo.
    pScene->SetSceneInfo(sceneInfo);

    AddThumbnailToScene(pScene);

    FbxNode* lPatch = CreatePatch(pScene, "Patch");
    FbxNode* lSkeletonRoot = CreateSkeleton(pScene, "Skeleton");


    // Build the node tree.
    FbxNode* lRootNode = pScene->GetRootNode();
    lRootNode->AddChild(lPatch);
    lRootNode->AddChild(lSkeletonRoot);

	// Store poses
    LinkPatchToSkeleton(pScene, lPatch, lSkeletonRoot);
    StoreBindPose(pScene, lPatch);
    StoreRestPose(pScene, lSkeletonRoot);

	// Animation
    AnimateSkeleton(pScene, lSkeletonRoot);

    return true;
}

// Create a cylinder centered on the Z axis. 
FbxNode* CreatePatch(FbxScene* pScene, const char* pName)
{
    FbxPatch* lPatch = FbxPatch::Create(pScene,pName);

    // Set patch properties.
    lPatch->InitControlPoints(4, FbxPatch::eBSpline, 7, FbxPatch::eBSpline);
    lPatch->SetStep(4, 4);
    lPatch->SetClosed(true, false);

    FbxVector4* lVector4 = lPatch->GetControlPoints();
    int i;

    for (i = 0; i < 7; i++) 
    {
        double lRadius = 15.0;
        double lSegmentLength = 20.0;
        lVector4[4*i + 0].Set(lRadius, 0.0, (i-3)*lSegmentLength);
        lVector4[4*i + 1].Set(0.0, -lRadius, (i-3)*lSegmentLength);
        lVector4[4*i + 2].Set(-lRadius, 0.0, (i-3)*lSegmentLength);
        lVector4[4*i + 3].Set(0.0, lRadius, (i-3)*lSegmentLength);
    }

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    // Rotate the cylinder along the X axis so the axis
    // of the cylinder is the same as the bone axis (Y axis)
    FbxVector4 lR(-90.0, 0.0, 0.0);
    lNode->LclRotation.Set(lR);
    lNode->SetNodeAttribute(lPatch);

    return lNode;
}

// Create a skeleton with 2 segments.
FbxNode* CreateSkeleton(FbxScene* pScene, const char* pName)
{
    // Create skeleton root. 
    FbxString lRootName(pName);
    lRootName += "Root";
    FbxSkeleton* lSkeletonRootAttribute = FbxSkeleton::Create(pScene, pName);
    lSkeletonRootAttribute->SetSkeletonType(FbxSkeleton::eRoot);
    FbxNode* lSkeletonRoot = FbxNode::Create(pScene,lRootName.Buffer());
    lSkeletonRoot->SetNodeAttribute(lSkeletonRootAttribute);    
    lSkeletonRoot->LclTranslation.Set(FbxVector4(0.0, -40.0, 0.0));

    // Create skeleton first limb node. 
    FbxString lLimbNodeName1(pName);
    lLimbNodeName1 += "LimbNode1";
    FbxSkeleton* lSkeletonLimbNodeAttribute1 = FbxSkeleton::Create(pScene,lLimbNodeName1);
    lSkeletonLimbNodeAttribute1->SetSkeletonType(FbxSkeleton::eLimbNode);
    lSkeletonLimbNodeAttribute1->Size.Set(1.0);
    FbxNode* lSkeletonLimbNode1 = FbxNode::Create(pScene,lLimbNodeName1.Buffer());
    lSkeletonLimbNode1->SetNodeAttribute(lSkeletonLimbNodeAttribute1);    
    lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(0.0, 40.0, 0.0));

    // Create skeleton second limb node. 
    FbxString lLimbNodeName2(pName);
    lLimbNodeName2 += "LimbNode2";
    FbxSkeleton* lSkeletonLimbNodeAttribute2 = FbxSkeleton::Create(pScene,lLimbNodeName2);
    lSkeletonLimbNodeAttribute2->SetSkeletonType(FbxSkeleton::eLimbNode);
    lSkeletonLimbNodeAttribute2->Size.Set(1.0);
    FbxNode* lSkeletonLimbNode2 = FbxNode::Create(pScene,lLimbNodeName2.Buffer());
    lSkeletonLimbNode2->SetNodeAttribute(lSkeletonLimbNodeAttribute2);    
    lSkeletonLimbNode2->LclTranslation.Set(FbxVector4(0.0, 40.0, 0.0));

    // Build skeleton node hierarchy. 
    lSkeletonRoot->AddChild(lSkeletonLimbNode1);
    lSkeletonLimbNode1->AddChild(lSkeletonLimbNode2);

    return lSkeletonRoot;
}

// Set the influence of the skeleton segments over the cylinder.
// The link mode is FbxCluster::eTotalOne which means the total
// of the weights assigned to a given control point must equal 1.
void LinkPatchToSkeleton(FbxScene* pScene, FbxNode* pPatch, FbxNode* pSkeletonRoot)
{
    int i, j;
    FbxAMatrix lXMatrix;

    FbxNode* lRoot = pSkeletonRoot;
    FbxNode* lLimbNode1 = pSkeletonRoot->GetChild(0);
    FbxNode* lLimbNode2 = lLimbNode1->GetChild(0);

    // Bottom section of cylinder is clustered to skeleton root.
    FbxCluster *lClusterToRoot = FbxCluster::Create(pScene,"");
    lClusterToRoot->SetLink(lRoot);
    lClusterToRoot->SetLinkMode(FbxCluster::eTotalOne);
    for(i=0; i<4; ++i)
        for(j=0; j<4; ++j)
            lClusterToRoot->AddControlPointIndex(4*i + j, 1.0 - 0.25*i);

    // Center section of cylinder is clustered to skeleton limb node.
    FbxCluster* lClusterToLimbNode1 = FbxCluster::Create(pScene, "");
    lClusterToLimbNode1->SetLink(lLimbNode1);
    lClusterToLimbNode1->SetLinkMode(FbxCluster::eTotalOne);

    for (i =1; i<6; ++i)
        for (j=0; j<4; ++j)
            lClusterToLimbNode1->AddControlPointIndex(4*i + j, (i == 1 || i == 5 ? 0.25 : 0.50));


    // Top section of cylinder is clustered to skeleton limb.

    FbxCluster * lClusterToLimbNode2 = FbxCluster::Create(pScene,"");
    lClusterToLimbNode2->SetLink(lLimbNode2);
    lClusterToLimbNode2->SetLinkMode(FbxCluster::eTotalOne);

    for (i=3; i<7; ++i)
        for (j=0; j<4; ++j)
            lClusterToLimbNode2->AddControlPointIndex(4*i + j, 0.25*(i - 2));

    // Now we have the Patch and the skeleton correctly positioned,
    // set the Transform and TransformLink matrix accordingly.
	FbxScene* lScene = pPatch->GetScene();
    if( lScene ) lXMatrix = pPatch->EvaluateGlobalTransform();

    lClusterToRoot->SetTransformMatrix(lXMatrix);
    lClusterToLimbNode1->SetTransformMatrix(lXMatrix);
    lClusterToLimbNode2->SetTransformMatrix(lXMatrix);



    if( lScene ) lXMatrix = lRoot->EvaluateGlobalTransform();
    lClusterToRoot->SetTransformLinkMatrix(lXMatrix);


    if( lScene ) lXMatrix = lLimbNode1->EvaluateGlobalTransform();
    lClusterToLimbNode1->SetTransformLinkMatrix(lXMatrix);


    if( lScene ) lXMatrix = lLimbNode2->EvaluateGlobalTransform();
    lClusterToLimbNode2->SetTransformLinkMatrix(lXMatrix);


    // Add the clusters to the patch by creating a skin and adding those clusters to that skin.
    // After add that skin.

    FbxGeometry* lPatchAttribute = (FbxGeometry*) pPatch->GetNodeAttribute();
    FbxSkin* lSkin = FbxSkin::Create(pScene, "");
    lSkin->AddCluster(lClusterToRoot);
    lSkin->AddCluster(lClusterToLimbNode1);
    lSkin->AddCluster(lClusterToLimbNode2);
    lPatchAttribute->AddDeformer(lSkin);

}

// Create two animation stacks.
void AnimateSkeleton(FbxScene* pScene, FbxNode* pSkeletonRoot)
{
    FbxString lAnimStackName;
    FbxTime lTime;
    int lKeyIndex = 0;

    FbxNode* lRoot = pSkeletonRoot;
    FbxNode* lLimbNode1 = pSkeletonRoot->GetChild(0);

    // First animation stack.
    lAnimStackName = "Bend on 2 sides";
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, lAnimStackName);

        // The animation nodes can only exist on AnimLayers therefore it is mandatory to
        // add at least one AnimLayer to the AnimStack. And for the purpose of this example,
        // one layer is all we need.
        FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
        lAnimStack->AddMember(lAnimLayer);

    // Create the AnimCurve on the Rotation.Z channel
    FbxAnimCurve* lCurve = lRoot->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();
        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 45.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, -45.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(3.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
        lCurve->KeyModifyEnd();
    }

    // Same thing for the next object
    lCurve = lLimbNode1->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();
        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, -90.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 90.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(3.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
        lCurve->KeyModifyEnd();
    }

    // Second animation stack.
    lAnimStackName = "Bend and turn around";
    lAnimStack = FbxAnimStack::Create(pScene, lAnimStackName);

        // The animation nodes can only exist on AnimLayers therefore it is mandatory to
        // add at least one AnimLayer to the AnimStack. And for the purpose of this example,
        // one layer is all we need.
        lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
        lAnimStack->AddMember(lAnimLayer);

    // Create the AnimCurve on the Rotation.Y channel
    lCurve = lRoot->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();
        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 720.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
        lCurve->KeyModifyEnd();
    }

    lCurve = lLimbNode1->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
    if (lCurve)
    {
        lCurve->KeyModifyBegin();
        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 90.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurve->KeyAdd(lTime);
        lCurve->KeySetValue(lKeyIndex, 0.0);
        lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
        lCurve->KeyModifyEnd();
    }
}

// Add a thumbnail to the scene
void AddThumbnailToScene(FbxScene* pScene)
{
    FbxThumbnail* lThumbnail = FbxThumbnail::Create(pScene,"");

    lThumbnail->SetDataFormat(FbxThumbnail::eRGB_24);
    lThumbnail->SetSize(FbxThumbnail::e64x64);
    lThumbnail->SetThumbnailImage(cSceneThumbnail);

    if (pScene->GetSceneInfo())
    {
        pScene->GetSceneInfo()->SetSceneThumbnail(lThumbnail);
    }
}

// Store the Bind Pose
void StoreBindPose(FbxScene* pScene, FbxNode* pPatch)
{
    // In the bind pose, we must store all the link's global matrix at the time of the bind.
    // Plus, we must store all the parent(s) global matrix of a link, even if they are not
    // themselves deforming any model.

    // In this example, since there is only one model deformed, we don't need walk through 
    // the scene
    //

    // Now list the all the link involve in the patch deformation
    FbxArray<FbxNode*> lClusteredFbxNodes;
    int                       i, j;

    if (pPatch && pPatch->GetNodeAttribute())
    {
        int lSkinCount=0;
        int lClusterCount=0;
        switch (pPatch->GetNodeAttribute()->GetAttributeType())
        {
	    default:
	        break;
        case FbxNodeAttribute::eMesh:
        case FbxNodeAttribute::eNurbs:
        case FbxNodeAttribute::ePatch:

            lSkinCount = ((FbxGeometry*)pPatch->GetNodeAttribute())->GetDeformerCount(FbxDeformer::eSkin);
            //Go through all the skins and count them
            //then go through each skin and get their cluster count
            for(i=0; i<lSkinCount; ++i)
            {
                FbxSkin *lSkin=(FbxSkin*)((FbxGeometry*)pPatch->GetNodeAttribute())->GetDeformer(i, FbxDeformer::eSkin);
                lClusterCount+=lSkin->GetClusterCount();
            }
            break;
        }
        //if we found some clusters we must add the node
        if (lClusterCount)
        {
            //Again, go through all the skins get each cluster link and add them
            for (i=0; i<lSkinCount; ++i)
            {
                FbxSkin *lSkin=(FbxSkin*)((FbxGeometry*)pPatch->GetNodeAttribute())->GetDeformer(i, FbxDeformer::eSkin);
                lClusterCount=lSkin->GetClusterCount();
                for (j=0; j<lClusterCount; ++j)
                {
                    FbxNode* lClusterNode = lSkin->GetCluster(j)->GetLink();
                    AddNodeRecursively(lClusteredFbxNodes, lClusterNode);
                }

            }

            // Add the patch to the pose
            lClusteredFbxNodes.Add(pPatch);
        }
    }

    // Now create a bind pose with the link list
    if (lClusteredFbxNodes.GetCount())
    {
        // A pose must be named. Arbitrarily use the name of the patch node.
        FbxPose* lPose = FbxPose::Create(pScene,pPatch->GetName());

        // default pose type is rest pose, so we need to set the type as bind pose
        lPose->SetIsBindPose(true);

        for (i=0; i<lClusteredFbxNodes.GetCount(); i++)
        {
            FbxNode*  lKFbxNode   = lClusteredFbxNodes.GetAt(i);
            FbxMatrix lBindMatrix = lKFbxNode->EvaluateGlobalTransform();

            lPose->Add(lKFbxNode, lBindMatrix);
        }

        // Add the pose to the scene
        pScene->AddPose(lPose);
    }
}

// Store a Rest Pose
void StoreRestPose(FbxScene* pScene, FbxNode* pSkeletonRoot)
{
    // This example show an arbitrary rest pose assignment.
    // This rest pose will set the bone rotation to the same value 
    // as time 1 second in the first stack of animation, but the 
    // position of the bone will be set elsewhere in the scene.
    FbxString     lNodeName;
    FbxNode*   lKFbxNode;
    FbxMatrix  lTransformMatrix;
    FbxVector4 lT,lR,lS(1.0, 1.0, 1.0);

    // Create the rest pose
    FbxPose* lPose = FbxPose::Create(pScene,"A Bind Pose");

    // Set the skeleton root node to the global position (10, 10, 10)
    // and global rotation of 45deg along the Z axis.
    lT.Set(10.0, 10.0, 10.0);
    lR.Set( 0.0,  0.0, 45.0);

    lTransformMatrix.SetTRS(lT, lR, lS);

    // Add the skeleton root node to the pose
    lKFbxNode = pSkeletonRoot;
    lPose->Add(lKFbxNode, lTransformMatrix, false /*it's a global matrix*/);

    // Set the lLimbNode1 node to the local position of (0, 40, 0)
    // and local rotation of -90deg along the Z axis. This show that
    // you can mix local and global coordinates in a rest pose.
    lT.Set(0.0, 40.0,   0.0);
    lR.Set(0.0,  0.0, -90.0);

    lTransformMatrix.SetTRS(lT, lR, lS);

    // Add the skeleton second node to the pose
    lKFbxNode = lKFbxNode->GetChild(0);
    lPose->Add(lKFbxNode, lTransformMatrix, true /*it's a local matrix*/);

    // Set the lLimbNode2 node to the local position of (0, 40, 0)
    // and local rotation of 45deg along the Z axis.
    lT.Set(0.0, 40.0, 0.0);
    lR.Set(0.0,  0.0, 45.0);

    lTransformMatrix.SetTRS(lT, lR, lS);

    // Add the skeleton second node to the pose
    lKFbxNode = lKFbxNode->GetChild(0);
    lNodeName = lKFbxNode->GetName();
    lPose->Add(lKFbxNode, lTransformMatrix, true /*it's a local matrix*/);

    // Now add the pose to the scene
    pScene->AddPose(lPose);
}

// Add the specified node to the node array. Also, add recursively
// all the parent node of the specified node to the array.
void AddNodeRecursively(FbxArray<FbxNode*>& pNodeArray, FbxNode* pNode)
{
    if (pNode)
    {
        AddNodeRecursively(pNodeArray, pNode->GetParent());

        if (pNodeArray.Find(pNode) == -1)
        {
            // Node not in the list, add it
            pNodeArray.Add(pNode);
        }
    }
}

void SetXMatrix(FbxAMatrix& pXMatrix, const FbxMatrix& pMatrix)
{
    memcpy((double*)pXMatrix, &pMatrix.mData[0][0], sizeof(pMatrix.mData));
}
