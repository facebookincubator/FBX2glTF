/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The scene created in this example is a skeleton made of 3 segments. 
// The position of a node in a .FBX file is expressed in coordinates 
// relative to it's parent. This example shows how to convert to and 
// from a global position. 
//
// The example illustrates how to:
//        1) create a skeleton segment
//        2) get a node's global default position
//        3) set a node's global default position
//        4) set limits, rotation order and pre/post pivots
//        5) export a scene in a .FBX file
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>
#ifdef FBXSDK_ENV_WIN
// On Windows platform need to include this to define  _msize()
#include <malloc.h>
#endif

//include needed for custom reader/writer
#include "../MyOwnWriterReader/MyOwnWriterReader.h"

#include "../Common/Common.h"

#define SAMPLE_FILENAME "ExportScene05.fbx"


// Function prototypes.
bool CreateScene(FbxScene* pScene);

void SetGlobalDefaultPosition(FbxNode* pNode, FbxAMatrix pGlobalPosition);
FbxAMatrix GetGlobalDefaultPosition(FbxNode* pNode);

int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene.
    lResult = CreateScene(lScene);

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while creating the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return 1;
    }

    // Save the scene.

    // The example can take an output file name as an argument.
	const char* lSampleFileName = NULL;
	for( int i = 1; i < argc; ++i )
	{
		if( FBXSDK_stricmp(argv[i], "-test") == 0 ) continue;
		else if( !lSampleFileName ) lSampleFileName = argv[i];
	}
	if( !lSampleFileName ) lSampleFileName = SAMPLE_FILENAME;

    lResult = SaveScene(lSdkManager, lScene, lSampleFileName);
    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return 1;
    }


    //At this point, we have an FBX file.  Let's try to write to a custom writer and read our file with our custom reader.

    FBXSDK_printf("Writing to file with custom writer\n");
    int lRegisteredCount;
    int lPluginId;

    //We need to register the writer for the sdk to be aware.
    lSdkManager->GetIOPluginRegistry()->RegisterWriter(CreateMyOwnWriter, GetMyOwnWriterInfo,
        lPluginId, lRegisteredCount, FillOwnWriterIOSettings);


    //The filename
    const char* lFileName = "CustomWriter.CFF";

    //at this point use our custom writer to write:
    FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

    //Here, we set the custom writer.
    int lFileFormat = lSdkManager->GetIOPluginRegistry()->FindWriterIDByExtension("CFF");
    
    //Initialize the file
    if(lExporter->Initialize(lFileName, lFileFormat, lSdkManager->GetIOSettings()) == false)
    {
        FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
        // Destroy all objects created by the FBX SDK.
        DestroySdkObjects(lSdkManager, false);
        return 1;
    }

    //This will call the write function of the custom writer.
    lResult = lExporter->Export(lScene); 
    if(lResult == false)
    {
        FBXSDK_printf("Error in write of our custom writer\n");
        // Destroy all objects created by the FBX SDK.
        DestroySdkObjects(lSdkManager, lResult);
        return 1;
    }


    //At this point, we have written to the custom writer, let's read with our custom reader.

    //Again, we need to register the reader.
    lSdkManager->GetIOPluginRegistry()->RegisterReader(CreateMyOwnReader, GetMyOwnReaderInfo,
        lPluginId, lRegisteredCount, FillOwnReaderIOSettings);

    //Create the importer
    FbxImporter* lImporter = FbxImporter::Create(lSdkManager,"");


    //We initialize our file.
    //Here we can simply pass the filename, and it should automatically find the right file format and reader.
    lResult = lImporter->Initialize(lFileName, -1, lSdkManager->GetIOSettings() );

    //At one point, this will call our read function of our custom reader.
    lResult = lImporter->Import(lScene);

    if(lResult == false)
    {
        FBXSDK_printf("There was a problem in the read of our custom reader\n");
        // Destroy all objects created by the FBX SDK.
        DestroySdkObjects(lSdkManager, lResult);
        return 1;
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

    return 0;
}

