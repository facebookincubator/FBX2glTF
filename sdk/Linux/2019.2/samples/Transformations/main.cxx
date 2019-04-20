/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Illustrates the followings:
// 1. How to use FbxAMatrix in transformation calculation.
// 2. What EvaluateGlobalTransform() and EvaluateLocalTransform() actually do.
// 3. How to get global and local transform of each joint in a joint hierarchy.
//
// Steps:
//  1. Initialize FBX SDK Manager and FBX Scene.
//  2. Load the input file to scene.
//  3. Calculate global and local transform by EvaluateGlobalTransform() and EvaluateLocalTransform().
//  4. Alternative way to calculate global and local transform from scratch by node's properties.
//  5. Compare, the above two results should be the same. 
//  6. Display the joint's global and local transformation.
//  7. Destroy the FBX SDK Manager and FBX Scene.
//
// Notice: This sample does not take rotation order into consideration. It only shows the transformation 
// calculation with XYZ rotation order.
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>
#include "../Common/Common.h"
#include "DisplayCommon.h"

const char * SAMPLE_FILENAME = "JointHierarchy.fbx";

FbxAMatrix CalculateGlobalTransform(FbxNode* pNode);
void CompareTransformations( FbxNode* pNode, FbxScene* pScene );

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
		if( FbxString(argv[i]) == "-test" ) continue;
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
        CompareTransformations( lScene->GetRootNode(), lScene );
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

    return 0;
}

void CompareTransformations(FbxNode* pNode, FbxScene * pScene)
{
    if( pNode != pScene->GetRootNode())
    {
        DisplayString(pNode->GetName());
        FbxNode* lParentNode = pNode->GetParent();

        // The first way: calculate global and local transform by EvaluateGlobalTransform() and EvaluateLocalTransform().
        FbxAMatrix lGlobal, lLocal;
		lGlobal= pNode->EvaluateGlobalTransform();
		lLocal = pNode->EvaluateLocalTransform();

        // The second way: calculate global and local transform from scratch by the node's properties.
        FbxAMatrix lParentTransform,lLocalTransform, lGlobalTransform;
        lGlobalTransform = CalculateGlobalTransform(pNode);
        if(lParentNode)
        {
            // Get parent global transform.
            lParentTransform = CalculateGlobalTransform(lParentNode);
            // Calculate local transform according to: LocalTransform = ParentGlobalInverse * GlobalTransform.
            lLocalTransform = lParentTransform.Inverse() * lGlobalTransform;
        }
        else
            lLocalTransform = lGlobalTransform;

        // Compare, the results are the same. Display the global and local transformation of each joint.
        if(lGlobal == lGlobalTransform)
        {
            for(int i = 0; i<4; ++i)
            {
                FbxString lHeader("GlobalTransform Row_");
                FbxString lIndex(i);
                lHeader += lIndex;
                lHeader += ": ";

                Display4DVector(lHeader, lGlobal.GetRow(i));
            }            
            FBXSDK_printf("\n");
        }
        else
        {
            FBXSDK_printf("Error: The two global transform results are not equal!\n");
            for(int i = 0; i<4; ++i)
            {
                FbxString lHeader("KFbxEvaluatorGlobalTransform Row_");
                FbxString lIndex(i);
                lHeader += lIndex;
                lHeader += ": ";

                Display4DVector(lHeader, lGlobal.GetRow(i));
            }            
            FBXSDK_printf("\n");

            for(int i = 0; i<4; ++i)
            {
                FbxString lHeader("FromScratchGlobalTransform Row_");
                FbxString lIndex(i);
                lHeader += lIndex;
                lHeader += ": ";

                Display4DVector(lHeader, lGlobalTransform.GetRow(i));
            }            
            FBXSDK_printf("\n");
        }

        if(lLocal == lLocalTransform)
        {
            for(int i = 0; i<4; ++i)
            {
                FbxString lHeader("LocalTransform Row_");
                FbxString lIndex(i);
                lHeader += lIndex;
                lHeader += ": ";

                Display4DVector(lHeader, lLocal.GetRow(i));
            }            
            FBXSDK_printf("\n");
        }
        else
        {
            FBXSDK_printf("Error: The two local transform results are not equal!\n");
            for(int i = 0; i<4; ++i)
            {
                FbxString lHeader("KFbxEvaluatorLocalTransform Row_");
                FbxString lIndex(i);
                lHeader += lIndex;
                lHeader += ": ";

                Display4DVector(lHeader, lLocal.GetRow(i));
            }            
            FBXSDK_printf("\n");

            for(int i = 0; i<4; ++i)
            {
                FbxString lHeader("FromScratchLocalTransform Row_");
                FbxString lIndex(i);
                lHeader += lIndex;
                lHeader += ": ";

                Display4DVector(lHeader, lLocalTransform.GetRow(i));
            }            
            FBXSDK_printf("\n");
        }
    }

    int lChildCount = pNode->GetChildCount();
    for( int i = 0; i<lChildCount; i++)
    {
        CompareTransformations(pNode->GetChild(i), pScene);
    }
}

