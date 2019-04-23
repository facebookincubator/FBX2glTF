/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

#include <fbxsdk.h>

void DisplayPivotsAndLimits(FbxNode* pNode)
{
    FbxVector4 lTmpVector;

    //
    // Pivots
    //
    FBXSDK_printf("    Pivot Information\n");

    FbxNode::EPivotState lPivotState;
    pNode->GetPivotState(FbxNode::eSourcePivot, lPivotState);
    FBXSDK_printf("        Pivot State: %s\n", lPivotState == FbxNode::ePivotActive ? "Active" : "Reference");

    lTmpVector = pNode->GetPreRotation(FbxNode::eSourcePivot);
    FBXSDK_printf("        Pre-Rotation: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

    lTmpVector = pNode->GetPostRotation(FbxNode::eSourcePivot);
    FBXSDK_printf("        Post-Rotation: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

    lTmpVector = pNode->GetRotationPivot(FbxNode::eSourcePivot);
    FBXSDK_printf("        Rotation Pivot: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

    lTmpVector = pNode->GetRotationOffset(FbxNode::eSourcePivot);
    FBXSDK_printf("        Rotation Offset: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

    lTmpVector = pNode->GetScalingPivot(FbxNode::eSourcePivot);
    FBXSDK_printf("        Scaling Pivot: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

    lTmpVector = pNode->GetScalingOffset(FbxNode::eSourcePivot);
    FBXSDK_printf("        Scaling Offset: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

    //
    // Limits
    //
    bool		lIsActive, lMinXActive, lMinYActive, lMinZActive;
    bool		lMaxXActive, lMaxYActive, lMaxZActive;
    FbxDouble3	lMinValues, lMaxValues;

    FBXSDK_printf("    Limits Information\n");

	lIsActive = pNode->TranslationActive;
	lMinXActive = pNode->TranslationMinX;
	lMinYActive = pNode->TranslationMinY;
	lMinZActive = pNode->TranslationMinZ;
	lMaxXActive = pNode->TranslationMaxX;
	lMaxYActive = pNode->TranslationMaxY;
	lMaxZActive = pNode->TranslationMaxZ;
	lMinValues = pNode->TranslationMin;
	lMaxValues = pNode->TranslationMax;

    FBXSDK_printf("        Translation limits: %s\n", lIsActive ? "Active" : "Inactive");
    FBXSDK_printf("            X\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinXActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[0]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxXActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[0]);
    FBXSDK_printf("            Y\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinYActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[1]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxYActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[1]);
    FBXSDK_printf("            Z\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinZActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[2]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxZActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[2]);

	lIsActive = pNode->RotationActive;
	lMinXActive = pNode->RotationMinX;
	lMinYActive = pNode->RotationMinY;
	lMinZActive = pNode->RotationMinZ;
	lMaxXActive = pNode->RotationMaxX;
	lMaxYActive = pNode->RotationMaxY;
	lMaxZActive = pNode->RotationMaxZ;
	lMinValues = pNode->RotationMin;
	lMaxValues = pNode->RotationMax;

    FBXSDK_printf("        Rotation limits: %s\n", lIsActive ? "Active" : "Inactive");
    FBXSDK_printf("            X\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinXActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[0]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxXActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[0]);
    FBXSDK_printf("            Y\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinYActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[1]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxYActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[1]);
    FBXSDK_printf("            Z\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinZActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[2]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxZActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[2]);

	lIsActive = pNode->ScalingActive;
	lMinXActive = pNode->ScalingMinX;
	lMinYActive = pNode->ScalingMinY;
	lMinZActive = pNode->ScalingMinZ;
	lMaxXActive = pNode->ScalingMaxX;
	lMaxYActive = pNode->ScalingMaxY;
	lMaxZActive = pNode->ScalingMaxZ;
	lMinValues = pNode->ScalingMin;
	lMaxValues = pNode->ScalingMax;

    FBXSDK_printf("        Scaling limits: %s\n", lIsActive ? "Active" : "Inactive");
    FBXSDK_printf("            X\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinXActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[0]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxXActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[0]);
    FBXSDK_printf("            Y\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinYActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[1]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxYActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[1]);
    FBXSDK_printf("            Z\n");
    FBXSDK_printf("                Min Limit: %s\n", lMinZActive ? "Active" : "Inactive");
    FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[2]);
    FBXSDK_printf("                Max Limit: %s\n", lMaxZActive ? "Active" : "Inactive");
    FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[2]);
}

