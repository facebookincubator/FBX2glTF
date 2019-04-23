/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// Illustrates the use of animation stacks, layers, curvenodes and curves.
//
//  1. Create a stack.
//  2. Add the mandatory base layer.
//  3. Show how to set the layer blend mode bypass.
//  4. Animate the layers's weight property.
//  5. Show how to use AddChannel and ResetChannels.
//  6. Create a 3 components curvenode and animate two of the three channels.
//  7. Add the curve node to the animation layer.
//  8. Create curves and add keys.
//  9. Create an extra layer with a different blend mode.
// 10. Show how to share the same animation curve on two layers (different channels)
// 11. Evaluate the overall result.
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>
#include "../Common/Common.h"

// Function prototypes.
bool CreateScene(FbxManager* pSdkManager, FbxScene* pScene);

int main(int /*argc*/, char** /*argv*/)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene.
    if( !CreateScene(lSdkManager, lScene) )
    {
        FBXSDK_printf("\n\nAn error occurred while creating the scene...\n");
        DestroySdkObjects(lSdkManager, false);
        return 0;
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, true);

    return 0;
}

bool CreateScene(FbxManager* /*pSdkManager*/, FbxScene* pScene)
{
    int i;
    FbxTime lTime;
    FbxAnimCurveKey key;
    FbxAnimCurve* lCurve = NULL;

    // Create one animation stack
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, "Stack001");

    // this stack animation range is limited from 0 to 1 second
    lAnimStack->LocalStop = FBXSDK_TIME_ONE_SECOND;
    lAnimStack->Description = "This is the animation stack description field.";

    // all animation stacks need, at least, one layer.
    FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");	// the AnimLayer object name is "Base Layer"
    lAnimStack->AddMember(lAnimLayer);											// add the layer to the stack

    // Set and get the blend mode bypass of the layer
    bool val;
    lAnimLayer->SetBlendModeBypass(eFbxTypeCount, true);       // set the bypass to all the datatypes.
    val = lAnimLayer->GetBlendModeBypass(eFbxBool);           // val = true
    lAnimLayer->SetBlendModeBypass(eFbxBool, false);          // overwrite just for the bool datatype.
    val = lAnimLayer->GetBlendModeBypass(eFbxBool);           // val = false
    val = lAnimLayer->GetBlendModeBypass(eFbxChar);           // val = true
    val = lAnimLayer->GetBlendModeBypass(eFbxDateTime);        // val = true
    val = lAnimLayer->GetBlendModeBypass((EFbxType)-1);     // invalid type, val = false
    val = lAnimLayer->GetBlendModeBypass((EFbxType)120);    // invalid type (>MAX_TYPES), val = false


    // we want to animate the layer's weight property.
    FbxAnimCurveNode* wcn = lAnimLayer->CreateCurveNode(lAnimLayer->Weight);
    if (wcn)
    {
        // the curve node from the Weight property already contains 1 channel (Weight).
        i = wcn->GetChannelsCount();                            // i = 1

        // Now, let's add a second channel to the animation node. Note that this code
        // is useless and has only been provided to show the usage of the AddChannel and
        // ResetChannels
        bool ret;
        ret = wcn->AddChannel<int>("MyAddedIntChannel", 99);    // this call will succed
        i = wcn->GetChannelsCount();                            // i = 2
        ret = wcn->AddChannel<int>("MyAddedIntChannel", 10);    // this call will fail, since the channel already exists.
        i = wcn->GetChannelsCount();                            // i = 2
        wcn->ResetChannels();                                   // remove any added channels
        i = wcn->GetChannelsCount();                            // i = 1
    }

    // get the Weight curve (and create it if it does not exist, wich is the case!)
    lCurve = lAnimLayer->Weight.GetCurve(lAnimLayer, true);
    if (lCurve)
    {
        // add two keys at time 0 sec and 1 sec with values 0 and 100 respectively.
        lCurve->KeyModifyBegin();
        for (i = 0; i < 2; i++)
        {
            lTime.SetSecondDouble((float)i);
            key.Set(lTime, i*100.0f);
            lCurve->KeyAdd(lTime, key);
        }
        lCurve->KeyModifyEnd();
    }

    //
    // now create a 3 components curvenode and animate two of the three channels.
    //
    // first, we need a "dummy" property so we can call the CreateTypedCurveNode
    FbxProperty p = FbxProperty::Create(pScene, FbxDouble3DT, "Vector3Property");
    p.Set(FbxDouble3(1.1, 2.2, 3.3));
    FbxAnimCurveNode* lCurveNode = FbxAnimCurveNode::CreateTypedCurveNode(p, pScene);

    // let's make sure the curveNode is added to the animation layer.
    lAnimLayer->AddMember(lCurveNode);

    // and to the "Vector3Property" since CreateTypedCurveNode does not make any connection
    p.ConnectSrcObject(lCurveNode);
    
	//Example of channel get value:
    //double v1 = lCurveNode->GetChannelValue<double>(0U, 0.0);   // v1 = 1.1
    //float  v2 = lCurveNode->GetChannelValue<float> (1U, 0.0f);  // v2 = 2.2
    //int    v3 = lCurveNode->GetChannelValue<int>   (2U, 0);     // v3 = 3

    //
    // create two free curves (not connected to anything)
    //

    // first curve
    lCurve = FbxAnimCurve::Create(pScene, "curve1");
    if (lCurve)
    {
        // add two keys at time 0 sec and 1 sec with values 0 and 10 respectively.
        lCurve->KeyModifyBegin();
        for (i = 0; i < 2; i++)
        {
            lTime.SetSecondDouble((float)i);
            key.Set(lTime, i*10.0f);
            lCurve->KeyAdd(lTime, key);
        }
        lCurve->KeyModifyEnd();
    }

    // connect it to the second channel
    lCurveNode->ConnectToChannel(lCurve, 1);

    // second curve
    lCurve = FbxAnimCurve::Create(pScene, "curve2");
    if (lCurve)
    {
        // add three keys at time 1, 2 and 3 sec with values 3.33, 6.66 and 9.99 respectively
        lCurve->KeyModifyBegin();
        for (i = 1; i < 4; i++)
        {
            lTime.SetSecondDouble((float)i);
            key.Set(lTime, i*3.33f);
            lCurve->KeyAdd(lTime, key);
        }
        lCurve->KeyModifyEnd();
    }
    // connect it to the third channel
    lCurveNode->ConnectToChannel(lCurve, "Z"); // for backward compatibility, string identifier are still 
    // allowed for the X,Y,Z and W components or "0", "1", ... "9", "A", "B", ... "F" for the Matrix44 datatype

    
    // ======================================================================
    //
    // Add a second animation layer and evaluate using the FbxAnimEvaluator
    //
    // ======================================================================
    lAnimLayer = FbxAnimLayer::Create(pScene, "Layer2"); 
    lAnimStack->AddMember(lAnimLayer);  

    // get the number of animation layers in the stack
    // int nbLayers = lAnimStack->GetMemberCount<FbxAnimLayer>();  // nblayers = 2
    lAnimLayer = lAnimStack->GetMember<FbxAnimLayer>(1);      // get the second layer

    // set its blend mode to Additive
    lAnimLayer->BlendMode.Set(FbxAnimLayer::eBlendAdditive);

    // Now, let's animate the first channel of the "Vector3Property" (remember, we animated the second and
    // third ones on the base layer - when we connected "curve1" and "curve2" on lCurveNode above)
    // but first, make sure the property is animatable otherwise the creation of the curveNode is prohibited.
    p.ModifyFlag(FbxPropertyFlags::eAnimatable, true);
    lCurveNode = p.GetCurveNode(lAnimLayer, true); // create it since it does not exist yet

    // use "curve2" to animate it on channel 0
    lCurveNode->ConnectToChannel(lCurve, 0U);

    // and set the other two channels values
    lCurveNode->SetChannelValue<double>(1U, 5.0);
    lCurveNode->SetChannelValue<double>(2U, 0.0);

    // evaluate the "Vector3Property" value at three different times
    // with the use of the FbxAnimEvaluator so we take into account the two layers

    // make sure the evaluator is using the correct context (context == animstack)
    pScene->SetCurrentAnimationStack(lAnimStack);
    for (i = 0; i < 3; i++)
    {
        lTime.SetSecondDouble((float)i);
		FbxDouble3 value = p.EvaluateValue<FbxDouble3>(lTime);
    }

    /* 
       The base layer has a weight curve:

              Time     |     0       |       1        |      2       |
        Weight         +-------------+----------------+--------------| 
           Base Layer  |     0.0     |     100.0%     |    (100.0%)  | 
           Layer2      |   <100.0%>  |    <100.0%>    |    <100.0%>  |
                       +-------------+----------------+--------------|
        
        () Querying values outside the first and/or last keys in a curve will return
           the first/last key defined.
        <> Since it has never been set, it defaults to the multiplication neutral element (in percent).
        
        At the specified times each channel value on their respective layers is:

             Time     |     0       |       1        |      2       |
        Channel       +-------------+----------------+--------------|
            0  Base   |     0*      |       1.1*     |    1.1*      |  
               Layer2 |    (3.33)   |      3.33      |    6.66      | (curve2)
                      +-------------+----------------+--------------|
            1  Base   |     0*      |      10.0*     |    (10.0)    | (curve1)
               Layer2 |    5.0      |       5.0      |     5.0      |
                      +-------------+----------------+--------------|
            2  Base   |     0*      |      3.3*      |    6.66*     | (curve2)
               Layer2 |    0.0      |      0.0       |    0.0       |
                      +-------------+----------------+--------------|

        * key (or property, if not animated) value multiplied by the weight.
        () same as value at time 1 since there is no key here.

        therefore, considering that the second animation layer's blend mode is set 
        to additive, the evaluated values for v at 0, 1 and 2 seconds are:
        
          time  |    0        |       1        |      2       |
       v        +-------------+----------------+--------------|
            0   |  3.33       |     4.43       |    7.76      |
            1   |  5.0        |    15.00       |   15.00      |
            2   |  0.0        |     3.33       |    6.66      |
                +-------------+----------------+--------------| 
    */          
    return true;
}

