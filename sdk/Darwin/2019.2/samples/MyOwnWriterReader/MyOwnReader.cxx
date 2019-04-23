/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "MyOwnReader.h"

MyOwnReader::MyOwnReader(FbxManager &pManager, int pID):
FbxReader(pManager, pID,  FbxStatusGlobal::GetRef()),
mFilePointer(NULL),
mManager(&pManager)
{
}

MyOwnReader::~MyOwnReader()
{
    FileClose();
}

void MyOwnReader::GetVersion(int& pMajor, int& pMinor, int& pRevision)

{
    pMajor = 1;
    pMinor = 0;
    pRevision=0;
}

bool MyOwnReader::FileOpen(char* pFileName)
{
    if(mFilePointer != NULL)
        FileClose();
    FBXSDK_fopen(mFilePointer, pFileName, "r");
    if(mFilePointer == NULL)
        return false;
    return true;
}
bool MyOwnReader::FileClose()
{
    if(mFilePointer!=NULL)
        fclose(mFilePointer);
    return true;
    
}
bool MyOwnReader::IsFileOpen()
{
    if(mFilePointer != NULL)
        return true;
    return false;
}

bool MyOwnReader::GetReadOptions(bool /*pParseFileAsNeeded*/)
{
    return true;
}

//Read the custom file and reconstruct node hierarchy.
bool MyOwnReader::Read(FbxDocument* pDocument)
{
    if (!pDocument)
    {
        GetStatus().SetCode(FbxStatus::eFailure, "Invalid document handle");
        return false;
    }
    FbxScene*      lScene = FbxCast<FbxScene>(pDocument);
    bool            lIsAScene = (lScene != NULL);
    bool            lResult = false;

    if(lIsAScene)
    {
        FbxNode* lRootNode = lScene->GetRootNode();
        FbxNodeAttribute * lRootNodeAttribute = FbxNull::Create(lScene,"");
        lRootNode->SetNodeAttribute(lRootNodeAttribute);

        int lSize;
        char* lBuffer = NULL;    
        if(mFilePointer != NULL)
        {
            //To obtain file size
            fseek (mFilePointer , 0 , SEEK_END);
            lSize = ftell (mFilePointer);
            rewind (mFilePointer);

            //Read file content to a string.
            lBuffer = (char*) malloc (sizeof(char)*lSize + 1);
            size_t lRead = fread(lBuffer, 1, lSize, mFilePointer);
            lBuffer[lRead]='\0';
            FbxString lString(lBuffer);

            //Parse the string to get name and relation of Nodes. 
            FbxString lSubString, lChildName, lParentName;
            FbxNode* lChildNode;
            FbxNode* lParentNode;
            FbxNodeAttribute* lChildAttribute;
            int lEndTokenCount = lString.GetTokenCount("\n");

            for (int i = 0; i < lEndTokenCount; i++)
            {
                lSubString = lString.GetToken(i, "\n");
                FbxString lNodeString;
                lChildName = lSubString.GetToken(0, "\"");
                lParentName = lSubString.GetToken(2, "\"");

                //Build node hierarchy.
                if(lParentName == "RootNode")
                {
                    lChildNode = FbxNode::Create(lScene,lChildName.Buffer());
                    lChildAttribute = FbxNull::Create(mManager,"");
                    lChildNode->SetNodeAttribute(lChildAttribute);

                    lRootNode->AddChild(lChildNode);
                }
                else
                {
                    lChildNode = FbxNode::Create(lScene,lChildName.Buffer());
                    lChildAttribute = FbxNull::Create(lScene,"");
                    lChildNode->SetNodeAttribute(lChildAttribute);

                    lParentNode = lRootNode->FindChild(lParentName.Buffer());
                    lParentNode->AddChild(lChildNode);
                }
            }
            free(lBuffer);
        }
        lResult = true;
    }    
    return lResult;
}
