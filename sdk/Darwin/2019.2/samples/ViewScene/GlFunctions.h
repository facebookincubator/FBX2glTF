/****************************************************************************************

Copyright (C) 2015 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _GL_FUNCTIONS_H
#define _GL_FUNCTIONS_H

#include <fbxsdk.h>

#include "GL/glew.h"
#define GLUT_DISABLE_ATEXIT_HACK
#if defined(__MACH__)
#include <GLUT/glut.h>
#else
#include "GL/gl.h"
#include "GL/glu.h"
#endif

#include <math.h>

enum ShadingMode
{
    SHADING_MODE_WIREFRAME,
    SHADING_MODE_SHADED,
};

void GlSetCameraPerspective(double pFieldOfViewY,
							double pAspect,
							double pNearPlane,
							double pFarPlane,
							FbxVector4& pEye,
							FbxVector4& pCenter,
							FbxVector4& pUp,
                            double  pFilmOffsetX,
                            double  pFilmOffsetY);
void GlSetCameraOrthogonal(double pLeftPlane,
						   double pRightPlane,
						   double pBottomPlane,
						   double pTopPlane,
						   double pNearPlane,
						   double pFarPlane,
						   FbxVector4& pEye,
						   FbxVector4& pCenter,
                           FbxVector4& pUp);

void GlDrawMarker(FbxAMatrix& pGlobalPosition);
void GlDrawLimbNode(FbxAMatrix& pGlobalBasePosition, 
					FbxAMatrix& pGlobalEndPosition);
void GlDrawCamera(FbxAMatrix& pGlobalPosition, 
				  double pRoll);
void GlDrawCrossHair(FbxAMatrix& pGlobalPosition);

#endif // #ifndef _GL_FUNCTIONS_H


