/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// In this example a scene is created containing a cube and a pyramid. 
//
// The example illustrates two things:
//  1) How to create user properties, attach them to the cube and 
//     animate them.
//  2) How to create a constraint, constraining the pyramid to the cube.
//
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME "UserProperties.fbx"
#define ANIM_STACK_ANIMATE_LIST "Animate Cube List"
#define ANIM_STACK_ANIMATE_CUBE "Animate Cube"
#define ANIM_STACK_ANIMATE_PYRAMID "Animate Pyramid"

// Function prototypes.
FbxNode* CreateCube(FbxScene* pScene, const char* pName);
FbxNode* CreatePyramid(FbxScene* pScene, const char* pName);
void CreateUserProperties(FbxNode *pNode);
void AnimateList(FbxScene* pScene, FbxProperty* pList);
FbxConstraintPosition* CreatePositionConstraint(FbxScene* pScene, FbxNode* pSourceNode, FbxNode* pConstrainedNode);
void AnimateCube(FbxScene* pScene, FbxNode* pNode);
void AnimatePyramid(FbxScene* pScene, FbxNode* pNode);
void AnimateNode(FbxNode* pNode, FbxAnimLayer* pAnimLayer);


int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene. 
    FbxNode* lCube = CreateCube(lScene, "Cube");
    FbxNode* lPyramid = CreatePyramid(lScene, "Pyramid");

    // Build the node tree.
    FbxNode* lRootNode = lScene->GetRootNode();
    lRootNode->AddChild(lCube);
    lRootNode->AddChild(lPyramid);

    // Create the user properties on the Cube.
    CreateUserProperties(lCube);

    //

    // animate the list "MyList"
    FbxProperty p6 = lCube->FindProperty("MyList", false);
    AnimateList(lScene, &p6);

    // Constraint (position constraint) the pyramid to the cube.
    FbxConstraintPosition* lPositionConstraint = (FbxConstraintPosition*)CreatePositionConstraint(lScene, lCube, lPyramid);
	if( lPositionConstraint ) lPositionConstraint->ConnectDstObject(lScene);

    // Animate the cube: the pyramid will follow, because of the position constraint.
    AnimateCube(lScene, lCube);

    // Animate the pyramid: it doesn't actually move, because it is constrained to the immobile cube.
    AnimatePyramid(lScene, lPyramid);

    // Save the scene.

    // The example can take an output file name as an argument.
	const char* lSampleFileName = NULL;
	for( int i = 1; i < argc; ++i )
	{
		if( FBXSDK_stricmp(argv[i], "-test") == 0 ) continue;
		else if( !lSampleFileName ) lSampleFileName = argv[i];
	}
	if( !lSampleFileName ) lSampleFileName = SAMPLE_FILENAME;

	lResult = SaveScene(lSdkManager, lScene, lSampleFileName, lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX ascii (*.fbx)") );
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
    typedef double Vector4[4];
    typedef double Vector2[2];

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
        { -50.0,  0.0,  50.0, 1.0}, {  50.0,  0.0,  50.0, 1.0}, {  50.0,100.0,  50.0, 1.0}, { -50.0,100.0,  50.0, 1.0}, 
        { -50.0,  0.0, -50.0, 1.0}, {  50.0,  0.0, -50.0, 1.0}, {  50.0,100.0, -50.0, 1.0}, { -50.0,100.0, -50.0, 1.0} 
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

    // create the main structure.
    FbxMesh* lMesh = FbxMesh::Create(pScene,"");

    // Create control points.
    lMesh->InitControlPoints(8);
    FbxVector4* vertex = lMesh->GetControlPoints();
    memcpy((void*)vertex, (void*)lControlPoints, 8*sizeof(FbxVector4));

    // create the polygons
    int vId = 0;
    for (int f=0; f<6; f++)
    {
        lMesh->BeginPolygon();
        for (int v=0; v<4; v++)
            lMesh->AddPolygon(vtxId[vId++]);
        lMesh->EndPolygon();
    }

    // specify normals per control point.

    FbxGeometryElementNormal* lNormlElement= lMesh->CreateElementNormal();
    lNormlElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
    lNormlElement->SetReferenceMode(FbxGeometryElement::eDirect);

    for (int n=0; n<8; n++)
        lNormlElement->GetDirectArray().Add(FbxVector4(lNormals[n][0], lNormals[n][1], lNormals[n][2]));


    // Finally we create the node containing the mesh
    FbxNode* lNode = FbxNode::Create(pScene,pName);
    pScene->GetRootNode()->AddChild(lNode);
    lNode->SetNodeAttribute(lMesh);

    return lNode;
}


