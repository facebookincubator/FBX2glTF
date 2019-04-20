/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef MY_OWN_READER_H
#define MY_OWN_READER_H

#include <fbxsdk.h>

// This class is a custom reader.
// The reader provide you the ability to get file version, read options and read hierarchy from file.
class MyOwnReader : public FbxReader
{
public:
	MyOwnReader(FbxManager &pManager, int pID);

	//VERY important to put the file close in the destructor
	virtual ~MyOwnReader();

	virtual void GetVersion(int& pMajor, int& pMinor, int& pRevision);
	virtual bool FileOpen(char* pFileName);
	virtual bool FileClose();
	virtual bool IsFileOpen();

	virtual bool GetReadOptions(bool pParseFileAsNeeded = true);
	virtual bool Read(FbxDocument* pDocument);

private:
	FILE*		mFilePointer;
	FbxManager*	mManager;
};

#endif /* MY_OWN_READER_H */
