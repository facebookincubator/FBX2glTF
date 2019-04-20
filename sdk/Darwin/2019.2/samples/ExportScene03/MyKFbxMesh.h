/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _MYKFBXMESH_H_
#define _MYKFBXMESH_H_

#include <fbxsdk.h>

//Show how to create a class derived from existing kfbx class
class MyKFbxMesh : public FbxMesh
{
	FBXSDK_OBJECT_DECLARE(MyKFbxMesh, FbxMesh);

public:
	typedef enum
	{
		eColor =0, //inherited by FbxMesh
		eMY_PROPERTY1,
		eMY_PROPERTY2,
		eMY_PROPERTY3,
		eMY_PROPERTY4,
		eMY_PROPERTY5,
		eMY_PROPERTY6,
		eMY_PROPERTY7,
		eMY_PROPERTY8,
		eMY_PROPERTY9,
		eMY_PROPERTY10,
		eMY_PROPERTY11,
		eMY_PROPERTY_COUNT
	} ePROPERTY;

	//Important to implement
    const char*	GetTypeName() const override;
	FbxProperty GetProperty(int pId);

protected:
    virtual void ConstructProperties(bool pForceSet) override;
	
private:
	int mExtraOption;
};

class MyFbxObject : public FbxObject
{
	FBXSDK_OBJECT_DECLARE(MyFbxObject, FbxObject);

public:
    virtual const char* GetTypeName() const override;

protected:
    virtual void Destruct(bool pRecursive) override;
    virtual void ConstructProperties(bool pForceSet) override;
};

#endif
