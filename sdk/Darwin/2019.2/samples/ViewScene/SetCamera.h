/****************************************************************************************

Copyright (C) 2015 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _SET_CAMERA_H
#define _SET_CAMERA_H

void SetCamera(FbxScene* pScene, 
               FbxTime& pTime, 
               FbxAnimLayer* pAnimLayer,
               const FbxArray<FbxNode*>& pCameraArray,
               int pWindowWidth, int pWindowHeight);

FbxCamera* GetCurrentCamera(FbxScene* pScene);

void CameraZoom(FbxScene* pScene, int pZoomDepth, int pZoomMode);

void CameraOrbit(FbxScene* pScene, FbxVector4 lOrigCamPos, double OrigRoll, int dX, int dY);

void CameraPan(FbxScene* pScene, FbxVector4 lOrigCamPos, FbxVector4 lOrigCamCenter, 
			   double OrigRoll, int dX, int dY);

#endif // #ifndef _SET_CAMERA_H






