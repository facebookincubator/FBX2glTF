/****************************************************************************************

   Copyright (C) 2016 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// Illustrates the use of audio tracks and audio clips.
//
//  1. Create a stack.
//  2. Add the mandatory animation base layer.
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
    // Create one animation stack
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, "Take 001");

    // this stack animation range is limited from 0 to 10 second
    lAnimStack->LocalStop = FBXSDK_TIME_ONE_SECOND * 10;
    lAnimStack->Description = "This is the animation stack description field.";

    // all animation stacks need, at least, one animation layer.
    FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");	// the AnimLayer object name is "Base Layer"
    lAnimStack->AddMember(lAnimLayer);										// add the layer to the stack

	// Now, we create two audio tracks
	FbxAudioLayer* lAudioTrack1 = FbxAudioLayer::Create(pScene, "AudioTrack0");
	lAnimStack->AddMember(lAudioTrack1);
	FbxAudioLayer* lAudioTrack2 = FbxAudioLayer::Create(pScene, "AudioTrack1");
	lAnimStack->AddMember(lAudioTrack2);
	lAudioTrack2->Solo = true;

	// Add a second animation layer
	FbxAnimLayer* lAnimLayer1 = FbxAnimLayer::Create(pScene, "Layer1");
	lAnimStack->AddMember(lAnimLayer1);

	/* At this point the FbxAnimStack object has the following members:
		GetMember(0) = lAnimLayer
		GetMember(1) = lAudioTrack1
		GetMember(2) = lAudioTrack2
		GetMember(3) = lAnimLayer1
	*/
	FBX_ASSERT(lAnimStack->GetMember(0) == lAnimLayer);
	FBX_ASSERT(lAnimStack->GetMember(1) == lAudioTrack1);
	FBX_ASSERT(lAnimStack->GetMember(2) == lAudioTrack2);
	FBX_ASSERT(lAnimStack->GetMember(3) == lAnimLayer1);

	FBX_ASSERT(lAnimStack->GetMember<FbxAnimLayer>(0)  == lAnimLayer);
	FBX_ASSERT(lAnimStack->GetMember<FbxAnimLayer>(1)  == lAnimLayer1);
	FBX_ASSERT(lAnimStack->GetMember<FbxAudioLayer>(0) == lAudioTrack1);
	FBX_ASSERT(lAnimStack->GetMember<FbxAudioLayer>(1) == lAudioTrack2);
	
	// Create the audio clips
	FbxTime t;
	FbxAudio* lAudio1 = FbxAudio::Create(pScene, "Clip1");
	lAudio1->SetFileName("audio2.wav");
	lAudio1->BitRate	= 176000;
	lAudio1->Channels	= 1;
	lAudio1->SampleRate = 22050;
	t.SetMilliSeconds(500);
	lAudio1->Duration	= t;
	lAudio1->ClipIn		= FBXSDK_TIME_ONE_SECOND * 25;
	lAudio1->ClipOut	= FBXSDK_TIME_ONE_SECOND * 40;

	FbxAudio* lAudio2 = FbxAudio::Create(pScene, "Clip2");
	lAudio2->SetFileName("audio1.mp3");
	lAudio2->Duration   = FBXSDK_TIME_ONE_SECOND;
	lAudio2->Channels	= 1;
	lAudio2->BitRate    = 64000;
	lAudio2->SampleRate = 22050;
	lAudio2->ClipIn		= 0;
	lAudio2->ClipOut	= FBXSDK_TIME_ONE_SECOND;

	FbxAudio* lAudio3 = FbxAudio::Create(pScene, "Clip3");
	lAudio3->SetFileName("audio1.mp3");             // same file as Clip2 thus, same attributes
	lAudio3->Duration   = FBXSDK_TIME_ONE_SECOND;
	lAudio3->Channels	= 1;
	lAudio3->BitRate    = 64000;
	lAudio3->SampleRate = 22050;
	lAudio3->ClipIn		= FBXSDK_TIME_ONE_SECOND*5;	
	lAudio3->ClipOut	= FBXSDK_TIME_ONE_SECOND*10;
	t.SetMilliSeconds(333);
	lAudio3->Offset     = t;

	// Attach the audio clips to the audio tracks
	lAudioTrack1->AddMember(lAudio1);

	lAudioTrack2->AddMember(lAudio1);
	lAudioTrack2->AddMember(lAudio2);
	lAudioTrack2->AddMember(lAudio3);

	// animate the volume on the audio clip 2
	// animate the volume on the audio clip 2
	lAudio2->Volume().Set(-3.0);
	FbxAnimCurve* lVolCurve = lAudio2->Volume().GetCurve(lAnimLayer, true);
	if (lVolCurve)
    {
	    FbxTime lTime;
		FbxAnimCurveKey key;

        // add two keys at time 0 sec and 1 sec with values 0 and 100 respectively.
        lVolCurve->KeyModifyBegin();
        for (int i = 0; i < 2; i++)
        {
            lTime.SetSecondDouble((float)i);
            key.Set(lTime, (1-i)*1.0f);
            lVolCurve->KeyAdd(lTime, key);
        }
        lVolCurve->KeyModifyEnd();
    }

    return true;
}