// Create a pyramid.
FbxNode* CreatePyramid(FbxScene* pScene, const char* pName)
{
    int i, j;
    FbxMesh* lMesh = FbxMesh::Create(pScene,"");

    FbxVector4 lControlPoint0(-50, 0, 50);
    FbxVector4 lControlPoint1(50, 0, 50);
    FbxVector4 lControlPoint2(50, 0, -50);
    FbxVector4 lControlPoint3(-50, 0, -50);
    FbxVector4 lControlPoint4(0, 100, 0);

    FbxVector4 lNormalP0(0, 1, 0);
    FbxVector4 lNormalP1(0, 0.447, 0.894);
    FbxVector4 lNormalP2(0.894, 0.447, 0);
    FbxVector4 lNormalP3(0, 0.447, -0.894);
    FbxVector4 lNormalP4(-0.894, 0.447, 0);

    // Create control points.
    lMesh->InitControlPoints(16);
    FbxVector4* lControlPoints = lMesh->GetControlPoints();

    lControlPoints[0] = lControlPoint0;
    lControlPoints[1] = lControlPoint1;
    lControlPoints[2] = lControlPoint2;
    lControlPoints[3] = lControlPoint3;
    lControlPoints[4] = lControlPoint0;
    lControlPoints[5] = lControlPoint1;
    lControlPoints[6] = lControlPoint4;
    lControlPoints[7] = lControlPoint1;
    lControlPoints[8] = lControlPoint2;
    lControlPoints[9] = lControlPoint4;
    lControlPoints[10] = lControlPoint2;
    lControlPoints[11] = lControlPoint3;
    lControlPoints[12] = lControlPoint4;
    lControlPoints[13] = lControlPoint3;
    lControlPoints[14] = lControlPoint0;
    lControlPoints[15] = lControlPoint4;

    // specify normals per control point.
    FbxGeometryElementNormal* lNormlElement = lMesh->CreateElementNormal();
    lNormlElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
    lNormlElement->SetReferenceMode(FbxGeometryElement::eDirect);

    lNormlElement->GetDirectArray().Add(lNormalP0);
    lNormlElement->GetDirectArray().Add(lNormalP0);
    lNormlElement->GetDirectArray().Add(lNormalP0);
    lNormlElement->GetDirectArray().Add(lNormalP0);
    lNormlElement->GetDirectArray().Add(lNormalP1);
    lNormlElement->GetDirectArray().Add(lNormalP1);
    lNormlElement->GetDirectArray().Add(lNormalP1);
    lNormlElement->GetDirectArray().Add(lNormalP2);
    lNormlElement->GetDirectArray().Add(lNormalP2);
    lNormlElement->GetDirectArray().Add(lNormalP2);
    lNormlElement->GetDirectArray().Add(lNormalP3);
    lNormlElement->GetDirectArray().Add(lNormalP3);
    lNormlElement->GetDirectArray().Add(lNormalP3);
    lNormlElement->GetDirectArray().Add(lNormalP4);
    lNormlElement->GetDirectArray().Add(lNormalP4);
    lNormlElement->GetDirectArray().Add(lNormalP4);

    // Array of polygon vertices.
    int lPolygonVertices[] = { 0, 3, 2, 1,
        4, 5, 6,
        7, 8, 9,
        10, 11, 12,
        13, 14, 15 };

    // Create polygons.

    // Pyramid base.
    lMesh->BeginPolygon();
    for(j = 0; j < 4; j++)
    {
        lMesh->AddPolygon(lPolygonVertices[j]); // Control point index.
    }

    lMesh->EndPolygon ();

    // Pyramid sides.
    for(i = 1; i < 5; i++)
    {
        lMesh->BeginPolygon();

        for(j = 0; j < 3; j++)
        {
            lMesh->AddPolygon(lPolygonVertices[4 + 3*(i - 1) + j]); // Control point index.
        }

        lMesh->EndPolygon ();
    }

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lMesh);
    pScene->GetRootNode()->AddChild(lNode);

    // Translate the pyramid
    FbxVector4 lTranslation(-150, 0, 0, 0);
    lNode->LclTranslation.Set(lTranslation);

    return lNode;
}


