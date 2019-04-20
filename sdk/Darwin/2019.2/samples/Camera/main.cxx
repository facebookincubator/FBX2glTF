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
//        1) Get a camera from a scene.
//        2) Inspect the camera attributes and show them.
//        3) Create my own camera.
//        4) Inspect the camera attributes and show them.
//        5) Modify the new camera aspect.
//        6) Inspect the camera attributes and show them.
//
//steps:
// 1. initialize FBX sdk object.
// 2. load fbx scene from the specified file.
// 3. Get root node of the scene.
// 4. Recursively traverse each node in the scene.
// 5. Detect and get camera from node attribute type.
// 6. Inspect the camera attributes and show the result. 
// 7. Create camera using NTSC format.
// 8. Inspect the camera attributes and show the result.
// 9. Reset the new camera's aspect attributes with NTSC standard.
// 10. Inspect the camera attributes and show the result.
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "../Common/Common.h"

#define SAMPLE_FILENAME "Camera.fbx"

#define GET_MAX(a, b)   (a) < (b) ?  (b) : (a)

double ComputePixelRatio( double pWidth, double pHeight, double pScreenRatio);

void DisplayCameraInfo(FbxNode* pNode);

FbxCamera* CreateMyCamera(FbxScene* pScene);

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
        if( lScene)
        {
            //get root node of the fbx scene
            FbxNode* lRootNode = lScene->GetRootNode();
            
            FBXSDK_printf("\n\rInspect camera's attributes from the scene...\n");
            //This function illustrates how to get camera info from scene.
            if( gVerbose ) DisplayCameraInfo(lRootNode);

            //create my own camera set
            FbxCamera* lMyCamera = CreateMyCamera(lScene);
            FBXSDK_printf("\n\rInspect camera's attributes from the scene...\n");
            if( gVerbose ) DisplayCameraInfo(lRootNode);
            //modify the camera's aspect attributes
            lMyCamera->SetAspect( FbxCamera::eFixedResolution, 640, 480);
            //Though we set the aspect attributes according to the NTSC standard, camera format is modified as eCUSTOM_FORMAT.
            FBXSDK_printf("\n\rInspect camera's attributes from the scene...\n");
            if( gVerbose ) DisplayCameraInfo(lRootNode);
        }
        else
        {
            FBXSDK_printf("\n\nNull scene...\n");
        }

    }

    //Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);
    return 0;
}

