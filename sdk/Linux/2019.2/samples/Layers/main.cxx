/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// The scene created in this example is a cube with Layered textures and
// with materials mapped on it's faces. 
//
// This sample illustrates how to use the following Elements:
//      - Normal
//      - Material
//      - UVs
//      - Vertex Color
//      - Polygon Group
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME "Layers.fbx"
#define BACKGROUND_IMAGE_NAME "Spotty"
#define BACKGROUND_IMAGE      "spotty.jpg"
#define GEO1_IMAGE_NAME     "One"
#define GEO1_IMAGE          "1.jpg"
#define GEO2_IMAGE_NAME     "Waffle"
#define GEO2_IMAGE          "waffle.jpg"

typedef double Vector4[4];
typedef double Vector2[2];

// Function prototypes.
FbxNode* CreateCube(FbxScene* pScene, const char* pName);
FbxTexture* CreateTexture(FbxScene* pScene, const char* name, const char* filename);

int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene.
    FbxNode* lCube = CreateCube(lScene, "Cube");

    // Build the node tree.
    FbxNode* lRootNode = lScene->GetRootNode();
    lRootNode->AddChild(lCube);

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
        return 0;
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

    return 0;
}


// Create a cube.
FbxNode* CreateCube(FbxScene* pScene, const char* pName)
{
    // indices of the vertices per each polygon
    static int vtxId[24] = {
        0,1,2,3, // front  face  (Z+)
        1,5,6,2, // right  side  (X+)
        5,4,7,6, // back   face  (Z-)
        4,0,3,7, // left   side  (X-)
        0,4,5,1, // bottom face  (Y-)
        3,2,6,7  // top    face  (Y+)
    };

    // control points
    static Vector4 lControlPoints[8] = {
        { -50.0,  0.0,  50.0, 1.0}, {  50.0,  0.0,  50.0, 1.0},    {  50.0,100.0,  50.0, 1.0},    { -50.0,100.0,  50.0, 1.0}, 
        { -50.0,  0.0, -50.0, 1.0}, {  50.0,  0.0, -50.0, 1.0}, {  50.0,100.0, -50.0, 1.0},    { -50.0,100.0, -50.0, 1.0} 
    };

    // normals
    static Vector4 lNormals[8] = {
        {-0.577350258827209,-0.577350258827209, 0.577350258827209, 1.0}, 
        { 0.577350258827209,-0.577350258827209, 0.577350258827209, 1.0}, 
        { 0.577350258827209, 0.577350258827209, 0.577350258827209, 1.0},
        {-0.577350258827209, 0.577350258827209, 0.577350258827209, 1.0}, 
        {-0.577350258827209,-0.577350258827209,-0.577350258827209, 1.0}, 
        { 0.577350258827209,-0.577350258827209,-0.577350258827209, 1.0},
        { 0.577350258827209, 0.577350258827209,-0.577350258827209, 1.0},
        {-0.577350258827209, 0.577350258827209,-0.577350258827209, 1.0}
    };

    // 2 uvs
    static Vector2 lUVs0[14] = {
        { 0.0, 0.0}, 
        { 1.0, 0.0}, 
        { 0.0, 1.0},
        { 1.0, 1.0}, 
        { 0.0, 2.0},
        { 1.0, 2.0},
        { 0.0, 3.0},
        { 1.0, 3.0},
        { 0.0, 4.0},
        { 1.0, 4.0},
        { 2.0, 0.0},
        { 2.0, 1.0},
        {-1.0, 0.0},
        {-1.0, 1.0}
    };

    static Vector2 lUVs1[14] = {
        { 0.0, 1.0}, 
        { 1.0, 0.0}, 
        { 0.0, 0.0},
        { 1.0, 1.0}
    };

    // indices of the uvs per each polygon
    static int uvsId[24] = {
        0,1,3,2,2,3,5,4,4,5,7,6,6,7,9,8,1,10,11,3,12,0,2,13
    };

    // colors
    static Vector4 lColors[8] = {
        // colors used for the materials
        {1.0, 1.0, 1.0, 1.0},
        {1.0, 1.0, 0.0, 1.0},
        {1.0, 0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 1.0},
        {0.0, 1.0, 0.0, 1.0},
        {0.0, 0.0, 0.0, 1.0},
    };

    // create the main structure.
    FbxMesh* lMesh = FbxMesh::Create(pScene,"");

    // Create control points.
    lMesh->InitControlPoints(8);
    FbxVector4* vertex = lMesh->GetControlPoints();
    memcpy((void*)vertex, (void*)lControlPoints, 8*sizeof(FbxVector4));

    // create the materials.
    /* Each polygon face will be assigned a unique material.
    */
    FbxGeometryElementMaterial* lMaterialElement = lMesh->CreateElementMaterial();
    lMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
    lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    // Create polygons later after FbxGeometryElementMaterial is created. Assign material indices.
    int vId = 0;
    for (int f=0; f<6; f++)
    {
        lMesh->BeginPolygon(f);//Material index.
        for (int v=0; v<4; v++)
            lMesh->AddPolygon(vtxId[vId++]);
        lMesh->EndPolygon();
    }

    // specify normals per control point.
    FbxGeometryElementNormal* lNormalElement = lMesh->CreateElementNormal();
	lNormalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
    lNormalElement->SetReferenceMode(FbxGeometryElement::eDirect);

    for (int n=0; n<8; n++)
        lNormalElement->GetDirectArray().Add(FbxVector4(lNormals[n][0], lNormals[n][1], lNormals[n][2]));


    // create color vertices
    /* We choose to define one color per control point. The other choice would
    have been to use the eByPolygonVertex mapping mode. In this second case,
    the reference mode should become eIndexToDirect.
    */
    FbxGeometryElementVertexColor* lVertexColorElement = lMesh->CreateElementVertexColor();
    lVertexColorElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
    lVertexColorElement->SetReferenceMode(FbxGeometryElement::eDirect);

    for (int c=0; c<8; c++)
        lVertexColorElement->GetDirectArray().Add(FbxColor(lColors[c][0]*.8, lColors[c][1]*0.8, lColors[c][2]*.8, lColors[c][3]*.8));



    // create polygroups. 
    /* We are going to make a first group with the 4 sides.
    And a second group with the top and bottom sides.

    NOTE that the only reference mode allowed is eIndex
    */ 
    FbxGeometryElementPolygonGroup* lPolygonGroupElement = lMesh->CreateElementPolygonGroup();
    lPolygonGroupElement->SetMappingMode(FbxGeometryElement::eByPolygon);
    lPolygonGroupElement->SetReferenceMode(FbxGeometryElement::eIndex);
    lPolygonGroupElement->GetIndexArray().Add(0); // front face assigned to group 0
    lPolygonGroupElement->GetIndexArray().Add(0); // right side assigned to group 0
    lPolygonGroupElement->GetIndexArray().Add(0); // back face assigned to group 0
    lPolygonGroupElement->GetIndexArray().Add(0); // left side assigned to group 0
    lPolygonGroupElement->GetIndexArray().Add(1); // bottom face assigned to group 1
    lPolygonGroupElement->GetIndexArray().Add(1); // top face assigned to group 1


    // create the UV textures mapping.
    FbxTexture* lTexture[3];
    FbxLayeredTexture::EBlendMode lBlendMode[3];

    // On layer 0 all the faces have the same texture
    FbxGeometryElementUV* lUVElement0 = lMesh->CreateElementUV( BACKGROUND_IMAGE_NAME);
	FBX_ASSERT( lUVElement0 != NULL);
    lUVElement0->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    lUVElement0->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    int i;
    for (i = 0; i<14; i++)
        lUVElement0->GetDirectArray().Add(FbxVector2(lUVs0[i][0], lUVs0[i][1]));

    for (i = 0; i<24; i++)
        lUVElement0->GetIndexArray().Add(uvsId[i]);

    // Create the node containing the mesh
    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lMesh);
    lNode->SetShadingMode(FbxNode::eTextureShading);
    

    // Put a different material on each polygon
    FbxSurfacePhong* lMaterial[6];
    for (i = 0; i < 6; i++ )
    {
        FbxString lMaterialName = "material";
        lMaterialName += i;

        lMaterial[i] = FbxSurfacePhong::Create(pScene,lMaterialName.Buffer());

        // Generate primary and secondary colors.
        lMaterial[i]->Emissive.Set(FbxDouble3(0.0, 0.0, 0.0));
        lMaterial[i]->Ambient.Set(FbxDouble3(lColors[i][0], lColors[i][1], lColors[i][2]));
        lMaterial[i]->Diffuse.Set(FbxDouble3(1.0, 1.0, 1.0));
        lMaterial[i]->Specular.Set(FbxDouble3(0.0, 0.0, 0.0));
        lMaterial[i]->TransparencyFactor.Set(0.0);
        lMaterial[i]->Shininess.Set(0.5);
        lMaterial[i]->ShadingModel.Set(FbxString("phong"));

        // add materials to the node
        lNode->AddMaterial(lMaterial[i]);
    }

    // Create textures and texture mappings.
    lTexture[0] = CreateTexture(pScene, BACKGROUND_IMAGE_NAME, BACKGROUND_IMAGE);
    
    lBlendMode[0] = FbxLayeredTexture::eTranslucent;
    
    // create second UVset
    FbxGeometryElementUV* lUVElement1 = lMesh->CreateElementUV( GEO1_IMAGE_NAME);
	FBX_ASSERT( lUVElement1 != NULL);
    lUVElement1->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    lUVElement1->SetReferenceMode(FbxGeometryElement::eIndexToDirect);
    for (i = 0; i <4; i++)
        lUVElement1->GetDirectArray().Add(FbxVector2(lUVs1[i][0], lUVs1[i][1]));

    for (i = 0; i<24; i++)
        lUVElement1->GetIndexArray().Add(uvsId[i%4]);

    lTexture[1] = CreateTexture(pScene, GEO1_IMAGE_NAME, GEO1_IMAGE);
    lBlendMode[1] = FbxLayeredTexture::eModulate;
    


    FbxGeometryElementUV* lUVElement2 = lMesh->CreateElementUV( GEO2_IMAGE_NAME);
	FBX_ASSERT( lUVElement2 != NULL);
    // we re-use the UV mapping.
    *lUVElement2 = *lUVElement0;  

    lTexture[2] = CreateTexture(pScene, GEO2_IMAGE_NAME, GEO2_IMAGE);
    
    lBlendMode[2] = FbxLayeredTexture::eModulate;

    // Because we can only connect one texture to the material propery, we need
    // to use a layered texture object to connect the multiple textures created above
    // we know that 5 faces of the cube will use 2 textures (lTexture[0] and lTexture[1])
    // and only one face uses the three textures. Therefore we need two layered textures.

    FbxLayeredTexture* lLayeredTexture[2];

    // the lLayeredTexure[1] is used for the 5 faces with two textures and lLayeredTexture[0] for
    // the only face that uses the three textures
    lLayeredTexture[0] = FbxLayeredTexture::Create(pScene, "layeredTexture0");
    lLayeredTexture[1] = FbxLayeredTexture::Create(pScene, "layeredTexture1");

    // the first connected texture is the bottom one!
    for (i = 0; i < 3; i++)
    {
        lLayeredTexture[0]->ConnectSrcObject(lTexture[i]);
        lLayeredTexture[0]->SetTextureBlendMode(i, lBlendMode[i]);
        if (i < 2)
        {
            lLayeredTexture[1]->ConnectSrcObject(lTexture[i]);
            lLayeredTexture[1]->SetTextureBlendMode(i, lBlendMode[i]);
        }
    }

    // connect the layered textures to the 6 materials allocated before (material[0] is the
    // material connected to the face 0 of the cube so it is the one that will have lLayeredTexture[0].
    lMaterial[0]->Diffuse.ConnectSrcObject(lLayeredTexture[0]);
    for (i = 1; i < 6; i++)
        lMaterial[i]->Diffuse.ConnectSrcObject(lLayeredTexture[1]);    

    return lNode;
}


FbxTexture* CreateTexture(FbxScene* pScene, const char* name, const char* filename)
{
    FbxFileTexture* lTexture = FbxFileTexture::Create(pScene,name);
    lTexture->SetFileName(filename); // Resource file is in current directory.
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);    
    return lTexture;
}
