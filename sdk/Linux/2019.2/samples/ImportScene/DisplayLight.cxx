/****************************************************************************************

   Copyright (C) 2017 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayLight.h"

void DisplayDefaultAnimationValues(FbxLight* pLight);

void DisplayLight(FbxNode* pNode)
{
    FbxLight* lLight = (FbxLight*) pNode->GetNodeAttribute();

    DisplayString("Light Name: ", (char *) pNode->GetName());
    DisplayMetaDataConnections(lLight);

    const char* lLightTypes[] = { "Point", "Directional", "Spot", "Area", "Volume" };

    DisplayString("    Type: ", lLightTypes[lLight->LightType.Get()]);
    DisplayBool("    Cast Light: ", lLight->CastLight.Get());

    if (!(lLight->FileName.Get().IsEmpty()))
    {
        DisplayString("    Gobo");

        DisplayString("        File Name: \"", lLight->FileName.Get().Buffer(), "\"");
        DisplayBool("        Ground Projection: ", lLight->DrawGroundProjection.Get());
        DisplayBool("        Volumetric Projection: ", lLight->DrawVolumetricLight.Get());
        DisplayBool("        Front Volumetric Projection: ", lLight->DrawFrontFacingVolumetricLight.Get());
    }

    DisplayDefaultAnimationValues(lLight);
}


void DisplayDefaultAnimationValues(FbxLight* pLight)
{
    DisplayString("    Default Animation Values");

    FbxDouble3 c = pLight->Color.Get();
    FbxColor lColor(c[0], c[1], c[2]);
    DisplayColor("        Default Color: ", lColor);
    DisplayDouble("        Default Intensity: ", pLight->Intensity.Get());
    DisplayDouble("        Default Outer Angle: ", pLight->OuterAngle.Get());
    DisplayDouble("        Default Fog: ", pLight->Fog.Get());
}


