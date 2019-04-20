/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The example illustrates how to:
//        1) get normals of mesh.
//        2) get smoothing info of mesh.
//        3) compute smoothing info from normals.
//        4) convert hard/soft edges info to smoothing group info.
//
//Background knowledge:
//There are two kinds of smoothing info: 
//1. Smoothing groups info which is saved by polygon. It usually come from 3ds Max, because 3ds Max can set smoothing groups for polygon.
//2. Hard/soft edges info which is saved by edge. It usually come from Maya, because Maya can set hard/soft edges.
//
//steps:
// 1. initialize FBX sdk object.
// 2. load fbx scene form the specified file.
// 3. Get root node of the scene.
// 4. Recursively traverse each node in the scene.
// 5. Get normals of mesh, according to different mapping mode and reference mode.
// 6. Recursively traverse each node in the scene.
// 7. Computing smoothing info from normals or convert smoothing info
// 8. Get smoothing info of mesh, according to different mapping mode and reference mode.
// 9. Destroy all objects.
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME "Normals.fbx"

//set pCompute true to compute smoothing from normals by default 
//set pConvertToSmoothingGroup true to convert hard/soft edge info to smoothing group info by default
void GetSmoothing(FbxManager* pSdkManager, FbxNode* pNode, bool pCompute = false, bool pConvertToSmoothingGroup = false);

//get mesh normals info
void GetNormals(FbxNode* pNode);

static bool gVerbose = true;

int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Load the scene.
    // The example can take a FBX file as an argument.
	FbxString lFilePath("");
	for( int i = 1, c = argc; i < c; ++i )
	{
		if( FbxString(argv[i]) == "-test" ) gVerbose = false;
		else if( lFilePath.IsEmpty() ) lFilePath = argv[i];
	}
	if( lFilePath.IsEmpty() ) lFilePath = SAMPLE_FILENAME;

	FBXSDK_printf("\n\nFile: %s\n\n", lFilePath.Buffer());
	lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while loading the scene...");        
    }
    else 
    {
        if(!lScene)
        {
            FBX_ASSERT_NOW("null scene");
        }

        //get root node of the fbx scene
        FbxNode* lRootNode = lScene->GetRootNode();

        //get normals info, if there're mesh in the scene
        GetNormals(lRootNode);

        //set me true to compute smoothing info from normals
        bool lComputeFromNormals = false;
        //set me true to convert hard/soft edges info to smoothing groups info
        bool lConvertToSmoothingGroup = false;
        //get smoothing info, if there're mesh in the scene
        GetSmoothing(lSdkManager, lRootNode, lComputeFromNormals, lConvertToSmoothingGroup);
    }

    //Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);
    return 0;
}

