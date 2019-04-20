/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "MyKFbxMesh.h"

FBXSDK_OBJECT_IMPLEMENT(MyKFbxMesh);
FBXSDK_OBJECT_IMPLEMENT(MyFbxObject);

const char* MyKFbxMesh::GetTypeName() const
{
    return "MyKFbxMesh";
}

FbxProperty MyKFbxMesh::GetProperty(int pId)
{
    FbxProperty dummy;
    switch (pId)
    {
    case eMY_PROPERTY1 : return FindProperty("MyExtraPropertyBool");
    case eMY_PROPERTY2 : return FindProperty("MyExtraPropertyInteger");
    case eMY_PROPERTY3 : return FindProperty("MyExtraPropertyFloat");
    case eMY_PROPERTY4 : return FindProperty("MyExtraPropertyDouble");
    case eMY_PROPERTY5 : return FindProperty("MyExtraPropertyString");
    case eMY_PROPERTY6 : return FindProperty("MyExtraPropertyVector3");
    case eMY_PROPERTY7 : return FindProperty("MyExtraPropertyColor");
    case eMY_PROPERTY8 : return FindProperty("MyExtraPropertyVector4");
    case eMY_PROPERTY9 : return FindProperty("MyExtraPropertyMatrix4x4");
    case eMY_PROPERTY10: return FindProperty("MyExtraPropertyEnum");
    case eMY_PROPERTY11: return FindProperty("MyExtraPropertyTime");
    default:
        break;
    };

    return dummy;
}

void MyKFbxMesh::ConstructProperties(bool pForceSet)
{
	ParentClass::ConstructProperties(pForceSet);

    FbxProperty::Create(this, FbxBoolDT, "MyExtraPropertyBool", "MyExtraPropertyLabel1");
    FbxProperty::Create(this, FbxIntDT, "MyExtraPropertyInteger", "MyExtraPropertyLabel2");
    FbxProperty::Create(this, FbxFloatDT, "MyExtraPropertyFloat", "MyExtraPropertyLabel3");
    FbxProperty::Create(this, FbxDoubleDT, "MyExtraPropertyDouble", "MyExtraPropertyLabel4");
    FbxProperty::Create(this, FbxStringDT, "MyExtraPropertyString", "MyExtraPropertyLabel5");
    FbxProperty::Create(this, FbxDouble3DT, "MyExtraPropertyVector3", "MyExtraPropertyLabel6");
    FbxProperty::Create(this, FbxColor3DT, "MyExtraPropertyColor", "MyExtraPropertyLabel7");
    FbxProperty::Create(this, FbxDouble4DT, "MyExtraPropertyVector4", "MyExtraPropertyLabel8");
    FbxProperty::Create(this, FbxDouble4x4DT, "MyExtraPropertyMatrix4x4", "MyExtraPropertyLabel9");
    FbxProperty::Create(this, FbxEnumDT, "MyExtraPropertyEnum", "MyExtraPropertyLabel10");
    FbxProperty::Create(this, FbxTimeDT, "MyExtraPropertyTime", "MyExtraPropertyLabel11");

    //we must set the flag to eUser if it is FbxEnumDT or FbxStringListDT
    this->GetProperty((int)eMY_PROPERTY10).ModifyFlag(FbxPropertyFlags::eUserDefined, true);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const char* MyFbxObject::GetTypeName() const
{
    return "MyFbxObject";
}

void MyFbxObject::Destruct(bool pRecursive)
{
    ParentClass::Destruct(pRecursive);
}

void MyFbxObject::ConstructProperties(bool pForceSet)
{
	ParentClass::ConstructProperties(pForceSet);
    FbxProperty::Create(this, FbxDoubleDT, "MyAnimatedPropertyName", "MyFbxObject Animated Property Label");
}