bool CreateScene(FbxScene* pScene)
{
    FbxVector4 lT, lR, lS;
    FbxAMatrix lGM;

    // Create nodes.
    FbxNode* pNodeA = FbxNode::Create(pScene,"A");
    FbxNode* pNodeB = FbxNode::Create(pScene,"B");
    FbxNode* pNodeC = FbxNode::Create(pScene,"C");
    FbxNode* pNodeD = FbxNode::Create(pScene,"D");

    // Create node attributes.
    FbxSkeleton* lSkeletonA = FbxSkeleton::Create(pScene,"");
    lSkeletonA->SetSkeletonType(FbxSkeleton::eRoot);
    pNodeA->SetNodeAttribute(lSkeletonA);
    FbxSkeleton* lSkeletonB = FbxSkeleton::Create(pScene,"");
    lSkeletonB->SetSkeletonType(FbxSkeleton::eLimbNode);
    pNodeB->SetNodeAttribute(lSkeletonB);
    FbxSkeleton* lSkeletonC = FbxSkeleton::Create(pScene,"");
    lSkeletonC->SetSkeletonType(FbxSkeleton::eLimbNode);
    pNodeC->SetNodeAttribute(lSkeletonC);
    FbxSkeleton* lSkeletonD = FbxSkeleton::Create(pScene,"");
    lSkeletonD->SetSkeletonType(FbxSkeleton::eLimbNode);
    pNodeD->SetNodeAttribute(lSkeletonD);


    // On node A we set translation limits
	pNodeA->TranslationActive = true;
	pNodeA->TranslationMinX = true;
	pNodeA->TranslationMinY = true;
	pNodeA->TranslationMinZ = true;
	pNodeA->TranslationMin = FbxVector4(0.1, 0.2, 0.3);
	pNodeA->TranslationMaxX = true;
	pNodeA->TranslationMaxY = true;
	pNodeA->TranslationMaxZ = true;
	pNodeA->TranslationMax = FbxVector4(5.0, 1.0, 0.0);
	pNodeA->UpdatePivotsAndLimitsFromProperties();

    // On node B we set the rotation order and the pre/post pivots
    // (for these value to have an effect, we need to enable the RotationActive flag)
	pNodeB->RotationActive = true;
	pNodeB->RotationMaxX = true;
	pNodeB->RotationMaxY = false;
	pNodeB->RotationMaxZ = false;
	pNodeB->RotationMax = FbxVector4(33.3, 0.0, 0.0);
	pNodeB->UpdatePivotsAndLimitsFromProperties();

    pNodeB->SetPivotState(FbxNode::eSourcePivot, FbxNode::ePivotActive);
    pNodeB->SetRotationOrder(FbxNode::eSourcePivot, eSphericXYZ);
    pNodeB->SetUseRotationSpaceForLimitOnly(FbxNode::eSourcePivot, false);
    pNodeB->SetQuaternionInterpolation(FbxNode::eSourcePivot, eQuatInterpClassic);
    pNodeB->SetRotationPivot(FbxNode::eSourcePivot, FbxVector4(11.1, 22.2, 33.3));
    pNodeB->SetPreRotation(FbxNode::eSourcePivot, FbxVector4(15.0, 30.0, 45.0));
    pNodeB->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(-45.0, -30.0, -15.0));

    // Set node hierarchy.
    pScene->GetRootNode()->AddChild(pNodeA);
    pNodeA->AddChild(pNodeB);
    pNodeB->AddChild(pNodeC);
    pNodeC->AddChild(pNodeD);

    // Set global position of node A.
    lT.Set(0.0, 0.0, 0.0); lGM.SetT(lT);
    lR.Set(0.0, 0.0, 45.0); lGM.SetR(lR);
    SetGlobalDefaultPosition(pNodeA, lGM);

    // Set global position of node B.
    lT.Set(30.0, 20.0, 0.0); lGM.SetT(lT);
    lR.Set(0.0, 0.0, 0.0); lGM.SetR(lR);
    SetGlobalDefaultPosition(pNodeB, lGM);

    // Set global position of node C.
    lT.Set(55.0, 20.0, 0.0); lGM.SetT(lT);
    lR.Set(0.0, 0.0, -40.0); lGM.SetR(lR);
    SetGlobalDefaultPosition(pNodeC, lGM);

    // Set global position of node D.
    lT.Set(70.0, 10.0, 0.0); lGM.SetT(lT);
    lR.Set(0.0, 0.0, 0.0); lGM.SetR(lR);
    SetGlobalDefaultPosition(pNodeD, lGM);

    // Set meta-data on some of the nodes.
    //
    // For this sample, we'll use a hiearchical set of meta-data:
    // 
    // Family
    //      Type
    //          Instance
    // 
    // Family contains all the common properties, and the lower levels override various
    // values.
    //
    FbxObjectMetaData* pFamilyMetaData = FbxObjectMetaData::Create(pScene, "Family");
    FbxProperty::Create(pFamilyMetaData, FbxStringDT, "Level", "Level").Set(FbxString("Family"));
    FbxProperty::Create(pFamilyMetaData, FbxStringDT, "Type", "Type").Set(FbxString("Wall"));
    FbxProperty::Create(pFamilyMetaData, FbxFloatDT, "Width", "Width").Set(10.0f);
    FbxProperty::Create(pFamilyMetaData, FbxDoubleDT, "Weight", "Weight").Set(25.0);
    FbxProperty::Create(pFamilyMetaData, FbxDoubleDT, "Cost", "Cost").Set(1.25);

    FbxObjectMetaData* pTypeMetaData = FbxCast<FbxObjectMetaData>(pFamilyMetaData->Clone(FbxObject::eReferenceClone, pScene));

    pTypeMetaData->SetName("Type");

    // On this level we'll just override two properties
    pTypeMetaData->FindProperty("Cost").Set(2500.0);
    pTypeMetaData->FindProperty("Level").Set(FbxString("Type"));

    FbxObjectMetaData* pInstanceMetaData = FbxCast<FbxObjectMetaData>(pTypeMetaData->Clone(FbxObject::eReferenceClone, pScene));

    pInstanceMetaData->SetName("Instance");

    // And on this level, we'll go in and add a brand new property, too.
    FbxProperty::Create(pInstanceMetaData, FbxStringDT, "Sku", "Sku#").Set(FbxString("143914-10"));
    pInstanceMetaData->FindProperty("Width").Set(1100.50f);
    pInstanceMetaData->FindProperty("Type").Set(FbxString("Super Heavy Duty Wall"));
    pInstanceMetaData->FindProperty("Level").Set(FbxString("Instance"));

    // Finally connect metadata information to some of our nodes.
    pNodeA->ConnectSrcObject(pInstanceMetaData);
    pNodeC->ConnectSrcObject(pInstanceMetaData);    // Share the same object

    pNodeD->ConnectSrcObject(pTypeMetaData);

    return true;
}