/*
Terminology:
Suffix "M" means this is a matrix, suffix "V" means it is a vector.
T is translation.
R is rotation.
S is scaling.
SH is shear.
GlobalRM(x) means the Global Rotation Matrix of node "x".
GlobalRM(P(x)) means the Global Rotation Matrix of the parent node of node "x".
All other transforms are described in the similar way.

The algorithm description:
To calculate global transform of a node x according to different InheritType, 
we need to calculate GlobalTM(x) and [GlobalRM(x) * (GlobalSHM(x) * GlobalSM(x))] separately.
GlobalM(x) = GlobalTM(x) * [GlobalRM(x) * (GlobalSHM(x) * GlobalSM(x))];

InhereitType = RrSs:
GlobalRM(x) * (GlobalSHM(x) * GlobalSM(x)) = GlobalRM(P(x)) * LocalRM(x) * [GlobalSHM(P(x)) * GlobalSM(P(x))] * LocalSM(x);

InhereitType = RSrs:
GlobalRM(x) * (GlobalSHM(x) * GlobalSM(x)) = GlobalRM(P(x)) * [GlobalSHM(P(x)) * GlobalSM(P(x))] * LocalRM(x) * LocalSM(x);

InhereitType = Rrs:
GlobalRM(x) * (GlobalSHM(x) * GlobalSM(x)) = GlobalRM(P(x)) * LocalRM(x) * LocalSM(x);

LocalM(x)= TM(x) * RoffsetM(x)  * RpivotM(x) * RpreM(x) * RM(x) * RpostM(x) * RpivotM(x)^-1 * SoffsetM(x) *SpivotM(x) * SM(x) * SpivotM(x)^-1
LocalTWithAllPivotAndOffsetInformationV(x) = Local(x).GetT();
GlobalTV(x) = GlobalM(P(x)) * LocalTWithAllPivotAndOffsetInformationV(x);

Notice: FBX SDK does not support shear yet, so all local transform won't have shear.
However, global transform might bring in shear by combine the global transform of node in higher hierarchy.
For example, if you scale the parent by a non-uniform scale and then rotate the child node, then a shear will
be generated on the child node's global transform.
In this case, we always compensates shear and store it in the scale matrix too according to following formula:
Shear*Scaling = RotationMatrix.Inverse * TranslationMatrix.Inverse * WholeTranformMatrix
*/
FbxAMatrix CalculateGlobalTransform(FbxNode* pNode) 
{
    FbxAMatrix lTranlationM, lScalingM, lScalingPivotM, lScalingOffsetM, lRotationOffsetM, lRotationPivotM, \
                lPreRotationM, lRotationM, lPostRotationM, lTransform;

    FbxAMatrix lParentGX, lGlobalT, lGlobalRS;

	if(!pNode)
	{
		lTransform.SetIdentity();
		return lTransform;
	}

    // Construct translation matrix
    FbxVector4 lTranslation = pNode->LclTranslation.Get();
    lTranlationM.SetT(lTranslation);

    // Construct rotation matrices
    FbxVector4 lRotation = pNode->LclRotation.Get();
    FbxVector4 lPreRotation = pNode->PreRotation.Get();
    FbxVector4 lPostRotation = pNode->PostRotation.Get();
    lRotationM.SetR(lRotation);
    lPreRotationM.SetR(lPreRotation);
    lPostRotationM.SetR(lPostRotation);

    // Construct scaling matrix
    FbxVector4 lScaling = pNode->LclScaling.Get();
    lScalingM.SetS(lScaling);

    // Construct offset and pivot matrices
    FbxVector4 lScalingOffset = pNode->ScalingOffset.Get();
    FbxVector4 lScalingPivot = pNode->ScalingPivot.Get();
    FbxVector4 lRotationOffset = pNode->RotationOffset.Get();
    FbxVector4 lRotationPivot = pNode->RotationPivot.Get();
    lScalingOffsetM.SetT(lScalingOffset);
    lScalingPivotM.SetT(lScalingPivot);
    lRotationOffsetM.SetT(lRotationOffset);
    lRotationPivotM.SetT(lRotationPivot);

    // Calculate the global transform matrix of the parent node
    FbxNode* lParentNode = pNode->GetParent();
    if(lParentNode)
	{
        lParentGX = CalculateGlobalTransform(lParentNode);
	}
	else
	{
		lParentGX.SetIdentity();
	}

    //Construct Global Rotation
    FbxAMatrix lLRM, lParentGRM;
    FbxVector4 lParentGR = lParentGX.GetR();
    lParentGRM.SetR(lParentGR);
    lLRM = lPreRotationM * lRotationM * lPostRotationM;

    //Construct Global Shear*Scaling
    //FBX SDK does not support shear, to patch this, we use:
    //Shear*Scaling = RotationMatrix.Inverse * TranslationMatrix.Inverse * WholeTranformMatrix
    FbxAMatrix lLSM, lParentGSM, lParentGRSM, lParentTM;
    FbxVector4 lParentGT = lParentGX.GetT();
    lParentTM.SetT(lParentGT);
    lParentGRSM = lParentTM.Inverse() * lParentGX;
    lParentGSM = lParentGRM.Inverse() * lParentGRSM;
    lLSM = lScalingM;

    //Do not consider translation now
    FbxTransform::EInheritType lInheritType = pNode->InheritType.Get();
    if(lInheritType == FbxTransform::eInheritRrSs)
    {
        lGlobalRS = lParentGRM * lLRM * lParentGSM * lLSM;
    }
    else if(lInheritType == FbxTransform::eInheritRSrs)
    {
        lGlobalRS = lParentGRM * lParentGSM * lLRM * lLSM;
    }
    else if(lInheritType == FbxTransform::eInheritRrs)
    {
		FbxAMatrix lParentLSM;
		FbxVector4 lParentLS = lParentNode->LclScaling.Get();
		lParentLSM.SetS(lParentLS);

		FbxAMatrix lParentGSM_noLocal = lParentGSM * lParentLSM.Inverse();
        lGlobalRS = lParentGRM * lLRM * lParentGSM_noLocal * lLSM;
    }
    else
    {
        FBXSDK_printf("error, unknown inherit type! \n");
    }

    // Construct translation matrix
    // Calculate the local transform matrix
    lTransform = lTranlationM * lRotationOffsetM * lRotationPivotM * lPreRotationM * lRotationM * lPostRotationM * lRotationPivotM.Inverse()\
        * lScalingOffsetM * lScalingPivotM * lScalingM * lScalingPivotM.Inverse();
    FbxVector4 lLocalTWithAllPivotAndOffsetInfo = lTransform.GetT();
    // Calculate global translation vector according to: 
    // GlobalTranslation = ParentGlobalTransform * LocalTranslationWithPivotAndOffsetInfo
    FbxVector4 lGlobalTranslation = lParentGX.MultT(lLocalTWithAllPivotAndOffsetInfo);
    lGlobalT.SetT(lGlobalTranslation);

    //Construct the whole global transform
    lTransform = lGlobalT * lGlobalRS;

    return lTransform;
}


