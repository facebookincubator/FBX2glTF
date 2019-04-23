/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

#include "DrawScene.h"
#include "SceneCache.h"
#include "GetPosition.h"

void DrawNode(FbxNode* pNode, 
              FbxTime& lTime, 
              FbxAnimLayer * pAnimLayer,
              FbxAMatrix& pParentGlobalPosition,
              FbxAMatrix& pGlobalPosition,
              FbxPose* pPose,
              ShadingMode pShadingMode);
void DrawMarker(FbxAMatrix& pGlobalPosition);
void DrawSkeleton(FbxNode* pNode, 
                  FbxAMatrix& pParentGlobalPosition, 
                  FbxAMatrix& pGlobalPosition);
void DrawMesh(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
              FbxAMatrix& pGlobalPosition, FbxPose* pPose, ShadingMode pShadingMode);
void ComputeShapeDeformation(FbxMesh* pMesh, 
                             FbxTime& pTime, 
                             FbxAnimLayer * pAnimLayer,
                             FbxVector4* pVertexArray);
void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, 
							   FbxMesh* pMesh,
							   FbxCluster* pCluster, 
							   FbxAMatrix& pVertexTransformMatrix,
							   FbxTime pTime, 
							   FbxPose* pPose);
void ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, 
							  FbxMesh* pMesh, 
							  FbxTime& pTime, 
							  FbxVector4* pVertexArray,
							  FbxPose* pPose);
void ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition, 
									  FbxMesh* pMesh, 
									  FbxTime& pTime, 
									  FbxVector4* pVertexArray,
									  FbxPose* pPose);
void ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, 
							FbxMesh* pMesh, 
							FbxTime& pTime, 
							FbxVector4* pVertexArray,
							FbxPose* pPose);
void ReadVertexCacheData(FbxMesh* pMesh, 
                         FbxTime& pTime, 
                         FbxVector4* pVertexArray);
void DrawCamera(FbxNode* pNode, 
                FbxTime& pTime, 
                FbxAnimLayer* pAnimLayer,
                FbxAMatrix& pGlobalPosition);
void DrawLight(const FbxNode* pNode, const FbxTime& pTime, const FbxAMatrix& pGlobalPosition);
void DrawNull(FbxAMatrix& pGlobalPosition);
void MatrixScale(FbxAMatrix& pMatrix, double pValue);
void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue);
void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix);

void InitializeLights(const FbxScene* pScene, const FbxTime & pTime, FbxPose* pPose)
{
    // Set ambient light. Turn on light0 and set its attributes to default (white directional light in Z axis).
    // If the scene contains at least one light, the attributes of light0 will be overridden.
    LightCache::IntializeEnvironment(pScene->GetGlobalSettings().GetAmbientColor());

    // Setting the lights before drawing the whole scene
	const int lLightCount = pScene->GetSrcObjectCount<FbxLight>();
    for (int lLightIndex = 0; lLightIndex < lLightCount; ++lLightIndex)
    {
        FbxLight * lLight = pScene->GetSrcObject<FbxLight>(lLightIndex);
        FbxNode * lNode = lLight->GetNode();
        if (lNode)
{
            FbxAMatrix lGlobalPosition = GetGlobalPosition(lNode, pTime, pPose);
            FbxAMatrix lGeometryOffset = GetGeometry(lNode);
            FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;
            DrawLight(lNode, pTime, lGlobalOffPosition);
    }
    }
}

// Draw recursively each node of the scene. To avoid recomputing 
// uselessly the global positions, the global position of each 
// node is passed to it's children while browsing the node tree.
// If the node is part of the given pose for the current scene,
// it will be drawn at the position specified in the pose, Otherwise
// it will be drawn at the given time.
void DrawNodeRecursive(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
                       FbxAMatrix& pParentGlobalPosition, FbxPose* pPose,
                       ShadingMode pShadingMode)
{
    FbxAMatrix lGlobalPosition = GetGlobalPosition(pNode, pTime, pPose, &pParentGlobalPosition);

    if (pNode->GetNodeAttribute())
    {
    // Geometry offset.
    // it is not inherited by the children.
    FbxAMatrix lGeometryOffset = GetGeometry(pNode);
    FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;

        DrawNode(pNode, pTime, pAnimLayer, pParentGlobalPosition, lGlobalOffPosition, pPose, pShadingMode);
    }

    const int lChildCount = pNode->GetChildCount();
    for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
        DrawNodeRecursive(pNode->GetChild(lChildIndex), pTime, pAnimLayer, lGlobalPosition, pPose, pShadingMode);
    }
}

