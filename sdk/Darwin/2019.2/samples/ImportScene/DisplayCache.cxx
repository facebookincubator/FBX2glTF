/****************************************************************************************

   Copyright (C) 2017 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/


#include <fbxsdk.h>

#include "DisplayCommon.h"

void DisplayCache(FbxGeometry* pGeometry)
{
	int lVertexCacheDeformerCount = pGeometry->GetDeformerCount( FbxDeformer::eVertexCache );

	for( int i = 0; i < lVertexCacheDeformerCount; ++i )
	{
		FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pGeometry->GetDeformer(i, FbxDeformer::eVertexCache));
		if( !lDeformer ) continue;

		FbxCache* lCache = lDeformer->GetCache();
		if( !lCache ) continue;
		
		if (lCache->OpenFileForRead())
		{                
			DisplayString("    Vertex Cache");
			int lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
			// skip normal channel
			if (lChannelIndex < 0)
				continue;
			
			FbxString lChnlName, lChnlInterp;

			FbxCache::EMCDataType lChnlType;
			FbxTime start, stop, rate;
			FbxCache::EMCSamplingType lChnlSampling;
			unsigned int lChnlSampleCount, lDataCount;

			lCache->GetChannelName(lChannelIndex, lChnlName);
			DisplayString("        Channel Name: ", lChnlName.Buffer());
			lCache->GetChannelDataType(lChannelIndex, lChnlType);
			switch (lChnlType)
			{
			case FbxCache::eUnknownData:
				DisplayString("        Channel Type: Unknown Data"); break;
			case FbxCache::eDouble:	
				DisplayString("        Channel Type: Double"); break;
			case FbxCache::eDoubleArray:
				DisplayString("        Channel Type: Double Array"); break;
			case FbxCache::eDoubleVectorArray:
				DisplayString("        Channel Type: Double Vector Array"); break;
			case FbxCache::eInt32Array:
				DisplayString("        Channel Type: Int32 Array"); break;
			case FbxCache::eFloatArray:
				DisplayString("        Channel Type: Float Array"); break;
			case FbxCache::eFloatVectorArray:
				DisplayString("        Channel Type: Float Vector Array"); break;
			}
			lCache->GetChannelInterpretation(lChannelIndex, lChnlInterp);
			DisplayString("        Channel Interpretation: ", lChnlInterp.Buffer());
			lCache->GetChannelSamplingType(lChannelIndex, lChnlSampling);
			DisplayInt("        Channel Sampling Type: ", lChnlSampling);
			lCache->GetAnimationRange(lChannelIndex, start, stop);
			lCache->GetChannelSamplingRate(lChannelIndex, rate);
			lCache->GetChannelSampleCount(lChannelIndex, lChnlSampleCount);
			DisplayInt("        Channel Sample Count: ", lChnlSampleCount);	 
			
			// Only display cache data if the data type is float vector array
			if (lChnlType != FbxCache::eFloatVectorArray)
				continue;

			if (lChnlInterp == "normals")
				DisplayString("        Normal Cache Data");
			else
				DisplayString("        Points Cache Data");
			float* lBuffer = NULL;
			unsigned int lBufferSize = 0;
			int lFrame = 0;
			for (FbxTime t = start; t <= stop; t+=rate)
			{
				DisplayInt("            Frame ", lFrame);
				lCache->GetChannelPointCount(lChannelIndex, t, lDataCount);
				if (lBuffer == NULL)
				{
					lBuffer = new float[lDataCount*3];
					lBufferSize = lDataCount*3;
				}
				else if (lBufferSize < lDataCount*3)
				{
					delete [] lBuffer;
					lBuffer = new float[lDataCount*3];
					lBufferSize = lDataCount*3;
				}
				else
					memset(lBuffer, 0, lBufferSize*sizeof(float));

				lCache->Read(lChannelIndex, t, lBuffer, lDataCount);
				if (lChnlInterp == "normals")
				{
					// display normals cache data
					// the normal data is per-polygon per-vertex. we can get the polygon vertex index
					// from the index array of polygon vertex
					FbxMesh* lMesh = (FbxMesh*)pGeometry;

					if (lMesh == NULL)
					{
						// Only Mesh can have normal cache data
						continue;
					}

					DisplayInt("                Normal Count ", lDataCount);
					int pi, j, lPolygonCount = lMesh->GetPolygonCount();
					unsigned lNormalIndex = 0;
					for (pi = 0; pi < lPolygonCount && lNormalIndex+2 < lDataCount*3; pi++)
					{
						DisplayInt("                    Polygon ", pi);
						DisplayString("                    Normals for Each Polygon Vertex: ");
						int lPolygonSize = lMesh->GetPolygonSize(pi);
						for (j = 0; j < lPolygonSize && lNormalIndex+2 < lDataCount*3; j++)
						{
							FbxVector4 normal(lBuffer[lNormalIndex], lBuffer[lNormalIndex+1], lBuffer[lNormalIndex+2]);
							Display3DVector("                       Normal Cache Data  ", normal);
							lNormalIndex += 3;
						}
					}
				}
				else
				{
					DisplayInt("               Points Count: ", lDataCount);
					for (unsigned int j = 0; j < lDataCount*3; j=j+3)
					{
						FbxVector4 points(lBuffer[j], lBuffer[j+1], lBuffer[j+2]);
						Display3DVector("                   Points Cache Data: ", points);
					}
				}

				lFrame++;
			}

			if (lBuffer != NULL) 
			{
				delete [] lBuffer;
				lBuffer = NULL;
			}

			lCache->CloseFile();
		}
	}
}