void CreateUserProperties(FbxNode *pNode) {
    // Now we create the user properties 
    FbxProperty p1 = FbxProperty::Create(pNode, FbxBoolDT, "MyBooleanProperty", "My Bool");
    FbxProperty p2 = FbxProperty::Create(pNode, FbxFloatDT, "MyRealProperty", "My floating point number");
    FbxProperty p3 = FbxProperty::Create(pNode, FbxColor3DT, "MyColorProperty", "My Color");
    FbxProperty p4 = FbxProperty::Create(pNode, FbxIntDT, "MyInteger", "");
    FbxProperty p5 = FbxProperty::Create(pNode, FbxDouble4DT, "MyVector", "");
    FbxProperty p6 = FbxProperty::Create(pNode, FbxStringListDT, "MyList", "");

    /* 
    NOTE: The properties labels exists only while the property object is in memory.
    The label is not saved in the FBX file. When loading properties from the FBX file
    it will take the same value as the property name.
    */

    // we now fill the properties. All the properties are user properties so we set the
    // correct flag
    p1.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    p2.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    p3.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    p4.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    p5.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
    p6.ModifyFlag(FbxPropertyFlags::eUserDefined, true);

    // let's make MyColorProperty, MyVector and MyList animatables
    p3.ModifyFlag(FbxPropertyFlags::eAnimatable, true);
    p5.ModifyFlag(FbxPropertyFlags::eAnimatable, true);
    p6.ModifyFlag(FbxPropertyFlags::eAnimatable, true);

    // we set the default values
    FbxColor lRed(1.0, 0.0, 0.0);
    p1.Set(false);
    p2.Set(3.33);
    p3.Set(lRed);
    p4.Set(11);
    p5.Set(FbxDouble3(-1.1, 2.2, -3.3));
    p6.Set(2);

    // and some limits
    p4.SetLimits(-5.0, 9.0);
    p5.SetLimits(0.0, 2.1);

    // add elements to the list
    p6.AddEnumValue("one");
    p6.AddEnumValue("two");
    p6.AddEnumValue("three");
    p6.AddEnumValue("Four");
    p6.InsertEnumValue(0, "zero");
}


// Animate the user property given by pList.
void AnimateList(FbxScene* pScene, FbxProperty* pList)
{
    // This is represented by the AnimStack object and in order to be able to add
    // animation curves, we need to define at least one AnimLayer.
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, ANIM_STACK_ANIMATE_LIST);
    FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
    lAnimStack->AddMember(lAnimLayer);

    FbxAnimCurveNode* lFbxFCurveNode = pList->GetCurveNode(lAnimLayer, true);
    if (lFbxFCurveNode)
    {
        FbxTime      lKeyTime;
        FbxAnimCurve*  lFCurve = lFbxFCurveNode->GetCurve(0U);
        if (lFCurve == NULL)
            lFCurve = lFbxFCurveNode->CreateCurve(lFbxFCurveNode->GetName());

        lFCurve->KeyModifyBegin();
        lFCurve->ResizeKeyBuffer(5);

        // One way of setting a keyframe value
        lKeyTime.SetSecondDouble(0.0);
        FbxAnimCurveKey lKey1(lKeyTime, 0.0);
        lKey1.SetInterpolation(FbxAnimCurveDef::eInterpolationConstant);
        lKey1.SetConstantMode(FbxAnimCurveDef::eConstantStandard);
        lFCurve->KeySet(0, lKey1);

        lKeyTime.SetSecondDouble(1.0);
        FbxAnimCurveKey lKey2(lKeyTime, 1.0);
        lKey2.SetInterpolation(FbxAnimCurveDef::eInterpolationConstant);
        lKey2.SetConstantMode(FbxAnimCurveDef::eConstantStandard);
        lFCurve->KeySet(1, lKey2);

        // an other way of setting a keyframe value
        lKeyTime.SetSecondDouble(2.0);

        // the cast to TangentMode is intended
        lFCurve->KeySet(2, lKeyTime, 2.0, FbxAnimCurveDef::eInterpolationConstant, (FbxAnimCurveDef::ETangentMode)FbxAnimCurveDef::eConstantStandard);

        lKeyTime.SetSecondDouble(3.0);
        lFCurve->KeySet(3, lKeyTime, 3.0, FbxAnimCurveDef::eInterpolationConstant, (FbxAnimCurveDef::ETangentMode)FbxAnimCurveDef::eConstantStandard);

        lKeyTime.SetSecondDouble(4.0);
        lFCurve->KeySet(4, lKeyTime, 0.0, FbxAnimCurveDef::eInterpolationConstant, (FbxAnimCurveDef::ETangentMode)FbxAnimCurveDef::eConstantStandard);

        lFCurve->KeyModifyEnd();
    }
}