// Draw the node following the content of it's node attribute.
void DrawNode(FbxNode* pNode, 
              FbxTime& pTime, 
              FbxAnimLayer* pAnimLayer,
              FbxAMatrix& pParentGlobalPosition,
              FbxAMatrix& pGlobalPosition,
              FbxPose* pPose, ShadingMode pShadingMode)
{
    FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

    if (lNodeAttribute)
    {
        // All lights has been processed before the whole scene because they influence every geometry.
        if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMarker)
        {
            DrawMarker(pGlobalPosition);
        }
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
        {
            DrawSkeleton(pNode, pParentGlobalPosition, pGlobalPosition);
        }
        // NURBS and patch have been converted into triangluation meshes.
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            DrawMesh(pNode, pTime, pAnimLayer, pGlobalPosition, pPose, pShadingMode);
        }
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eCamera)
        {
            DrawCamera(pNode, pTime, pAnimLayer, pGlobalPosition);
        }
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNull)
        {
            DrawNull(pGlobalPosition);
        }
    }
    else
    {
        // Draw a Null for nodes without attribute.
        DrawNull(pGlobalPosition);
    }
}


// Draw a small box where the node is located.
void DrawMarker(FbxAMatrix& pGlobalPosition)
{
    GlDrawMarker(pGlobalPosition);  
}


// Draw a limb between the node and its parent.
void DrawSkeleton(FbxNode* pNode, FbxAMatrix& pParentGlobalPosition, FbxAMatrix& pGlobalPosition)
{
    FbxSkeleton* lSkeleton = (FbxSkeleton*) pNode->GetNodeAttribute();

    // Only draw the skeleton if it's a limb node and if 
    // the parent also has an attribute of type skeleton.
    if (lSkeleton->GetSkeletonType() == FbxSkeleton::eLimbNode &&
        pNode->GetParent() &&
        pNode->GetParent()->GetNodeAttribute() &&
        pNode->GetParent()->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
        GlDrawLimbNode(pParentGlobalPosition, pGlobalPosition); 
    }
}