//This function illustrates how to get camera info from scene.
void DisplayCameraInfo(FbxNode* pNode)
{
    if(!pNode)
        return;

    //get camera
    FbxCamera* lCamera = pNode->GetCamera();

    if( lCamera != NULL)
    {
        FBXSDK_printf( "/*-----------------Camera: %s----------------------*/\n\r", lCamera->GetName());
        //get camera's information

        double lResolutionHeight = 0.0;
        double lResolutionWidth = 0.0;
        //get camera format
        FbxCamera::EFormat lCameraFormat = lCamera->GetFormat();
        //camera using specific format has a given resolution(aspect) width and height.
        /*
        resolution width            resolution height      
        eD1_NTSC                        720                         486 
        eNTSC                           640                         480  
        ePAL                            570                         486
        eD1_PAL                         720                         576
        eHD                             1980                        1080
        e640x480                        640                         480
        e320x200                        320                         200
        e320x240                        320                         240
        e128x128                        128                         128
        eFULL_SCREEN                    1280                        1024
        */
        switch( lCameraFormat)
        {
        case FbxCamera::eCustomFormat:
            FBXSDK_printf( "Camera format is customized.\n");
            break;
        case FbxCamera::eNTSC:
            FBXSDK_printf( "Camera format is NTSC.\n");
            lResolutionWidth = 640;
            lResolutionHeight = 480;
            break;
        case FbxCamera::eD1NTSC:
            FBXSDK_printf( "Camera format is D1 NTSC.\n");
            lResolutionWidth = 720;
            lResolutionHeight = 486;
            break;
        case FbxCamera::ePAL:
            FBXSDK_printf( "Camera format is PAL.\n");
            lResolutionWidth = 570;
            lResolutionHeight = 486;
            break;
        case FbxCamera::eD1PAL:
            FBXSDK_printf( "Camera format is D1 PAL.\n");
            lResolutionWidth = 720;
            lResolutionHeight = 576;
            break;
        case FbxCamera::eHD:
            FBXSDK_printf( "Camera format is HD.\n");
            lResolutionWidth = 1980;
            lResolutionHeight = 1080;
            break;
        case FbxCamera::e640x480:
            FBXSDK_printf( "Camera format is 640x480.\n");
            lResolutionWidth = 640;
            lResolutionHeight = 480;
            break;
        case FbxCamera::e320x200:
            FBXSDK_printf( "Camera format is 320x200.\n");
            lResolutionWidth = 320;
            lResolutionHeight = 200;
            break;
        case FbxCamera::e320x240:
            FBXSDK_printf( "Camera format is 320x240.\n");
            lResolutionWidth = 320;
            lResolutionHeight = 240;
            break;
        case FbxCamera::e128x128:
            FBXSDK_printf( "Camera format is 128x128.\n");
            lResolutionWidth = 128;
            lResolutionHeight = 128;
            break;
        case FbxCamera::eFullscreen:
            FBXSDK_printf( "Camera format is full screen.\n");
            lResolutionWidth = 1280;
            lResolutionHeight = 1024;
            break;
        default:
            FBXSDK_printf( "Unknown camera format.\n");
            break;
        }

        //get camera's inherent properties
        {
            //get aspect height
            double lAspectHeight = lCamera->AspectHeight.Get();
            //get aspect width
            double lAspectWidth = lCamera->AspectWidth.Get();
            //get aspect ratio
            double lPixeltRatio = lCamera->GetPixelRatio();
            //verify the pixel ratio
            double lScreenRatio     = 4 / 3; // default screen ratio is 4 : 3;
            if( lCamera->GetFormat() == FbxCamera::eHD)
            {
                lScreenRatio    = 16 / 9; // in HD mode, screen ratio is 16 : 9;
            }
            double lInspectedPixelRatio = ComputePixelRatio( lResolutionWidth, lResolutionHeight, lScreenRatio);
            if( lPixeltRatio != lInspectedPixelRatio)
            {
                FBXSDK_printf( "Camera pixel ratio is not right.\n\rRevise the ratio: %lf to %lf.\n\n", lPixeltRatio, lInspectedPixelRatio);
                lCamera->PixelAspectRatio.Set( lInspectedPixelRatio);
                lPixeltRatio = lInspectedPixelRatio;
            }
            else
            {
                FBXSDK_printf( "camera pixel ratio is %lf.\n", lPixeltRatio);
            }
            //get aspect ratio mode
            /*
            If the ratio mode is eWINDOW_SIZE, both width and height values aren't relevant.
            If the ratio mode is eFIXED_RATIO, the height value is set to 1.0 and the width value is relative to the height value.
            If the ratio mode is eFIXED_RESOLUTION, both width and height values are in pixels.
            If the ratio mode is eFIXED_WIDTH, the width value is in pixels and the height value is relative to the width value.
            If the ratio mode is eFIXED_HEIGHT, the height value is in pixels and the width value is relative to the height value.
            */
            FbxCamera::EAspectRatioMode lCameraAspectRatioMode = lCamera->GetAspectRatioMode();
            switch( lCameraAspectRatioMode)
            {
            case FbxCamera::eWindowSize:
                FBXSDK_printf( "Camera aspect ratio mode is window size.\n");
                break;
            case FbxCamera::eFixedRatio:
                FBXSDK_printf( "Camera aspect ratio mode is fixed ratio.\n");
                break;
            case FbxCamera::eFixedResolution:
                FBXSDK_printf( "Camera aspect ratio mode is fixed resolution.\n");
                break;
            case FbxCamera::eFixedWidth:
                FBXSDK_printf( "Camera aspect ratio mode is fixed width.\n");
                break;
            case FbxCamera::eFixedHeight:
                FBXSDK_printf( "Camera aspect ratio mode is fixed height.\n");
                break;
            default:
                FBXSDK_printf( "Unknown camera aspect ratio mode.\n");
                break;

            }
            //inspect the aspect width and height
            if( lCameraFormat != FbxCamera::eCustomFormat && lCameraAspectRatioMode != FbxCamera::eWindowSize)
            {
                double lInspectedAspectHeight = 0.0;
                double lInspectedAspectWidth = 0.0;
                switch( lCameraAspectRatioMode)
                {
		        default:
		            break;
                case FbxCamera::eFixedRatio:
                    if( lAspectHeight != 1.0)
                    {
                        FBXSDK_printf( "Camera aspect height should be 1.0 in fixed ratio mode.\n\rRevise the height: %lf to 1.0.\n\n", lAspectHeight);
                        lCamera->AspectHeight.Set( 1.0);
                        lAspectHeight = 1.0;
                    }                    
                    lInspectedAspectWidth = lResolutionWidth / lResolutionHeight * lPixeltRatio;
                    if( lAspectWidth != lInspectedAspectWidth)
                    {
                        FBXSDK_printf( "Camera aspect width is not right.\n\rRevise the width: %lf to %lf.\n\n", lAspectWidth, lInspectedAspectWidth);
                        lCamera->AspectWidth.Set( lInspectedAspectWidth);
                        lAspectWidth = lInspectedAspectWidth;
                    }
                    break;
                case FbxCamera::eFixedResolution:
                    if( lAspectWidth != lResolutionWidth)
                    {
                        FBXSDK_printf( "Camera aspect width is not right.\n\rRevise the width: %lf to %lf.\n\n", lAspectWidth, lResolutionWidth);
                        lCamera->AspectWidth.Set( lResolutionWidth);
                        lAspectWidth = lResolutionWidth;
                    }
                    if( lAspectHeight != lResolutionHeight)
                    {
                        FBXSDK_printf( "Camera aspect height is not right.\n\rRevise the height: %lf to %lf.\n\n", lAspectHeight, lResolutionHeight);
                        lCamera->AspectHeight.Set( lResolutionHeight);
                        lAspectHeight = lResolutionHeight;
                    }  
                    break;
                case FbxCamera::eFixedWidth:
                    lInspectedAspectHeight = lResolutionHeight / lResolutionWidth;
                    if( lAspectHeight != lInspectedAspectHeight)
                    {
                        FBXSDK_printf( "Camera aspect height is not right.\n\rRevise the height: %lf to %lf.\n\n", lAspectHeight, lInspectedAspectHeight);
                        lCamera->AspectHeight.Set( lInspectedAspectHeight);
                        lAspectHeight = lInspectedAspectHeight;
                    }
                    break;
                case FbxCamera::eFixedHeight:
                    lInspectedAspectWidth = lResolutionWidth / lResolutionHeight;
                    if( lAspectWidth != lInspectedAspectWidth)
                    {
                        FBXSDK_printf( "Camera aspect width is not right.\n\rRevise the width: %lf to %lf.\n\n", lAspectWidth, lInspectedAspectWidth);
                        lCamera->AspectHeight.Set( lInspectedAspectWidth);
                        lAspectHeight = lInspectedAspectWidth;
                    }
                    break;

                }
            }
            FBXSDK_printf( "Camera aspect width: %lf .\n Camera aspect height: %lf .\n", lAspectWidth, lAspectHeight);

            //inspect aperture width and height
            double lInspectedApertureHeight = 0.0;
            double lInspectedApertureWidth = 0.0;
            //get camera's aperture format
            FbxCamera::EApertureFormat lApertureFormat = lCamera->GetApertureFormat();
            //camera using specific aperture format has a given aperture width and height.
            /*
            aperture width              aperture width             aperture height      (unit: inch)
            e16MM_THEATRICAL            0.4040                      0.2950
            eSUPER_16MM                 0.4930                      0.2920
            e35MM_ACADEMY               0.8640                      0.6300
            e35MM_TV_PROJECTION         0.8160                      0.6120
            e35MM_FULL_APERTURE         0.9800                      0.7350
            e35MM_185_PROJECTION        0.8250                      0.4460
            e35MM_ANAMORPHIC            0.8640                      0.7320
            e70MM_PROJECTION            2.0660                      0.9060
            eVISTAVISION                1.4850                      0.9910
            eDYNAVISION                 2.0800                      1.4800
            eIMAX                       2.7720                      2.0720
            */
            switch( lApertureFormat)
            {
            case FbxCamera::eCustomAperture:
                FBXSDK_printf( "Camera aperture format is customized.\n");
                break;
            case FbxCamera::e16mmTheatrical:
                FBXSDK_printf( "Camera aperture format is 16mm theatrical.\n");
                lInspectedApertureWidth = 0.4040;
                lInspectedApertureHeight = 0.2950;
                break;
            case FbxCamera::eSuper16mm:
                FBXSDK_printf( "Camera aperture format is super 16mm.\n");
                lInspectedApertureWidth = 0.4930;
                lInspectedApertureHeight = 0.2920;
                break;
            case FbxCamera::e35mmAcademy:
                FBXSDK_printf( "Camera aperture format is 35mm academy.\n");
                lInspectedApertureWidth = 0.8640;
                lInspectedApertureHeight = 0.6300;
                break;
            case FbxCamera::e35mmTVProjection:
                FBXSDK_printf( "Camera aperture format is 35mm TV projection.\n");
                lInspectedApertureWidth = 0.8160;
                lInspectedApertureHeight = 0.6120;
                break;
            case FbxCamera::e35mmFullAperture:
                FBXSDK_printf( "Camera aperture format is 35mm full projection.\n");
                lInspectedApertureWidth = 0.9800;
                lInspectedApertureHeight = 0.7350;
                break;
            case FbxCamera::e35mm185Projection:
                FBXSDK_printf( "Camera aperture format is 35mm 185 projection.\n");
                lInspectedApertureWidth = 0.8250;
                lInspectedApertureHeight = 0.4460;
                break;
            case FbxCamera::e35mmAnamorphic:
                FBXSDK_printf( "Camera aperture format is 35mm anamorphic.\n");
                lInspectedApertureWidth = 0.8640;
                lInspectedApertureHeight = 0.7320;
                break;
            case FbxCamera::e70mmProjection:
                FBXSDK_printf( "Camera aperture format is 70mm projection.\n");
                lInspectedApertureWidth = 2.0660;
                lInspectedApertureHeight = 0.9060;
                break;
            case FbxCamera::eVistaVision:
                FBXSDK_printf( "Camera aperture format is vistavision.\n");
                lInspectedApertureWidth = 1.4850;
                lInspectedApertureHeight = 0.9910;
                break;
            case FbxCamera::eDynaVision:
                FBXSDK_printf( "Camera aperture format is dynavision.\n");
                lInspectedApertureWidth = 2.0800;
                lInspectedApertureHeight = 1.4800;
                break;
            case FbxCamera::eIMAX:
                FBXSDK_printf( "Camera aperture format is imax.\n");
                lInspectedApertureWidth = 2.7720;
                lInspectedApertureHeight = 2.0720;
                break;
            default:
                FBXSDK_printf( "Unknown camera aperture format.\n");
                break;
            }

            //get camera's aperture mode
            FbxCamera::EApertureMode lApertureMode = lCamera->GetApertureMode();
            /*
            Camera aperture modes. The aperture mode determines which values drive the camera aperture. 
            If the aperture mode is eHORIZONTAL_AND_VERTICAL, then the FOVX and FOVY is used. 
            If the aperture mode is eHORIZONTAL or eVERTICAL, then the FOV is used.
            if the aperture mode is eFOCAL_LENGTH, then the focal length is used.
            */
            switch( lApertureMode)
            {
                //fit the resolution gate within the film gate
            case FbxCamera::eHorizAndVert:
                FBXSDK_printf( "Camera aperture mode is horizontal and vertical.\n");
                break;
                //fit the resolution gate horizontally within the film gate
            case FbxCamera::eHorizontal:
                FBXSDK_printf( "Camera aperture mode is horizontal.\n");
                break;
                //fit the resolution gate vertically within the film gate
            case FbxCamera::eVertical:
                FBXSDK_printf( "Camera aperture mode is vertical.\n");
                break;
                //fit the resolution gate according to the focal length
            case FbxCamera::eFocalLength:
                FBXSDK_printf( "Camera aperture mode is focal length.\n");
                break;
            default:
                FBXSDK_printf( "Unknown camera aperture mode.\n");
                break;

            }
            //get camera's aperture height.
            double lApertureHeight = lCamera->GetApertureHeight();
            //get camera's aperture width.
            double lApertureWidth = lCamera->GetApertureWidth();
            //get camera's aperture ratio.
            double lApertureRatio = lCamera->FilmAspectRatio.Get();
            if( lApertureFormat != FbxCamera::eCustomAperture)
            {
                //inspect aperture width.
                if( lApertureWidth != lInspectedApertureWidth)
                {
                    FBXSDK_printf( "Camera aperture width is not right.\n\rRevise the width: %lf inches to %lf inches.\n\n", lApertureWidth, lInspectedApertureWidth);
                    lCamera->FilmWidth.Set( lInspectedApertureWidth);
                    lApertureWidth = lInspectedApertureWidth;
                }
                //inspect aperture height
                if( lApertureHeight != lInspectedApertureHeight)
                {
                    FBXSDK_printf( "Camera aperture height is not right.\n\rRevise the height: %lf inches to %lf inches.\n\n", lApertureHeight, lInspectedApertureHeight);
                    lCamera->FilmHeight.Set( lInspectedApertureHeight);
                    lApertureHeight = lInspectedApertureHeight;
                }
                //inspect aperture ratio.
                double lInspectedApertureRatio = lApertureWidth / lApertureHeight;
                if( lApertureRatio != lInspectedApertureRatio)
                {
                    FBXSDK_printf( "Camera aperture ratio is not right.\n\rRevise the ratio: %lf to %lf.\n\n", lApertureRatio, lInspectedApertureRatio);
                    lCamera->FilmAspectRatio.Set( lInspectedApertureRatio);
                    lApertureRatio = lInspectedApertureRatio;
                }
            }

            FBXSDK_printf( "Camera aperture width: %lf inches.\n\rCamera aperture height: %lf inches.\n", lApertureWidth, lApertureHeight);
            FBXSDK_printf( "Camera aperture ratio is %lf.\n\n", lApertureRatio);


            //get focal length
            double lFocalLength = lCamera->FocalLength.Get();
            //get FOV
            double lFocalAngle = lCamera->FieldOfView.Get();
            //get squeeze ratio
            double lFilmSqueezeRatio = lCamera->GetSqueezeRatio();
            if( lApertureMode == FbxCamera::eHorizAndVert)
            {
                //compute the focal length using FOVX.
                double lFocalAngleX = lCamera->FieldOfViewX.Get();//horizontal fov
                double lFocalAngleY = lCamera->FieldOfViewY.Get();//vertical fov
                double lComputedFocalLength = lCamera->ComputeFocalLength( lFocalAngleX);
                if( lFocalLength != lComputedFocalLength)
                {
                    FBXSDK_printf( "Camera focal length is not right.\n\rRevise the focal length: %lf mm to %lf mm.\n\n", lFocalLength, lComputedFocalLength);
                    lCamera->FocalLength.Set( lComputedFocalLength);
                    lFocalLength = lComputedFocalLength;
                }
                else
                {
                    FBXSDK_printf( "Camera focal length is %lf mm.\n", lFocalLength);
                }
                FBXSDK_printf("Camera horizontal FOV: %lf.\n\rCamera vertical FOV: %lf.\n ", lFocalAngleX, lFocalAngleY);
            }
            else if( lApertureMode == FbxCamera::eFocalLength)
            {
                //compute the FOV using focal length
                double lComputedFOV = lCamera->ComputeFieldOfView( lFocalLength);
                if( lFocalAngle != lComputedFOV)
                {
                    FBXSDK_printf( "Camera FOV is not right.\n\rRevise the FOV: %lf degrees to %lf degrees.\n\n", lFocalAngle, lComputedFOV);
                    lCamera->FieldOfView.Set( lComputedFOV);
                    lFocalAngle = lComputedFOV;
                }
                else
                {
                    FBXSDK_printf( "Camera FOV is %lf degrees.\n", lFocalAngle);
                }
                 FBXSDK_printf( "Camera focal length is %lf mm.\n", lFocalLength);
            }
            else
            {
                //compute the focal length using FOV
                double lComputedFocalLength = lCamera->ComputeFocalLength( lFocalAngle);
                if( lFocalLength != lComputedFocalLength)
                {
                    FBXSDK_printf( "Camera focal length is not right.\n\rRevise the focal length: %lf mm to %lf mm.\n\n", lFocalLength, lComputedFocalLength);
                    lCamera->FocalLength.Set( lComputedFocalLength);
                    lFocalLength = lComputedFocalLength;
                }
                else
                {
                    FBXSDK_printf( "Camera focal length is %lf mm.\n", lFocalLength);
                }
                FBXSDK_printf("Camera FOV: %lf.\n", lFocalAngle);
            }
            FBXSDK_printf( "Camera squeeze ratio is %lf \n", lFilmSqueezeRatio);

        }
        FBXSDK_printf( "/*-----------------Camera: %s----------------------*/\n\r", lCamera->GetName());
    }



    //recursively traverse each node in the scene
    int i, lCount = pNode->GetChildCount();
    for (i = 0; i < lCount; i++)
    {
        DisplayCameraInfo(pNode->GetChild(i));
    }
}