// Create a position constraint whith pSourceNode as source node and pConstraintedNode as constrained node.
FbxConstraintPosition* CreatePositionConstraint(FbxScene* pScene, FbxNode* pSourceNode, FbxNode* pConstrainedNode)
{
    FbxConstraintPosition *lPositionConstraint = FbxConstraintPosition::Create(pScene,"Position");

    // set constrained object
    lPositionConstraint->SetConstrainedObject(pConstrainedNode);

    // set source
    lPositionConstraint->AddConstraintSource(pSourceNode, 100.0);

    // Constrain the position in X, Y and Z
	lPositionConstraint->AffectX = true;
    lPositionConstraint->AffectY = true;
    lPositionConstraint->AffectZ = true;

    // keep offset between source and constrained object
    FbxVector4 lPositionSource;
    FbxVector4 lPositionConstrainedObj;
    lPositionSource = pSourceNode->LclTranslation.Get();
    lPositionConstrainedObj = pConstrainedNode->LclTranslation.Get();
    FbxVector4 lOffset = lPositionConstrainedObj - lPositionSource;
	lPositionConstraint->Translation = FbxDouble3(lOffset);

    // activate property
    FbxProperty lActive = lPositionConstraint->FindProperty("Active", false);
    lActive.Set(true);


    return lPositionConstraint;
}


// Animate the cube by translating it in X, Y and Z.
void AnimateCube(FbxScene* pScene, FbxNode* pNode)
{
    // This is represented by the AnimStack object and in order to be able to add
    // animation curves, we need to define at least one AnimLayer.
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, ANIM_STACK_ANIMATE_CUBE);
    FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
    lAnimStack->AddMember(lAnimLayer);

    AnimateNode(pNode, lAnimLayer);

}


void AnimatePyramid(FbxScene* pScene, FbxNode* pNode)
{
    // This is represented by the AnimStack object and in order to be able to add
    // animation curves, we need to define at least one AnimLayer.
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, ANIM_STACK_ANIMATE_PYRAMID);
    FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
    lAnimStack->AddMember(lAnimLayer);

    AnimateNode(pNode, lAnimLayer);
}


// Animate a given node.
void AnimateNode(FbxNode* pNode, FbxAnimLayer* pAnimLayer)
{
    FbxAnimCurve* lCurveX = NULL;
    FbxAnimCurve* lCurveY = NULL;
    FbxAnimCurve* lCurveZ = NULL;
    FbxTime lTime;
    int lKeyIndex = 0;

    // The CurveNode must be created in order to access the AnimCurves
    pNode->LclTranslation.GetCurveNode(pAnimLayer, true);
    lCurveX = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
    lCurveY = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
    lCurveZ = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

    if (lCurveX)
    {
        lCurveX->KeyModifyBegin();

        lTime.SetSecondDouble(0.0);
        lKeyIndex = lCurveX->KeyAdd(lTime);
        lCurveX->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(1.0);
        lKeyIndex = lCurveX->KeyAdd(lTime);
        lCurveX->KeySet(lKeyIndex, lTime, 100.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurveX->KeyAdd(lTime);
        lCurveX->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(3.0);
        lKeyIndex = lCurveX->KeyAdd(lTime);
        lCurveX->KeySet(lKeyIndex, lTime, -100.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(4.0);
        lKeyIndex = lCurveX->KeyAdd(lTime);
        lCurveX->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lCurveX->KeyModifyEnd();
    }

    if (lCurveY)
    {
        lCurveY->KeyModifyBegin();

        lTime.SetSecondDouble(2.0);
        lKeyIndex = lCurveY->KeyAdd(lTime);
        lCurveY->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(3.0);
        lKeyIndex = lCurveY->KeyAdd(lTime);
        lCurveY->KeySet(lKeyIndex, lTime, 100.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(4.0);
        lKeyIndex = lCurveY->KeyAdd(lTime);
        lCurveY->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(5.0);
        lKeyIndex = lCurveY->KeyAdd(lTime);
        lCurveY->KeySet(lKeyIndex, lTime, -100.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(6.0);
        lKeyIndex = lCurveY->KeyAdd(lTime);
        lCurveY->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lCurveY->KeyModifyEnd();
    }

    if (lCurveZ)
    {
        lCurveZ->KeyModifyBegin();

        lTime.SetSecondDouble(5.0);
        lKeyIndex = lCurveZ->KeyAdd(lTime);
        lCurveZ->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(6.0);
        lKeyIndex = lCurveZ->KeyAdd(lTime);
        lCurveZ->KeySet(lKeyIndex, lTime, 100.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(7.0);
        lKeyIndex = lCurveZ->KeyAdd(lTime);
        lCurveZ->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(8.0);
        lKeyIndex = lCurveZ->KeyAdd(lTime);
        lCurveZ->KeySet(lKeyIndex, lTime, -100.0, FbxAnimCurveDef::eInterpolationCubic);

        lTime.SetSecondDouble(9.0);
        lKeyIndex = lCurveZ->KeyAdd(lTime);
        lCurveZ->KeySet(lKeyIndex, lTime, 0.0, FbxAnimCurveDef::eInterpolationCubic);

        lCurveZ->KeyModifyEnd();
    }

}