// Function to get a node's global default position.
// As a prerequisite, parent node's default local position must be already set.
void SetGlobalDefaultPosition(FbxNode* pNode, FbxAMatrix pGlobalPosition)
{
    FbxAMatrix lLocalPosition;
    FbxAMatrix lParentGlobalPosition;

    if (pNode->GetParent())
    {
        lParentGlobalPosition = GetGlobalDefaultPosition(pNode->GetParent());
        lLocalPosition = lParentGlobalPosition.Inverse() * pGlobalPosition;
    }
    else
    {
        lLocalPosition = pGlobalPosition;
    }

    pNode->LclTranslation.Set(lLocalPosition.GetT());
    pNode->LclRotation.Set(lLocalPosition.GetR());
    pNode->LclScaling.Set(lLocalPosition.GetS());
}

// Recursive function to get a node's global default position.
// As a prerequisite, parent node's default local position must be already set.
FbxAMatrix GetGlobalDefaultPosition(FbxNode* pNode)
{
    FbxAMatrix lLocalPosition;
    FbxAMatrix lGlobalPosition;
    FbxAMatrix lParentGlobalPosition;

    lLocalPosition.SetT(pNode->LclTranslation.Get());
    lLocalPosition.SetR(pNode->LclRotation.Get());
    lLocalPosition.SetS(pNode->LclScaling.Get());

    if (pNode->GetParent())
    {
        lParentGlobalPosition = GetGlobalDefaultPosition(pNode->GetParent());
        lGlobalPosition = lParentGlobalPosition * lLocalPosition;
    }
    else
    {
        lGlobalPosition = lLocalPosition;
    }

    return lGlobalPosition;
}