// Draw the vertices of a mesh.
void DrawMesh(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
              FbxAMatrix& pGlobalPosition, FbxPose* pPose, ShadingMode pShadingMode)
{
    FbxMesh* lMesh = pNode->GetMesh();
    const int lVertexCount = lMesh->GetControlPointsCount();

    // No vertex to draw.
    if (lVertexCount == 0)
    {
        return;
    }

    const VBOMesh * lMeshCache = static_cast<const VBOMesh *>(lMesh->GetUserDataPtr());

    // If it has some defomer connection, update the vertices position
    const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
		(static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
    const bool lHasShape = lMesh->GetShapeCount() > 0;
    const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
    const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;

    FbxVector4* lVertexArray = NULL;
    if (!lMeshCache || lHasDeformation)
    {
        lVertexArray = new FbxVector4[lVertexCount];
        memcpy(lVertexArray, lMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
    }

    if (lHasDeformation)
    {
        // Active vertex cache deformer will overwrite any other deformer
        if (lHasVertexCache)
        {
            ReadVertexCacheData(lMesh, pTime, lVertexArray);
        }
        else
        {
            if (lHasShape)
            {
                // Deform the vertex array with the shapes.
                ComputeShapeDeformation(lMesh, pTime, pAnimLayer, lVertexArray);
            }

            //we need to get the number of clusters
            const int lSkinCount = lMesh->GetDeformerCount(FbxDeformer::eSkin);
            int lClusterCount = 0;
            for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
            {
                lClusterCount += ((FbxSkin *)(lMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
            }
            if (lClusterCount)
            {
                // Deform the vertex array with the skin deformer.
                ComputeSkinDeformation(pGlobalPosition, lMesh, pTime, lVertexArray, pPose);
            }
        }

        if (lMeshCache)
            lMeshCache->UpdateVertexPosition(lMesh, lVertexArray);
    }

    glPushMatrix();
    glMultMatrixd((const double*)pGlobalPosition);

    if (lMeshCache)
    {
        lMeshCache->BeginDraw(pShadingMode);
        const int lSubMeshCount = lMeshCache->GetSubMeshCount();
        for (int lIndex = 0; lIndex < lSubMeshCount; ++lIndex)
        {
            if (pShadingMode == SHADING_MODE_SHADED)
            {
                const FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lIndex);
                if (lMaterial)
                {
                    const MaterialCache * lMaterialCache = static_cast<const MaterialCache *>(lMaterial->GetUserDataPtr());
                    if (lMaterialCache)
                    {
                        lMaterialCache->SetCurrentMaterial();
                    }
                }
                else
                {
                    // Draw green for faces without material
                    MaterialCache::SetDefaultMaterial();
                }
            }

            lMeshCache->Draw(lIndex, pShadingMode);
        }
        lMeshCache->EndDraw();
    }
    else
    {
        // OpenGL driver is too lower and use Immediate Mode
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        const int lPolygonCount = lMesh->GetPolygonCount();
        for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; lPolygonIndex++)
        {
            const int lVerticeCount = lMesh->GetPolygonSize(lPolygonIndex);
            glBegin(GL_LINE_LOOP);
            for (int lVerticeIndex = 0; lVerticeIndex < lVerticeCount; lVerticeIndex++)
            {
				int index = lMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
				if (index < 0)
				{
					FBX_ASSERT_NOW("Invalid index!");
					continue;
				}
                glVertex3dv((GLdouble *)lVertexArray[index]);
            }
            glEnd();
        }
    }

    glPopMatrix();

    delete [] lVertexArray;
}


// Deform the vertex array with the shapes contained in the mesh.
void ComputeShapeDeformation(FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, FbxVector4* pVertexArray)
{
    int lVertexCount = pMesh->GetControlPointsCount();

    FbxVector4* lSrcVertexArray = pVertexArray;
    FbxVector4* lDstVertexArray = new FbxVector4[lVertexCount];
    memcpy(lDstVertexArray, pVertexArray, lVertexCount * sizeof(FbxVector4));

	int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for(int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for(int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; ++lChannelIndex)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if(lChannel)
			{
				// Get the percentage of influence on this channel.
				FbxAnimCurve* lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);
				if (!lFCurve) continue;
				double lWeight = lFCurve->Evaluate(pTime);

				/*
				If there is only one targetShape on this channel, the influence is easy to calculate:
                influence = (targetShape - baseGeometry) * weight * 0.01
				dstGeometry = baseGeometry + influence

				But if there are more than one targetShapes on this channel, this is an in-between 
				blendshape, also called progressive morph. The calculation of influence is different.
				
				For example, given two in-between targets, the full weight percentage of first target
				is 50, and the full weight percentage of the second target is 100.
				When the weight percentage reach 50, the base geometry is already be fully morphed 
				to the first target shape. When the weight go over 50, it begin to morph from the 
				first target shape to the second target shape.

				To calculate influence when the weight percentage is 25:
				1. 25 falls in the scope of 0 and 50, the morphing is from base geometry to the first target.
				2. And since 25 is already half way between 0 and 50, so the real weight percentage change to 
				the first target is 50.
				influence = (firstTargetShape - baseGeometry) * (25-0)/(50-0) * 100
				dstGeometry = baseGeometry + influence

				To calculate influence when the weight percentage is 75:
				1. 75 falls in the scope of 50 and 100, the morphing is from the first target to the second.
				2. And since 75 is already half way between 50 and 100, so the real weight percentage change 
				to the second target is 50.
				influence = (secondTargetShape - firstTargetShape) * (75-50)/(100-50) * 100
				dstGeometry = firstTargetShape + influence
				*/

				// Find the two shape indices for influence calculation according to the weight.
				// Consider index of base geometry as -1.

				int lShapeCount = lChannel->GetTargetShapeCount();
				double* lFullWeights = lChannel->GetTargetShapeFullWeights();

				// Find out which scope the lWeight falls in.
				int lStartIndex = -1;
				int lEndIndex = -1;
				for(int lShapeIndex = 0; lShapeIndex<lShapeCount; ++lShapeIndex)
				{
					if(lWeight > 0 && lWeight <= lFullWeights[0])
					{
						lEndIndex = 0;
						break;
					}
					if(lWeight > lFullWeights[lShapeIndex] && lWeight < lFullWeights[lShapeIndex+1])
					{
						lStartIndex = lShapeIndex;
						lEndIndex = lShapeIndex + 1;
						break;
					}
				}

				FbxShape* lStartShape = NULL;
				FbxShape* lEndShape = NULL;
				if(lStartIndex > -1)
				{
					lStartShape = lChannel->GetTargetShape(lStartIndex);
				}
				if(lEndIndex > -1)
				{
					lEndShape = lChannel->GetTargetShape(lEndIndex);
				}

				//The weight percentage falls between base geometry and the first target shape.
				if(lStartIndex == -1 && lEndShape) 
				{
					double lEndWeight = lFullWeights[0];
					// Calculate the real weight.
					lWeight = (lWeight/lEndWeight) * 100;
					// Initialize the lDstVertexArray with vertex of base geometry.
					memcpy(lDstVertexArray, lSrcVertexArray, lVertexCount * sizeof(FbxVector4));
					for (int j = 0; j < lVertexCount; j++)
					{
						// Add the influence of the shape vertex to the mesh vertex.
						FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lSrcVertexArray[j]) * lWeight * 0.01;
						lDstVertexArray[j] += lInfluence;
					}	
				}
				//The weight percentage falls between two target shapes.
				else if(lStartShape && lEndShape)
				{
					double lStartWeight = lFullWeights[lStartIndex];
					double lEndWeight = lFullWeights[lEndIndex];
					// Calculate the real weight.
					lWeight = ((lWeight-lStartWeight)/(lEndWeight-lStartWeight)) * 100;
					// Initialize the lDstVertexArray with vertex of the previous target shape geometry.
					memcpy(lDstVertexArray, lStartShape->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
					for (int j = 0; j < lVertexCount; j++)
					{
						// Add the influence of the shape vertex to the previous shape vertex.
						FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lStartShape->GetControlPoints()[j]) * lWeight * 0.01;
						lDstVertexArray[j] += lInfluence;
					}	
				}
			}//If lChannel is valid
		}//For each blend shape channel
	}//For each blend shape deformer

    memcpy(pVertexArray, lDstVertexArray, lVertexCount * sizeof(FbxVector4));

    delete [] lDstVertexArray;
}

