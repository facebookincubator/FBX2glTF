/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef MY_OWN_WRITER_H
#define MY_OWN_WRITER_H

#include <fbxsdk.h>

//This class is a custom writer.
//The writer provide you the ability to write out node hierarchy to a custom file format.
class MyOwnWriter : public FbxWriter
{
public:
    MyOwnWriter(FbxManager &pManager, int pID);

    //VERY important to put the file close in the destructor
    virtual ~MyOwnWriter();

    virtual bool FileCreate(char* pFileName) ;
    virtual bool FileClose() ;
    virtual bool IsFileOpen();
    virtual void GetWriteOptions() ;
    virtual void PrintHierarchy(FbxNode* pStartNode);
    virtual bool Write(FbxDocument* pDocument);

    virtual bool PreprocessScene(FbxScene &pScene);
    virtual bool PostprocessScene(FbxScene &pScene);

private:
    FILE*		mFilePointer;
    FbxManager*	mManager;
};

#endif
