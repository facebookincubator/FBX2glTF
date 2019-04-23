/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include <fbxsdk.h>

#include "DisplayCommon.h"

void DisplayProperties(FbxObject* pObject);
void DisplayGenericInfo(FbxNode* pNode, int pDepth);
void DisplayGenericInfo(FbxScene* pScene)
{
    int i;
    FbxNode* lRootNode = pScene->GetRootNode();

    for( i = 0; i < lRootNode->GetChildCount(); i++)
    {
        DisplayGenericInfo(lRootNode->GetChild(i), 0);
    }

	//Other objects directly connected onto the scene
	for( i = 0; i < pScene->GetSrcObjectCount(); ++i )
	{
		DisplayProperties(pScene->GetSrcObject(i));
	}
}


void DisplayGenericInfo(FbxNode* pNode, int pDepth)
{
    FbxString lString;
    int i;

    for(i = 0; i < pDepth; i++)
    {
        lString += "     ";
    }

    lString += pNode->GetName();
    lString += "\n";

    DisplayString(lString.Buffer());

    //Display generic info about that Node
    DisplayProperties(pNode);
    DisplayString("");
    for(i = 0; i < pNode->GetChildCount(); i++)
    {
        DisplayGenericInfo(pNode->GetChild(i), pDepth + 1);
    }
}

void DisplayProperties(FbxObject* pObject)
{

    DisplayString("Name: ", (char *)pObject->GetName());

    // Display all the properties
    int i,  lCount = 0;
    FbxProperty lProperty = pObject->GetFirstProperty();
    while (lProperty.IsValid())
    {
        lCount++;
        lProperty = pObject->GetNextProperty(lProperty);
    }

    FbxString lTitleStr = "    Property Count: ";

    if (lCount == 0)
        return; // there are no properties to display

    DisplayInt(lTitleStr.Buffer(), lCount);

    i=0;
	lProperty = pObject->GetFirstProperty();
    while (lProperty.IsValid())
    {
        // exclude user properties

        FbxString lString;
        DisplayInt("        Property ", i);
        lString = lProperty.GetLabel();
        DisplayString("            Display Name: ", lString.Buffer());
        lString = lProperty.GetName();
        DisplayString("            Internal Name: ", lString.Buffer());
        lString = lProperty.GetPropertyDataType().GetName();
        DisplayString("            Type: ",lString.Buffer());
        if (lProperty.HasMinLimit()) DisplayDouble("            Min Limit: ", lProperty.GetMinLimit());
        if (lProperty.HasMaxLimit()) DisplayDouble("            Max Limit: ", lProperty.GetMaxLimit());
        DisplayBool  ("            Is Animatable: ", lProperty.GetFlag(FbxPropertyFlags::eAnimatable));


        switch (lProperty.GetPropertyDataType().GetType())
        {
        case eFbxBool:
            DisplayBool("            Default Value: ", lProperty.Get<FbxBool>());
            break;

        case eFbxDouble:
            DisplayDouble("            Default Value: ", lProperty.Get<FbxDouble>());
            break;

        case eFbxDouble4:
            {
                FbxColor lDefault;
                char      lBuf[64];

                lDefault = lProperty.Get<FbxColor>();
                FBXSDK_sprintf(lBuf, 64, "R=%f, G=%f, B=%f, A=%f", lDefault.mRed, lDefault.mGreen, lDefault.mBlue, lDefault.mAlpha);
                DisplayString("            Default Value: ", lBuf);
            }
            break;

        case eFbxInt:
            DisplayInt("            Default Value: ", lProperty.Get<FbxInt>());
            break;

        case eFbxDouble3:
            {
                FbxDouble3 lDefault;
                char   lBuf[64];

                lDefault = lProperty.Get<FbxDouble3>();
                FBXSDK_sprintf(lBuf, 64, "X=%f, Y=%f, Z=%f", lDefault[0], lDefault[1], lDefault[2]);
                DisplayString("            Default Value: ", lBuf);
            }
            break;

        //case FbxEnumDT:
        //    DisplayInt("            Default Value: ", lProperty.Get<FbxEnum>());
        //    break;

        case eFbxFloat:
            DisplayDouble("            Default Value: ", lProperty.Get<FbxFloat>());
            break;
        case eFbxString:
            lString = lProperty.Get<FbxString>();
            DisplayString("            Default Value: ", lString.Buffer());
            break;

        default:
            DisplayString("            Default Value: UNIDENTIFIED");
            break;
        }
        i++;
        lProperty = pObject->GetNextProperty(lProperty);
    }
}