//Compute the transform matrix that the cluster will transform the vertex.
void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, 
							   FbxMesh* pMesh,
							   FbxCluster* pCluster, 
							   FbxAMatrix& pVertexTransformMatrix,
							   FbxTime pTime, 
							   FbxPose* pPose)
{
    FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

	FbxAMatrix lReferenceGlobalInitPosition;
	FbxAMatrix lReferenceGlobalCurrentPosition;
	FbxAMatrix lAssociateGlobalInitPosition;
	FbxAMatrix lAssociateGlobalCurrentPosition;
	FbxAMatrix lClusterGlobalInitPosition;
	FbxAMatrix lClusterGlobalCurrentPosition;

	FbxAMatrix lReferenceGeometry;
	FbxAMatrix lAssociateGeometry;
	FbxAMatrix lClusterGeometry;

	FbxAMatrix lClusterRelativeInitPosition;
	FbxAMatrix lClusterRelativeCurrentPositionInverse;
	
	if (lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel())
	{
		pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
		// Geometric transform of the model
		lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
		lAssociateGlobalInitPosition *= lAssociateGeometry;
		lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);

		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;
		lReferenceGlobalCurrentPosition = pGlobalPosition;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		// Multiply lClusterGlobalInitPosition by Geometric Transformation
		lClusterGeometry = GetGeometry(pCluster->GetLink());
		lClusterGlobalInitPosition *= lClusterGeometry;
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

		// Compute the shift of the link relative to the reference.
		//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
		pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
			lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
	}
	else
	{
		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		lReferenceGlobalCurrentPosition = pGlobalPosition;
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

		// Compute the initial position of the link relative to the reference.
		lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
	}
}

