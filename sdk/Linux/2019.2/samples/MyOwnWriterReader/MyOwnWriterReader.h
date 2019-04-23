/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

////////////////////////////////////////////////////////////////////////////
// This example illustrates how to create your own writer/reader,
// and this will be used in IOPluginRegistry to RegisterWriter/Reader.
////////////////////////////////////////////////////////////////////////////


#ifndef MY_OWN_WRITER_PLUGIN_H
#define MY_OWN_WRITER_PLUGIN_H

#include <fbxsdk.h>
#include "MyOwnWriter.h"
#include "MyOwnReader.h"

FbxWriter* CreateMyOwnWriter(FbxManager& pManager, FbxExporter& pExporter, int pSubID, int pPluginID);
void* GetMyOwnWriterInfo(FbxWriter::EInfoRequest pRequest, int pId);
void FillOwnWriterIOSettings(FbxIOSettings& pIOS);

FbxReader* CreateMyOwnReader(FbxManager& pManager, FbxImporter& pImporter, int pSubID, int pPluginID);
void *GetMyOwnReaderInfo(FbxReader::EInfoRequest pRequest, int pId);
void FillOwnReaderIOSettings(FbxIOSettings& pIOS);

#endif
