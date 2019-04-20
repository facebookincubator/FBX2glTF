/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayNurb.h"

#include "DisplayTexture.h"
#include "DisplayMaterial.h"
#include "DisplayLink.h"
#include "DisplayShape.h"
#include "DisplayCache.h"

#if defined (FBXSDK_ENV_MAC)
// disable the “format not a string literal and no format arguments” warning since
// the FBXSDK_printf calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void DisplayNurb(FbxNode* pNode)
{
    FbxNurbs* lNurbs = (FbxNurbs*) pNode->GetNodeAttribute ();
    int i;

    DisplayString("Nurb Name: ", (char *) pNode->GetName());
    DisplayMetaDataConnections(lNurbs);

    const char* lSurfaceModes[] = { "Raw", "Low No Normals", "Low", "High No Normals", "High" };

    DisplayString("    Surface Mode: ", lSurfaceModes[lNurbs->GetSurfaceMode()]);

    int lControlPointsCount = lNurbs->GetControlPointsCount();
    FbxVector4* lControlPoints = lNurbs->GetControlPoints();

    for (i = 0; i < lControlPointsCount; i++)
    {
        DisplayInt("    Control Point ", i);
        Display3DVector("        Coordinates: ", lControlPoints[i]);
        DisplayDouble("        Weight: ", lControlPoints[i][3]);
    }

    const char* lNurbTypes[] = { "Periodic", "Closed", "Open" };

    DisplayString("    Nurb U Type: ", lNurbTypes[lNurbs->GetNurbsUType()]);
    DisplayInt("    U Count: ", lNurbs->GetUCount());
    DisplayString("    Nurb V Type: ", lNurbTypes[lNurbs->GetNurbsVType()]);
    DisplayInt("    V Count: ", lNurbs->GetVCount());
    DisplayInt("    U Order: ", lNurbs->GetUOrder());
    DisplayInt("    V Order: ", lNurbs->GetVOrder());
    DisplayInt("    U Step: ", lNurbs->GetUStep());
    DisplayInt("    V Step: ", lNurbs->GetVStep());

    FbxString lString;
    int lUKnotCount = lNurbs->GetUKnotCount();
    int lVKnotCount = lNurbs->GetVKnotCount();
    int lUMultiplicityCount = lNurbs->GetUCount();
    int lVMultiplicityCount = lNurbs->GetVCount();
    double* lUKnotVector = lNurbs->GetUKnotVector();
    double* lVKnotVector = lNurbs->GetVKnotVector();
    int* lUMultiplicityVector = lNurbs->GetUMultiplicityVector();
    int* lVMultiplicityVector = lNurbs->GetVMultiplicityVector();

    lString = "    U Knot Vector: ";

    for (i = 0; i < lUKnotCount; i++)
    {
        lString += (float) lUKnotVector[i];

        if (i < lUKnotCount - 1)
        {
            lString += ", ";
        }
    }

    lString += "\n";
    FBXSDK_printf(lString);

    lString = "    V Knot Vector: ";

    for (i = 0; i < lVKnotCount; i++)
    {
        lString += (float) lVKnotVector[i];

        if (i < lVKnotCount - 1)
        {
            lString += ", ";
        }
    }

    lString += "\n";
    FBXSDK_printf(lString);

    lString = "    U Multiplicity Vector: ";

    for (i = 0; i < lUMultiplicityCount; i++)
    {
        lString += lUMultiplicityVector[i];

        if (i < lUMultiplicityCount - 1)
        {
            lString += ", ";
        }
    }

    lString += "\n";
    FBXSDK_printf(lString);

    lString = "    V Multiplicity Vector: ";

    for (i = 0; i < lVMultiplicityCount; i++)
    {
        lString += lVMultiplicityVector[i];

        if (i < lVMultiplicityCount - 1)
        {
            lString += ", ";
        }
    }

    lString += "\n";
    FBXSDK_printf(lString);

    DisplayString("");

    DisplayTexture(lNurbs);
    DisplayMaterial(lNurbs);
    DisplayLink(lNurbs);
    DisplayShape(lNurbs);
	DisplayCache(lNurbs);
}
