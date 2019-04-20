/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// This example shows to the user how to get UV information from mesh,
// and how to modify UV information on a specific UV set.
// Steps:
//  1. Initialize FBX SDK Manager and FBX Scene
//  2. Load the input file to scene
//  3. Access Node and its mesh
//  4. Get the UVs Information from the mesh
//  5. Modify the UV information and apply to the mesh
//  6. Save the scene to output file
//  7. Destroy the FBX SDK Manager and FBX Scene
/////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

//input file path
static const char* sInputFile = "sadface.fbx";

//output file path
static const char* sOutputFile = "happyface.fbx";


//We load the all the UV information from the mesh
void LoadUVInformation(FbxMesh* pMesh);

//We modify certain UV set and save the UV to mesh
void SaveUVInformation(FbxMesh* pMesh);


int main()
{

    //FBX SDK Default Manager
    FbxManager* lSdkManager = NULL;

    //Scene to load from file
    FbxScene* lScene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Load the scene.
    bool lResult = LoadScene(lSdkManager, lScene, sInputFile);

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while loading the scene...");
    }
    else 
    {
        //Get the first node in the scene
        FbxNode* lNodeOfInterest = lScene->GetRootNode()->GetChild(0);
        if(lNodeOfInterest)
        {
            FbxMesh* lMeshOFInterest = lNodeOfInterest->GetMesh();
            if(lMeshOFInterest)
            {
                //first, load the UV information and display them
                LoadUVInformation(lMeshOFInterest);

                //then, modify certain uv set and save it
                SaveUVInformation(lMeshOFInterest);

                //save the modified scene to file
                SaveScene(lSdkManager, lScene, sOutputFile);
            }
        }
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);
    return 0;
}

void LoadUVInformation(FbxMesh* pMesh)
{
    //get all UV set names
    FbxStringList lUVSetNameList;
    pMesh->GetUVSetNames(lUVSetNameList);

    //iterating over all uv sets
    for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
    {
        //get lUVSetIndex-th uv set
        const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
        const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

        if(!lUVElement)
            continue;

        // only support mapping mode eByPolygonVertex and eByControlPoint
        if( lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
            lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint )
            return;

        //index array, where holds the index referenced to the uv data
        const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
        const int lIndexCount= (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

        //iterating through the data by polygon
        const int lPolyCount = pMesh->GetPolygonCount();

        if( lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint )
        {
            for( int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex )
            {
                // build the max index array that we need to pass into MakePoly
                const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
                for( int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex )
                {
                    FbxVector2 lUVValue;

                    //get the index of the current vertex in control points array
                    int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex,lVertIndex);

                    //the UV index depends on the reference mode
                    int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

                    lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

                    //User TODO:
                    //Print out the value of UV(lUVValue) or log it to a file
                }
            }
        }
        else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
        {
            int lPolyIndexCounter = 0;
            for( int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex )
            {
                // build the max index array that we need to pass into MakePoly
                const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
                for( int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex )
                {
                    if (lPolyIndexCounter < lIndexCount)
                    {
                        FbxVector2 lUVValue;

                        //the UV index depends on the reference mode
                        int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

                        lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

                        //User TODO:
                        //Print out the value of UV(lUVValue) or log it to a file

                        lPolyIndexCounter++;
                    }
                }
            }
        }
    }
}