// Deform the vertex array in classic linear way.
void ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, 
                               FbxMesh* pMesh, 
                               FbxTime& pTime, 
                               FbxVector4* pVertexArray,
							   FbxPose* pPose)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
	memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	if (lClusterMode == FbxCluster::eAdditive)
	{
		for (int i = 0; i < lVertexCount; ++i)
		{
			lClusterDeformation[i].SetIdentity();
		}
	}

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		
		int lClusterCount = lSkinDeformer->GetClusterCount();
		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k) 
			{            
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
				{
					continue;
				}

				// Compute the influence of the link on the vertex.
				FbxAMatrix lInfluence = lVertexTransformMatrix;
				MatrixScale(lInfluence, lWeight);

				if (lClusterMode == FbxCluster::eAdditive)
				{    
					// Multiply with the product of the deformations on the vertex.
					MatrixAddToDiagonal(lInfluence, 1.0 - lWeight);
					lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					// Add to the sum of the deformations on the vertex.
					MatrixAdd(lClusterDeformation[lIndex], lInfluence);

					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex			
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < lVertexCount; i++) 
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeight = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeight != 0.0) 
		{
			lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);
			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeight;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeight);
				lDstVertex += lSrcVertex;
			}
		} 
	}

	delete [] lClusterDeformation;
	delete [] lClusterWeight;
}

// Deform the vertex array in Dual Quaternion Skinning way.
void ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition, 
									 FbxMesh* pMesh, 
									 FbxTime& pTime, 
									 FbxVector4* pVertexArray,
									 FbxPose* pPose)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
	memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		int lClusterCount = lSkinDeformer->GetClusterCount();
		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

			FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
			FbxVector4 lT = lVertexTransformMatrix.GetT();
			FbxDualQuaternion lDualQuaternion(lQ, lT);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k) 
			{ 
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
					continue;

				// Compute the influence of the link on the vertex.
				FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
				if (lClusterMode == FbxCluster::eAdditive)
				{    
					// Simply influenced by the dual quaternion.
					lDQClusterDeformation[lIndex] = lInfluence;

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					if(lClusterIndex == 0)
					{
						lDQClusterDeformation[lIndex] = lInfluence;
					}
					else
					{
						// Add to the sum of the deformations on the vertex.
						// Make sure the deformation is accumulated in the same rotation direction. 
						// Use dot product to judge the sign.
						double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());
						if( lSign >= 0.0 )
						{
							lDQClusterDeformation[lIndex] += lInfluence;
						}
						else
						{
							lDQClusterDeformation[lIndex] -= lInfluence;
						}
					}
					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < lVertexCount; i++) 
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeightSum = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeightSum != 0.0) 
		{
			lDQClusterDeformation[i].Normalize();
			lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);

			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeightSum;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeightSum);
				lDstVertex += lSrcVertex;
			}
		} 
	}

	delete [] lDQClusterDeformation;
	delete [] lClusterWeight;
}

// Deform the vertex array according to the links contained in the mesh and the skinning type.
void ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, 
									 FbxMesh* pMesh, 
									 FbxTime& pTime, 
									 FbxVector4* pVertexArray,
									 FbxPose* pPose)
{
	FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

	if(lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid)
	{
		ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
	}
	else if(lSkinningType == FbxSkin::eDualQuaternion)
	{
		ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
	}
	else if(lSkinningType == FbxSkin::eBlend)
	{
		int lVertexCount = pMesh->GetControlPointsCount();

		FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayLinear, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayDQ, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayLinear, pPose);
		ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayDQ, pPose);

		// To blend the skinning according to the blend weights
		// Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
		// DQSVertex: vertex that is deformed by dual quaternion skinning method;
		// LinearVertex: vertex that is deformed by classic linear skinning method;
		int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();
		for(int lBWIndex = 0; lBWIndex<lBlendWeightsCount; ++lBWIndex)
		{
			double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
			pVertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
		}
	}
}


void ReadVertexCacheData(FbxMesh* pMesh, 
                         FbxTime& pTime, 
                         FbxVector4* pVertexArray)
{
    FbxVertexCacheDeformer* lDeformer     = static_cast<FbxVertexCacheDeformer*>(pMesh->GetDeformer(0, FbxDeformer::eVertexCache));
    FbxCache*               lCache        = lDeformer->GetCache();
    int                     lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
    unsigned int            lVertexCount  = (unsigned int)pMesh->GetControlPointsCount();
    bool                    lReadSucceed  = false;
    float*                  lReadBuf      = NULL;
	unsigned int			BufferSize	  = 0;

	if (lDeformer->Type.Get() != FbxVertexCacheDeformer::ePositions)
		// only process positions
		return; 

	unsigned int Length = 0;
	lCache->Read(NULL, Length, FBXSDK_TIME_ZERO, lChannelIndex);
	if (Length != lVertexCount*3)
		// the content of the cache is by vertex not by control points (we don't support it here)
		return;

    lReadSucceed = lCache->Read(&lReadBuf, BufferSize, pTime, lChannelIndex);
    if (lReadSucceed)
    {
        unsigned int lReadBufIndex = 0;

        while (lReadBufIndex < 3*lVertexCount)
        {
            // In statements like "pVertexArray[lReadBufIndex/3].SetAt(2, lReadBuf[lReadBufIndex++])", 
            // on Mac platform, "lReadBufIndex++" is evaluated before "lReadBufIndex/3". 
            // So separate them.
            pVertexArray[lReadBufIndex/3].mData[0] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
            pVertexArray[lReadBufIndex/3].mData[1] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
            pVertexArray[lReadBufIndex/3].mData[2] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
        }
    }
}


