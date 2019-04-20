/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// This program converts any file in a format supported by the FBX SDK 
// into DAE, FBX, 3DS, OBJ and DXF files. 
//
// Steps:
// 1. Initialize SDK objects.
// 2. Load a file(fbx, obj,...) to a FBX scene.
// 3. Create a exporter.
// 4. Retrieve the writer ID according to the description of file format.
// 5. Initialize exporter with specified file format
// 6. Export.
// 7. Destroy the exporter
// 8. Destroy the FBX SDK manager
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME "box.fbx"

const char* lFileTypes[] =
{
    "_dae.dae",            "Collada DAE (*.dae)",
    "_fbx7binary.fbx", "FBX binary (*.fbx)",
    "_fbx7ascii.fbx",  "FBX ascii (*.fbx)",
    "_fbx6binary.fbx", "FBX 6.0 binary (*.fbx)",
    "_fbx6ascii.fbx",  "FBX 6.0 ascii (*.fbx)",
    "_obj.obj",            "Alias OBJ (*.obj)",
    "_dxf.dxf",            "AutoCAD DXF (*.dxf)"
};

int main(int argc, char** argv)
{
	FbxString lFilePath("");
	for( int i = 1, c = argc; i < c; ++i )
	{
		if( FbxString(argv[i]) == "-test" ) continue;
		else if( lFilePath.IsEmpty() ) lFilePath = argv[i];
	}
	if( lFilePath.IsEmpty() ) lFilePath = SAMPLE_FILENAME;

    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

	bool lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());
    if( lResult )
    {		
		const size_t lFileNameLength = strlen((argc>=3)?argv[2]:lFilePath.Buffer());
        char* lNewFileName = new char[lFileNameLength+64];
        FBXSDK_strcpy(lNewFileName,lFileNameLength+64,(argc>=3)?argv[2]:lFilePath.Buffer());

        const size_t lFileTypeCount = sizeof(lFileTypes)/sizeof(lFileTypes[0])/2;

        for(size_t i=0; i<lFileTypeCount; ++i)
        {
            // Retrieve the writer ID according to the description of file format.
            int lFormat = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription(lFileTypes[i*2+1]);

            // Construct the output file name.
            FBXSDK_strcpy(lNewFileName+lFileNameLength-4,60, lFileTypes[i*2]);

            // Create an exporter.
            FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

            // Initialize the exporter.
			lResult = lExporter->Initialize(lNewFileName, lFormat, lSdkManager->GetIOSettings());
            if( !lResult )
            {
                FBXSDK_printf("%s:\tCall to FbxExporter::Initialize() failed.\n", lFileTypes[i*2+1]);
                FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
            }
            else
            {
                // Export the scene.
				lResult = lExporter->Export(lScene);
                if( !lResult )
                {
                    FBXSDK_printf("Call to FbxExporter::Export() failed.\n");
                }
            }

            // Destroy the exporter.
            lExporter->Destroy();
        }
        delete[] lNewFileName;
    }
	else
    {
        FBXSDK_printf("Call to LoadScene() failed.\n");
    }

    // Delete the FBX SDK manager. All the objects that have been allocated 
    // using the FBX SDK manager and that haven't been explicitly destroyed 
    // are automatically destroyed at the same time.
    DestroySdkObjects(lSdkManager, lResult);
    return 0;
}

