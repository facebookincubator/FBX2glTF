/****************************************************************************************

Copyright (C) 2015 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/
 
#ifndef _GET_POSITION_H
#define _GET_POSITION_H
 
#include <fbxsdk.h>

FbxAMatrix GetGlobalPosition(FbxNode* pNode, 
							  const FbxTime& pTime, 
							  FbxPose* pPose = NULL,
							  FbxAMatrix* pParentGlobalPosition = NULL);
FbxAMatrix GetPoseMatrix(FbxPose* pPose, 
                          int pNodeIndex);
FbxAMatrix GetGeometry(FbxNode* pNode);

#endif // #ifndef _GET_POSITION_H