// Draw an oriented camera box where the node is located.
void DrawCamera(FbxNode* pNode, 
                FbxTime& pTime, 
                FbxAnimLayer* pAnimLayer,
                FbxAMatrix& pGlobalPosition)
{
    FbxAMatrix lCameraGlobalPosition;
    FbxVector4 lCameraPosition, lCameraDefaultDirection, lCameraInterestPosition;

    lCameraPosition = pGlobalPosition.GetT();

    // By default, FBX cameras point towards the X positive axis.
    FbxVector4 lXPositiveAxis(1.0, 0.0, 0.0);
    lCameraDefaultDirection = lCameraPosition + lXPositiveAxis;

    lCameraGlobalPosition = pGlobalPosition;

    // If the camera is linked to an interest, get the interest position.
    if (pNode->GetTarget())
    {
        lCameraInterestPosition = GetGlobalPosition(pNode->GetTarget(), pTime).GetT();

        // Compute the required rotation to make the camera point to it's interest.
        FbxVector4 lCameraDirection;
        FbxVector4::AxisAlignmentInEulerAngle(lCameraPosition, 
            lCameraDefaultDirection, 
            lCameraInterestPosition, 
            lCameraDirection);

        // Must override the camera rotation 
        // to make it point to it's interest.
        lCameraGlobalPosition.SetR(lCameraDirection);
    }

    // Get the camera roll.
    FbxCamera* cam = pNode->GetCamera();
    double lRoll = 0;

    if (cam)
    {
        lRoll = cam->Roll.Get();
        FbxAnimCurve* fc = cam->Roll.GetCurve(pAnimLayer);
        if (fc) fc->Evaluate(pTime);
    }
    GlDrawCamera(lCameraGlobalPosition, lRoll);
}


// Draw a colored sphere or cone where the node is located.
void DrawLight(const FbxNode* pNode, const FbxTime& pTime, const FbxAMatrix& pGlobalPosition)
{
    const FbxLight* lLight = pNode->GetLight();
    if (!lLight)
        return;

    // Must rotate the light's global position because 
    // FBX lights point towards the Y negative axis.
    FbxAMatrix lLightRotation;
    const FbxVector4 lYNegativeAxis(-90.0, 0.0, 0.0);
    lLightRotation.SetR(lYNegativeAxis);
    const FbxAMatrix lLightGlobalPosition = pGlobalPosition * lLightRotation;

    glPushMatrix();
    glMultMatrixd((const double*)lLightGlobalPosition);

    const LightCache * lLightCache = static_cast<const LightCache *>(lLight->GetUserDataPtr());
    if (lLightCache)
    {
        lLightCache->SetLight(pTime);
    }

    glPopMatrix();
}


// Draw a cross hair where the node is located.
void DrawNull(FbxAMatrix& pGlobalPosition)
{
    GlDrawCrossHair(pGlobalPosition);
}


// Scale all the elements of a matrix.
void MatrixScale(FbxAMatrix& pMatrix, double pValue)
{
    int i,j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pMatrix[i][j] *= pValue;
        }
    }
}


// Add a value to all the elements in the diagonal of the matrix.
void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue)
{
    pMatrix[0][0] += pValue;
    pMatrix[1][1] += pValue;
    pMatrix[2][2] += pValue;
    pMatrix[3][3] += pValue;
}


// Sum two matrices element by element.
void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix)
{
    int i,j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pDstMatrix[i][j] += pSrcMatrix[i][j];
        }
    }
}
