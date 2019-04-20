/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "MyOwnWriterReader.h"

// Create your own writer.
// And your writer will get a pPluginID and pSubID. 
FbxWriter* CreateMyOwnWriter(FbxManager& pManager, FbxExporter& /*pExporter*/, int /*pSubID*/, int pPluginID)
{
    // use FbxNew instead of new, since FBX will take charge its deletion
    return FbxNew< MyOwnWriter >(pManager, pPluginID);
}

// Get extension, description or version info about MyOwnWriter
void* GetMyOwnWriterInfo(FbxWriter::EInfoRequest pRequest, int /*pId*/)
{
    static const char* sExt[] = 
    {
        "CFF",
        0
    };

    static const char* sDesc[] = 
    {
        "Example Custom FileFormat (*.CFF)",
        0
    };

    switch (pRequest)
    {
    case FbxWriter::eInfoExtension:
        return sExt;
    case FbxWriter::eInfoDescriptions:
        return sDesc;
    case FbxWriter::eInfoVersions:
        return 0;
    default:
        return 0;
    }
}

void FillOwnWriterIOSettings(FbxIOSettings& /*pIOS*/)
{
    // Here you can write your own FbxIOSettings and parse them.
}


// Creates a MyOwnReader in the Sdk Manager
FbxReader* CreateMyOwnReader(FbxManager& pManager, FbxImporter& /*pImporter*/, int /*pSubID*/, int pPluginID)
{
    // use FbxNew instead of new, since FBX will take charge its deletion
    return FbxNew< MyOwnReader >(pManager, pPluginID);
}

// Get extension, description or version info about MyOwnReader
void *GetMyOwnReaderInfo(FbxReader::EInfoRequest pRequest, int pId)
{
    switch (pRequest)
    {
    case FbxReader::eInfoExtension:
        return GetMyOwnWriterInfo(FbxWriter::eInfoExtension, pId);
    case FbxReader::eInfoDescriptions:
        return GetMyOwnWriterInfo(FbxWriter::eInfoDescriptions, pId);
    default:
        return 0;
    }
}

void FillOwnReaderIOSettings(FbxIOSettings& /*pIOS*/)
{    
    // Here you can write your own FbxIOSettings and parse them.
}
