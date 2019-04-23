/****************************************************************************************

Copyright (C) 2015 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _SCENE_CONTEXT_H
#define _SCENE_CONTEXT_H

#include "GlFunctions.h"

class DrawText;

// This class is responsive for loading files and recording current status as
// a bridge between window system such as GLUT or Qt and a specific FBX scene.
class SceneContext
{
public:
    enum Status
    {
        UNLOADED,               // Unload file or load failure;
        MUST_BE_LOADED,         // Ready for loading file;
        MUST_BE_REFRESHED,      // Something changed and redraw needed;
        REFRESHED               // No redraw needed.
    };
    Status GetStatus() const { return mStatus; }

    // Initialize with a .FBX, .DAE or .OBJ file name and current window size.
    SceneContext(const char * pFileName, int pWindowWidth, int pWindowHeight, bool pSupportVBO);
    ~SceneContext();

    // Return the FBX scene for more informations.
    const FbxScene * GetScene() const { return mScene; }
    // Load the FBX or COLLADA file into memory.
    bool LoadFile();

    // The time period for one frame.
    const FbxTime GetFrameTime() const { return mFrameTime; }

    // Call this method when redraw is needed.
    bool OnDisplay();
    // Call this method when window size is changed.
    void OnReshape(int pWidth, int pHeight);
    // Call this method when keyboard input occurs.
    void OnKeyboard(unsigned char pKey);
    // Call this method when mouse buttons are pushed or released.
    void OnMouse(int pButton, int pState, int pX, int pY);
    // Call this method when mouse is moved.
    void OnMouseMotion(int pX, int pY);
    // Call this method when timer is finished, for animation display.
    void OnTimerClick() const;

    // Methods for creating menus.
    // Get all the cameras in current scene, including producer cameras.
    const FbxArray<FbxNode *> & GetCameraArray() const { return mCameraArray; }
    // Get all the animation stack names in current scene.
    const FbxArray<FbxString *> & GetAnimStackNameArray() const { return mAnimStackNameArray; }
    // Get all the pose in current scene.
    const FbxArray<FbxPose *> & GetPoseArray() const { return mPoseArray; }

    // The input index is corresponding to the array returned from GetAnimStackNameArray.
    bool SetCurrentAnimStack(int pIndex);
    // Set the current camera with its name.
    bool SetCurrentCamera(const char * pCameraName);
    // The input index is corresponding to the array returned from GetPoseArray.
    bool SetCurrentPoseIndex(int pPoseIndex);
    // Set the currently selected node from external window system.
    void SetSelectedNode(FbxNode * pSelectedNode);
    // Set the shading mode, wire-frame or shaded.
    void SetShadingMode(ShadingMode pMode);

    // Pause the animation.
    void SetPause(bool pPause) { mPause = pPause; }
    // Check whether the animation is paused.
    bool GetPause() const { return mPause; }


    enum CameraZoomMode
    {
        ZOOM_FOCAL_LENGTH,
        ZOOM_POSITION
    };
    CameraZoomMode  GetZoomMode()        { return mCameraZoomMode; }
    void            SetZoomMode( CameraZoomMode pZoomMode);       

private:
    // Display information about current status in the left-up corner of the window.
    void DisplayWindowMessage();
    // Display a X-Z grid.
    void DisplayGrid(const FbxAMatrix & pTransform);

    enum CameraStatus
    {
        CAMERA_NOTHING,
        CAMERA_ORBIT,
        CAMERA_ZOOM,
        CAMERA_PAN
    };

    const char * mFileName;
    mutable Status mStatus;
    mutable FbxString mWindowMessage;

    FbxManager * mSdkManager;
    FbxScene * mScene;
    FbxImporter * mImporter;
    FbxAnimLayer * mCurrentAnimLayer;
    FbxNode * mSelectedNode;

    int mPoseIndex;
    FbxArray<FbxString*> mAnimStackNameArray;
    FbxArray<FbxNode*> mCameraArray;
    FbxArray<FbxPose*> mPoseArray;

    mutable FbxTime mFrameTime, mStart, mStop, mCurrentTime;
	mutable FbxTime mCache_Start, mCache_Stop;

    // Data for camera manipulation
    mutable int mLastX, mLastY;
    mutable FbxVector4 mCamPosition, mCamCenter;
    mutable double mRoll;
    mutable CameraStatus mCameraStatus;

    bool mPause;
    ShadingMode mShadingMode;
    bool mSupportVBO;

    //camera zoom mode
    CameraZoomMode mCameraZoomMode;

    int mWindowWidth, mWindowHeight;
    // Utility class for draw text in OpenGL.
    DrawText * mDrawText;
};

// Initialize GLEW, must be called after the window is created.
bool InitializeOpenGL();

#endif // _SCENE_CONTEXT_H