//get mesh smoothing info
//set pCompute true to compute smoothing from normals by default 
//set pConvertToSmoothingGroup true to convert hard/soft edge info to smoothing group info by default
void GetSmoothing(FbxManager* pSdkManager, FbxNode* pNode, bool pCompute, bool pConvertToSmoothingGroup)
{
    if(!pNode || !pSdkManager)
        return;

    //get mesh
    FbxMesh* lMesh = pNode->GetMesh();
    if(lMesh)
    {
        //print mesh node name
        FBXSDK_printf("current mesh node: %s\n", pNode->GetName());

        //if there's no smoothing info in fbx file, but you still want to get smoothing info.
        //please compute smoothing info from normals.
        //Another case to recompute smoothing info from normals is:
        //If users edit normals manually in 3ds Max or Maya and export the scene to FBX with smoothing info,
        //The smoothing info may NOT match with normals.
        //the mesh called "fbx_customNormals" in Normals.fbx is the case. All edges are hard, but normals actually represent the "soft" looking.
        //Generally, the normals in fbx file holds the smoothing result you'd like to get.
        //If you want to get correct smoothing info(smoothing group or hard/soft edges) which match with normals,
        //please drop the original smoothing info of fbx file, and recompute smoothing info from normals.
        //if you want to get soft/hard edge info, please call FbxGeometryConverter::ComputeEdgeSmoothingFromNormals().
        //if you want to get smoothing group info, please get soft/hard edge info first by ComputeEdgeSmoothingFromNormals() 
        //And then call FbxGeometryConverter::ComputePolygonSmoothingFromEdgeSmoothing().
        if(pCompute)
        {
            FbxGeometryConverter lGeometryConverter(pSdkManager);
            lGeometryConverter.ComputeEdgeSmoothingFromNormals(lMesh);
            //convert soft/hard edge info to smoothing group info
            if(pConvertToSmoothingGroup)
                lGeometryConverter.ComputePolygonSmoothingFromEdgeSmoothing(lMesh);
        }

        //if there is smoothing groups info in your fbx file, but you want to get hard/soft edges info
        //please use following code:
        //FbxGeometryConverter lGeometryConverter(lSdkManager);
        //lGeometryConverter.ComputeEdgeSmoothingFromPolygonSmoothing(lMesh);

        //get smoothing info
        FbxGeometryElementSmoothing* lSmoothingElement = lMesh->GetElementSmoothing();
        if(lSmoothingElement)
        {
            //mapping mode is by edge. The mesh usually come from Maya, because Maya can set hard/soft edges.
            //we can get smoothing info(which edges are soft, which edges are hard) by retrieving each edge. 
            if( lSmoothingElement->GetMappingMode() == FbxGeometryElement::eByEdge )
            {
                //Let's get smoothing of each edge, since the mapping mode of smoothing element is by edge
                for(int lEdgeIndex = 0; lEdgeIndex < lMesh->GetMeshEdgeCount(); lEdgeIndex++)
                {
                    int lSmoothingIndex = 0;
                    //reference mode is direct, the smoothing index is same as edge index.
                    //get smoothing by the index of edge
                    if( lSmoothingElement->GetReferenceMode() == FbxGeometryElement::eDirect )
                        lSmoothingIndex = lEdgeIndex;

                    //reference mode is index-to-direct, get smoothing by the index-to-direct
                    if(lSmoothingElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        lSmoothingIndex = lSmoothingElement->GetIndexArray().GetAt(lEdgeIndex);

                    //Got smoothing of each vertex.
                    int lSmoothingFlag = lSmoothingElement->GetDirectArray().GetAt(lSmoothingIndex);
                    if( gVerbose ) FBXSDK_printf("hard/soft value for edge[%d]: %d \n", lEdgeIndex, lSmoothingFlag);
                    //add your custom code here, to output smoothing or get them into a list, such as KArrayTemplate<int>
                    //. . .
                }//end for lEdgeIndex
            }//end eByEdge
            //mapping mode is by polygon. The mesh usually come from 3ds Max, because 3ds Max can set smoothing groups for polygon.
            //we can get smoothing info(smoothing group ID for each polygon) by retrieving each polygon. 
            else if(lSmoothingElement->GetMappingMode() == FbxGeometryElement::eByPolygon)
            {
                //Let's get smoothing of each polygon, since the mapping mode of smoothing element is by polygon.
                for(int lPolygonIndex = 0; lPolygonIndex < lMesh->GetPolygonCount(); lPolygonIndex++)
                {
                    int lSmoothingIndex = 0;
                    //reference mode is direct, the smoothing index is same as polygon index.
                    if( lSmoothingElement->GetReferenceMode() == FbxGeometryElement::eDirect )
                        lSmoothingIndex = lPolygonIndex;

                    //reference mode is index-to-direct, get smoothing by the index-to-direct
                    if(lSmoothingElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        lSmoothingIndex = lSmoothingElement->GetIndexArray().GetAt(lPolygonIndex);

                    //Got smoothing of each polygon.
                    int lSmoothingFlag = lSmoothingElement->GetDirectArray().GetAt(lSmoothingIndex);
                    if( gVerbose ) FBXSDK_printf("smoothing group ID for polygon[%d]: %d \n", lPolygonIndex, lSmoothingFlag);
                    //add your custom code here, to output normals or get them into a list, such as KArrayTemplate<int>
                    //. . .

                }//end for lPolygonIndex //PolygonCount

            }//end eByPolygonVertex
        }//end if lSmoothingElement
    }//end if lMesh

    //recursively traverse each node in the scene
    int i, lCount = pNode->GetChildCount();
    for (i = 0; i < lCount; i++)
    {
        GetSmoothing(pSdkManager, pNode->GetChild(i), pCompute, pConvertToSmoothingGroup);
    }
}

//get mesh normals info
void GetNormals(FbxNode* pNode)
{
    if(!pNode)
        return;

    //get mesh
    FbxMesh* lMesh = pNode->GetMesh();
    if(lMesh)
    {
        //print mesh node name
        FBXSDK_printf("current mesh node: %s\n", pNode->GetName());

        //get the normal element
        FbxGeometryElementNormal* lNormalElement = lMesh->GetElementNormal();
        if(lNormalElement)
        {
            //mapping mode is by control points. The mesh should be smooth and soft.
            //we can get normals by retrieving each control point
            if( lNormalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint )
            {
                //Let's get normals of each vertex, since the mapping mode of normal element is by control point
                for(int lVertexIndex = 0; lVertexIndex < lMesh->GetControlPointsCount(); lVertexIndex++)
                {
                    int lNormalIndex = 0;
                    //reference mode is direct, the normal index is same as vertex index.
                    //get normals by the index of control vertex
                    if( lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect )
                        lNormalIndex = lVertexIndex;

                    //reference mode is index-to-direct, get normals by the index-to-direct
                    if(lNormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        lNormalIndex = lNormalElement->GetIndexArray().GetAt(lVertexIndex);

                    //Got normals of each vertex.
                    FbxVector4 lNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                    if( gVerbose ) FBXSDK_printf("normals for vertex[%d]: %f %f %f %f \n", lVertexIndex, lNormal[0], lNormal[1], lNormal[2], lNormal[3]);
                    //add your custom code here, to output normals or get them into a list, such as KArrayTemplate<FbxVector4>
                    //. . .
                }//end for lVertexIndex
            }//end eByControlPoint
            //mapping mode is by polygon-vertex.
            //we can get normals by retrieving polygon-vertex.
            else if(lNormalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
            {
                int lIndexByPolygonVertex = 0;
                //Let's get normals of each polygon, since the mapping mode of normal element is by polygon-vertex.
                for(int lPolygonIndex = 0; lPolygonIndex < lMesh->GetPolygonCount(); lPolygonIndex++)
                {
                    //get polygon size, you know how many vertices in current polygon.
                    int lPolygonSize = lMesh->GetPolygonSize(lPolygonIndex);
                    //retrieve each vertex of current polygon.
                    for(int i = 0; i < lPolygonSize; i++)
                    {
                        int lNormalIndex = 0;
                        //reference mode is direct, the normal index is same as lIndexByPolygonVertex.
                        if( lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect )
                            lNormalIndex = lIndexByPolygonVertex;

                        //reference mode is index-to-direct, get normals by the index-to-direct
                        if(lNormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                            lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndexByPolygonVertex);

                        //Got normals of each polygon-vertex.
                        FbxVector4 lNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                        if( gVerbose ) FBXSDK_printf("normals for polygon[%d]vertex[%d]: %f %f %f %f \n", 
                            lPolygonIndex, i, lNormal[0], lNormal[1], lNormal[2], lNormal[3]);
                        //add your custom code here, to output normals or get them into a list, such as KArrayTemplate<FbxVector4>
                        //. . .

                        lIndexByPolygonVertex++;
                    }//end for i //lPolygonSize
                }//end for lPolygonIndex //PolygonCount

            }//end eByPolygonVertex
        }//end if lNormalElement

    }//end if lMesh

    //recursively traverse each node in the scene
    int i, lCount = pNode->GetChildCount();
    for (i = 0; i < lCount; i++)
    {
        GetNormals(pNode->GetChild(i));
    }
}

