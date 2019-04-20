/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayUserProperties.h"

void DisplayUserProperties(FbxObject* pObject)
{
    int lCount = 0;
    FbxString lTitleStr = "    Property Count: ";

    FbxProperty lProperty = pObject->GetFirstProperty();
    while (lProperty.IsValid())
    {
        if (lProperty.GetFlag(FbxPropertyFlags::eUserDefined))
            lCount++;

        lProperty = pObject->GetNextProperty(lProperty);
    }

    if (lCount == 0)
        return; // there are no user properties to display

    DisplayInt(lTitleStr.Buffer(), lCount);

    lProperty = pObject->GetFirstProperty();
    int i = 0;
    while (lProperty.IsValid())
    {
        if (lProperty.GetFlag(FbxPropertyFlags::eUserDefined))
        {
            DisplayInt("        Property ", i);
            FbxString lString = lProperty.GetLabel();
            DisplayString("            Display Name: ", lString.Buffer());
            lString = lProperty.GetName();
            DisplayString("            Internal Name: ", lString.Buffer());
            DisplayString("            Type: ", lProperty.GetPropertyDataType().GetName());
            if (lProperty.HasMinLimit()) DisplayDouble("            Min Limit: ", lProperty.GetMinLimit());
            if (lProperty.HasMaxLimit()) DisplayDouble("            Max Limit: ", lProperty.GetMaxLimit());
            DisplayBool  ("            Is Animatable: ", lProperty.GetFlag(FbxPropertyFlags::eAnimatable));
            
			FbxDataType lPropertyDataType=lProperty.GetPropertyDataType();

			// BOOL
			if (lPropertyDataType.GetType() == eFbxBool)
            {
                DisplayBool("            Default Value: ", lProperty.Get<FbxBool>());
			}
			// REAL
			else if (lPropertyDataType.GetType() == eFbxDouble || lPropertyDataType.GetType() == eFbxFloat)
			{
                DisplayDouble("            Default Value: ", lProperty.Get<FbxDouble>());
			}
			// COLOR
			else if (lPropertyDataType.Is(FbxColor3DT) || lPropertyDataType.Is(FbxColor4DT))
            {
				FbxColor lDefault;
                char      lBuf[64];

                lDefault = lProperty.Get<FbxColor>();
                FBXSDK_sprintf(lBuf, 64, "R=%f, G=%f, B=%f, A=%f", lDefault.mRed, lDefault.mGreen, lDefault.mBlue, lDefault.mAlpha);
                DisplayString("            Default Value: ", lBuf);
            }
			// INTEGER
			else if (lPropertyDataType.GetType() == eFbxInt)
			{
                DisplayInt("            Default Value: ", lProperty.Get<FbxInt>());
			}
			// VECTOR
			else if(lPropertyDataType.GetType() == eFbxDouble3 || lPropertyDataType.GetType() == eFbxDouble4)
			{
				FbxDouble3 lDefault;
                char   lBuf[64];

                lDefault = lProperty.Get<FbxDouble3>();
                FBXSDK_sprintf(lBuf, 64, "X=%f, Y=%f, Z=%f", lDefault[0], lDefault[1], lDefault[2]);
                DisplayString("            Default Value: ", lBuf);
            }
			// LIST
			else if (lPropertyDataType.GetType() == eFbxEnum)
			{
                DisplayInt("            Default Value: ", lProperty.Get<FbxEnum>());
			}
			// UNIDENTIFIED
            else
			{
                DisplayString("            Default Value: UNIDENTIFIED");
            }
            i++;
        }

        lProperty = pObject->GetNextProperty(lProperty);
    }
}

