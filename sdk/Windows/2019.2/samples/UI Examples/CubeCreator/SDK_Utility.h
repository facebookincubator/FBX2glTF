/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

// use the fbxsdk.h
#include <fbxsdk.h>

// to create a basic scene
bool CreateScene();

// to save a scene to a FBX file
bool Export(    
                const char* pFilename, 
                int pFileFormat 
            );

// to save a scene to a FBX file
bool SaveScene(
                FbxManager* pSdkManager, 
                FbxDocument* pScene, 
                const char* pFilename, 
                int pFileFormat, 
                bool pEmbedMedia
              );

// to get filters for the <Save file> dialog (description + file extention)
const char *GetWriterSFNFilters();

// to get a file extention for a WriteFileFormat
const char *GetFileFormatExt(
                                const int pWriteFileFormat
                            );

// to create an instance of the SDK manager
bool InitializeSdkObjects(
                            FbxManager*& pSdkManager, 
                            FbxScene*& pScene
                         );


// to destroy an instance of the SDK manager
void DestroySdkObjects(
                        FbxManager* pSdkManager,
						bool pExitStatus
                      );

// to get the root node
const FbxNode* GetRootNode();

// to get the root node name
const char * GetRootNodeName();

// create a new cube under the root node
void CreateCube(bool pWithTexture, bool pAnimate);

// remove all cubes from the scene
void RemoveCubes();

// to get a string from the node name and attribute type
FbxString GetNodeNameAndAttributeTypeName(
                                         const FbxNode* pNode
                                       );

// to get a string from the node default translation values
FbxString GetDefaultTranslationInfo(
								   const FbxNode* pNode
								 );

// to get a string with info about material, texture, animation
FbxString GetNodeInfo(
                     const FbxNode* pNode
                   );


// Create a marker to use a point of interest for the camera. 
FbxNode* CreateMarker(
                        FbxScene* pScene, 
                        char* pName
                      );

// Create a camera
FbxNode* CreateCamera(
                        FbxScene* pScene, 
                        char* pName
                      );

// Create a cube mesh
FbxNode* CreateCubeMesh(
                          FbxScene* pScene, 
                          char* pName
                        );

// Create texture
void CreateTexture(
                    FbxScene* pScene
                  );

// Create material
void CreateMaterial(
                      FbxScene* pScene 
                    );

// Add materials to a mesh
void AddMaterials(
                   FbxMesh* pMesh
                 );

void SetCameraPointOfInterest(
                               FbxNode* pCamera, 
                               FbxNode* pPointOfInterest
                             );

void SetMarkerDefaultPosition(
                               FbxNode* pMarker
                             );

void SetCameraDefaultPosition(
                               FbxNode* pCamera
                             );

void AnimateCamera(
                    FbxNode* pCamera, 
                    FbxAnimLayer* pAnimLayer
                  );

void AnimateCube(
                  FbxNode* pCube, 
                  FbxAnimLayer* pAnimLayer, 
                  int pRotAxe
                );

void CreateCubeDetailed(
                         char* pCubeName, 
                         double pX, 
                         double pY, 
                         double pZ, 
                         int pRotateAxe,
                         bool pWithTexture, 
                         bool pAnim
                       );

void SetInitialCubeData();