void SaveUVInformation(FbxMesh* pMesh)
{
    //iterating over all uv sets
    for (int lUVSetIndex = 0; lUVSetIndex < pMesh->GetElementUVCount(); lUVSetIndex++)
    {
        //get lUVSetIndex-th uv set
        const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetIndex);

        if(!lUVElement)
            continue;

        // only support mapping mode eByPolygonVertex and eByControlPoint
        if( lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
            lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint )
            return;

        //new UV data
        int lNewUVCount = 274/2;
        double lNewUVData[274] = 
        {
            0.000499486923217773,0.000499725341796875,0.10039958357811,0.000499695539474487,0.200299650430679,0.0004996657371521,0.300199747085571,0.0004996657371521,0.400099813938141,0.000499635934829712,0.49999988079071,0.000499606132507324,0.59989994764328,0.000499576330184937,0.69980001449585,0.000499546527862549,0.799700140953064,0.000499546527862549,0.899600148200989,0.000499516725540161,0.999500274658203,0.000499486923217773,0.000499516725540161,0.100399821996689,0.100399613380432,0.100399792194366,0.200299680233002,0.100399762392044,0.300199747085571,0.100399762392044,0.400099813938141,0.100399732589722,0.499999910593033,0.100399702787399,0.599900007247925,0.100399672985077,0.699800074100494,0.100399643182755,0.799700140953064,0.100399643182755,0.899600207805634,0.100399613380432,0.999500274658203,0.10039958357811,0.000499546527862549,0.200299888849258,0.100399643182755,0.200299859046936,0.200299710035324,0.400099903345108,0.300199776887894,0.400099903345108,0.400099843740463,0.40009993314743,0.499999940395355,0.400099962949753,0.599900007247925,0.400099992752075,0.699800074100494,0.400100022554398,0.799700140953064,0.400100022554398,0.899600267410278,0.200299680233002,0.999500334262848,0.200299650430679,0.000499546527862549,0.30019998550415,0.100399643182755,0.300199925899506,0.200299710035324,0.300199806690216,0.300199806690216,0.300199836492538,0.400099873542786,0.300199866294861,0.499999940395355,0.300199866294861,0.599900007247925,0.300199925899506,0.699800133705139,0.300199925899506,0.799700140953064,0.300199955701828,0.899600267410278,0.300199747085571,0.999500334262848,0.300199747085571,0.000499576330184937,0.40010005235672,0.100399672985077,0.400100022554398,0.200299739837646,0.200299739837646,0.300199806690216,0.200299769639969,0.400099903345108,0.200299799442291,0.499999970197678,0.200299799442291,0.59990006685257,0.200299829244614,0.699800133705139,0.200299859046936,0.799700200557709,0.200299888849258,0.899600267410278,0.400099813938141,0.999500393867493,0.400099813938141,0.000499606132507324,
            0.50000011920929,0.100399702787399,0.50000011920929,0.200299769639969,0.500000059604645,0.300199866294861,0.500000059604645,0.40009993314743,0.5,0.5,0.5,0.59990006685257,0.499999970197678,0.699800133705139,0.499999940395355,0.799700260162354,0.499999940395355,0.899600267410278,0.499999910593033,0.999500393867493,0.49999988079071,0.000499635934829712,0.599900186061859,0.100399732589722,0.599900186061859,0.200299799442291,0.599900126457214,0.300199866294861,0.599900126457214,0.40009993314743,0.599900126457214,0.5,0.59990006685257,0.599900126457214,0.59990006685257,0.699800193309784,0.599900007247925,0.799700260162354,0.599900007247925,0.899600327014923,0.599900007247925,0.999500393867493,0.59989994764328,0.0004996657371521,0.699800252914429,0.100399762392044,0.699800252914429,0.200299829244614,0.699800252914429,0.300199896097183,0.699800193309784,0.400099962949753,0.699800193309784,0.500000059604645,0.699800133705139,0.599900126457214,0.699800133705139,0.699800193309784,0.699800133705139,0.799700260162354,0.699800074100494,0.899600386619568,0.699800074100494,0.999500453472137,0.69980001449585,0.0004996657371521,0.799700379371643,0.100399762392044,0.799700319766998,0.200299829244614,0.799700260162354,0.300199925899506,0.799700260162354,0.400099992752075,0.799700260162354,0.500000059604645,0.799700260162354,0.599900126457214,0.799700200557709,0.699800252914429,0.799700140953064,0.799700260162354,0.799700140953064,0.899600386619568,0.799700140953064,0.999500453472137,0.799700140953064,0.000499695539474487,0.899600386619568,0.100399792194366,0.899600386619568,0.200299859046936,0.899600386619568,0.300199925899506,0.899600386619568,0.400100022554398,0.899600327014923,0.50000011920929,0.899600267410278,0.599900186061859,0.899600267410278,0.699800252914429,0.899600267410278,0.799700319766998,0.899600267410278,0.899600386619568,0.899600207805634,0.999500513076782,0.899600148200989,0.000499725341796875,0.999500513076782,0.100399821996689,0.999500513076782,0.200299888849258,0.999500453472137,0.30019998550415,0.999500453472137,
            0.40010005235672,0.999500393867493,0.50000011920929,0.999500393867493,0.599900186061859,0.999500393867493,0.699800252914429,0.999500334262848,0.799700379371643,0.999500334262848,0.899600386619568,0.999500274658203,0.999500513076782,0.999500274658203,0.200299710035324,0.200299829244614,0.300199776887894,0.200299829244614,0.400099843740463,0.200299799442291,0.499999940395355,0.200299769639969,0.599900007247925,0.200299739837646,0.699800074100494,0.200299710035324,0.799700140953064,0.200299710035324,0.200299710035324,0.300199925899506,0.799700140953064,0.300199776887894,0.200299739837646,0.400099992752075,0.300199806690216,0.400099962949753,0.400099903345108,0.40009993314743,0.499999970197678,0.40009993314743,0.59990006685257,0.400099903345108,0.699800133705139,0.400099873542786,0.799700200557709,0.400099843740463
        };

        //new UV index data
        int lNewUVIndexCount = 600;
        int lNewUVIndices[600] = 
        {
            11,0,12,1,12,0,12,1,13,2,13,1,13,2,14,3,14,2,14,3,15,4,15,3,15,4,16,5,16,4,16,5,17,6,17,5,17,6,18,7,18,6,18,7,19,8,19,7,19,8,20,9,20,8,20,9,21,10,21,9,22,11,23,12,23,11,23,12,121,13,121,12,121,13,122,14,122,13,122,14,123,15,123,14,123,15,124,16,124,15,124,16,125,17,125,16,125,17,126,18,126,17,126,18,127,19,127,18,127,19,31,20,31,19,31,20,32,21,32,20,33,22,34,23,34,22,34,23,128,121,128,23,35,24,36,25,36,24,36,25,37,26,37,25,37,26,38,27,38,26,38,27,39,28,39,27,39,28,40,29,40,28,40,29,41,30,41,29,129,127,42,31,42,127,42,31,43,32,43,31,44,33,45,34,45,33,45,34,130,128,130,34,46,35,47,36,47,35,47,36,48,37,48,36,48,37,49,38,49,37,49,38,50,39,50,38,50,39,51,40,51,39,51,40,52,41,52,40,136,129,53,42,53,129,53,42,54,43,54,42,55,44,56,45,56,44,56,45,57,130,57,45,57,130,58,131,58,130,58,131,59,132,59,131,59,132,60,133,60,132,60,133,61,134,61,133,61,134,62,135,62,134,62,135,63,136,63,135,63,136,64,53,64,136,64,53,65,54,65,53,66,55,67,56,67,55,67,56,68,57,68,56,68,57,69,58,69,57,69,58,70,59,70,58,70,59,71,60,71,59,71,60,72,61,72,60,72,61,73,62,73,61,73,62,74,63,74,62,74,63,75,64,75,63,75,64,76,65,76,64,77,66,78,67,78,66,78,67,79,68,79,67,79,68,80,69,80,68,80,69,81,70,81,69,81,70,82,71,82,70,82,71,83,72,83,71,83,72,84,73,84,72,84,73,85,74,85,73,85,74,86,75,86,74,86,75,87,76,87,75,88,77,89,78,89,77,89,78,90,79,90,78,90,79,91,80,91,79,91,80,92,81,92,80,92,81,93,82,93,81,93,82,94,83,94,82,94,83,95,84,95,83,95,84,96,85,96,84,96,85,97,86,97,85,97,86,98,87,98,86,99,88,100,89,100,88,100,89,101,90,101,89,101,90,102,91,102,90,102,91,103,92,103,91,103,92,104,93,104,92,104,93,105,94,105,93,105,94,106,95,106,94,106,95,107,96,107,95,107,96,108,97,108,96,108,97,109,98,109,97,110,99,111,100,111,99,111,100,112,101,112,100,112,101,113,102,113,101,113,102,114,103,114,102,114,103,115,104,115,103,115,104,116,105,116,104,116,105,117,106,117,105,117,106,118,107,118,106,118,107,119,108,119,107,119,108,120,109,120,108
        };

        //set new UV data to direct array
        lUVElement->GetDirectArray().Resize(lNewUVCount);
        for(int lUVIndex = 0; lUVIndex < lNewUVCount; lUVIndex++)
        {
            FbxVector2 lUV(lNewUVData[2*lUVIndex], lNewUVData[2*lUVIndex+1]);
            lUVElement->GetDirectArray().SetAt(lUVIndex, lUV);
        }

        //set new UV index data to index array
        lUVElement->GetIndexArray().Resize(lNewUVIndexCount);
        for(int lUVIndex = 0; lUVIndex < lNewUVIndexCount; lUVIndex++)
        {
            lUVElement->GetIndexArray().SetAt(lUVIndex, lNewUVIndices[lUVIndex]);
        }

        return;
    }
}