//This function illustrates how to create and connect camera.
FbxCamera* CreateMyCamera(FbxScene* pScene)
{
    if(!pScene)
        return NULL;

    //create a fbx node for camera
    FbxNode* lMyCameraNode = FbxNode::Create(pScene,"myCameraNode");
    //connect camera node to root node
    FbxNode* lRootNode = pScene->GetRootNode();
    lRootNode->ConnectSrcObject( lMyCameraNode);
    //create a cameraStereo, it's a node attribute of  camera node.
    FbxCamera* lMyCamera = FbxCamera::Create(pScene,"myCamera");
    //set Camera as a node attribute of the FBX node.
    lMyCameraNode->SetNodeAttribute (lMyCamera);
    //we recommend to use existed format
    //set camera format
    lMyCamera->SetFormat( FbxCamera::eNTSC);
    //set camera aperture format
    lMyCamera->SetApertureFormat( FbxCamera::e16mmTheatrical);
    //set camera aperture mode
    lMyCamera->SetApertureMode(FbxCamera::eVertical);
    //set camera FOV
    double lFOV = 45;
    lMyCamera->FieldOfView.Set( lFOV);
    //set camera FOV
    double lFocalLength = lMyCamera->ComputeFocalLength( lFOV);
    lMyCamera->FocalLength.Set( lFocalLength);
    


    return lMyCamera;

}

//This function computes the pixel ratio
double ComputePixelRatio( double pWidth, double pHeight, double pScreenRatio)
{
    if( pWidth < 0.0 || pHeight < 0.0)
        return 0.0;

    pWidth = GET_MAX( pWidth, 1.0 );
    pHeight = GET_MAX( pHeight, 1.0 );

    double lResolutionRatio = (double) pWidth / pHeight;

    return pScreenRatio / lResolutionRatio;
}


