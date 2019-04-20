/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

// use the fbxsdk.h
#include <fbxsdk.h>

// to build a scene from an FBX file
bool LoadFBXScene(
                    const char *pFbxFilePath 
                 );

// to read a file using an FBX SDK reader
bool LoadScene(
                FbxManager       *pSdkManager, 
                FbxScene            *pScene, 
                const char           *pFbxFilePath
              );


// to create a SDK manager and a new scene
void InitializeSdkManagerAndScene();

// to get the filters for the <Open file> dialog (description + file extention)
const char *GetReaderOFNFilters();



// to destroy an instance of the SDK manager
void DestroySdkObjects(
                        FbxManager* pSdkManager,
						bool pExitStatus
                      );

// to get the root node
const FbxNode* GetRootNode();

// to get the root node name
const char * GetRootNodeName();

// to get a string from the node name and attribute type
FbxString GetNodeNameAndAttributeTypeName(
                                         const FbxNode* pNode
                                       );

// to get a string from the node default translation values
FbxString GetDefaultTranslationInfo(
								   const FbxNode* pNode
								 );

// to get a string from the node visibility value
FbxString GetNodeVisibility(
							const FbxNode* pNode
						 );




