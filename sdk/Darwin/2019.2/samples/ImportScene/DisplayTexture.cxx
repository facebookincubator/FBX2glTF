/****************************************************************************************

   Copyright (C) 2017 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

#include <fbxsdk.h>

#include "DisplayCommon.h"

void DisplayTextureInfo(FbxTexture* pTexture, int pBlendMode)
{
	FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(pTexture);
	FbxProceduralTexture *lProceduralTexture = FbxCast<FbxProceduralTexture>(pTexture);

    DisplayString("            Name: \"", (char *) pTexture->GetName(), "\"");
	if (lFileTexture)
	{
		DisplayString("            Type: File Texture");
		DisplayString("            File Name: \"", (char *) lFileTexture->GetFileName(), "\"");
	}
	else if (lProceduralTexture)
	{
		DisplayString("            Type: Procedural Texture");
	}
    DisplayDouble("            Scale U: ", pTexture->GetScaleU());
    DisplayDouble("            Scale V: ", pTexture->GetScaleV());
    DisplayDouble("            Translation U: ", pTexture->GetTranslationU());
    DisplayDouble("            Translation V: ", pTexture->GetTranslationV());
    DisplayBool("            Swap UV: ", pTexture->GetSwapUV());
    DisplayDouble("            Rotation U: ", pTexture->GetRotationU());
    DisplayDouble("            Rotation V: ", pTexture->GetRotationV());
    DisplayDouble("            Rotation W: ", pTexture->GetRotationW());

    const char* lAlphaSources[] = { "None", "RGB Intensity", "Black" };

    DisplayString("            Alpha Source: ", lAlphaSources[pTexture->GetAlphaSource()]);
    DisplayDouble("            Cropping Left: ", pTexture->GetCroppingLeft());
    DisplayDouble("            Cropping Top: ", pTexture->GetCroppingTop());
    DisplayDouble("            Cropping Right: ", pTexture->GetCroppingRight());
    DisplayDouble("            Cropping Bottom: ", pTexture->GetCroppingBottom());

    const char* lMappingTypes[] = { "Null", "Planar", "Spherical", "Cylindrical", 
        "Box", "Face", "UV", "Environment" };

    DisplayString("            Mapping Type: ", lMappingTypes[pTexture->GetMappingType()]);

    if (pTexture->GetMappingType() == FbxTexture::ePlanar)
    {
        const char* lPlanarMappingNormals[] = { "X", "Y", "Z" };

        DisplayString("            Planar Mapping Normal: ", lPlanarMappingNormals[pTexture->GetPlanarMappingNormal()]);
    }

    const char* lBlendModes[]   = { "Translucent", "Additive", "Modulate", "Modulate2", "Over", "Normal", "Dissolve", "Darken", "ColorBurn", "LinearBurn",
                                    "DarkerColor", "Lighten", "Screen", "ColorDodge", "LinearDodge", "LighterColor", "SoftLight", "HardLight", "VividLight",
                                    "LinearLight", "PinLight", "HardMix", "Difference", "Exclusion", "Substract", "Divide", "Hue", "Saturation", "Color",
                                    "Luminosity", "Overlay"};   
    
    if(pBlendMode >= 0)
        DisplayString("            Blend Mode: ", lBlendModes[pBlendMode]);
    DisplayDouble("            Alpha: ", pTexture->GetDefaultAlpha());

    if (lFileTexture)
    {
        const char* lMaterialUses[] = { "Model Material", "Default Material" };
        DisplayString("            Material Use: ", lMaterialUses[lFileTexture->GetMaterialUse()]);
    }

    const char* pTextureUses[] = { "Standard", "Shadow Map", "Light Map", 
        "Spherical Reflexion Map", "Sphere Reflexion Map", "Bump Normal Map" };

    DisplayString("            Texture Use: ", pTextureUses[pTexture->GetTextureUse()]);
    DisplayString("");                

}

void FindAndDisplayTextureInfoByProperty(FbxProperty pProperty, bool& pDisplayHeader, int pMaterialIndex){

    if( pProperty.IsValid() )
    {
		int lTextureCount = pProperty.GetSrcObjectCount<FbxTexture>();

		for (int j = 0; j < lTextureCount; ++j)
		{
			//Here we have to check if it's layeredtextures, or just textures:
			FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
			if (lLayeredTexture)
			{
                DisplayInt("    Layered Texture: ", j);                
                int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
                for(int k =0; k<lNbTextures; ++k)
                {
                    FbxTexture* lTexture = lLayeredTexture->GetSrcObject<FbxTexture>(k);
                    if(lTexture)
                    {

                        if(pDisplayHeader){                    
                            DisplayInt("    Textures connected to Material ", pMaterialIndex);
                            pDisplayHeader = false;
                        }

                        //NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the texture.
                        //Why is that?  because one texture can be shared on different layered textures and might
                        //have different blend modes.

                        FbxLayeredTexture::EBlendMode lBlendMode;
                        lLayeredTexture->GetTextureBlendMode(k, lBlendMode);
                        DisplayString("    Textures for ", pProperty.GetName());
                        DisplayInt("        Texture ", k);  
                        DisplayTextureInfo(lTexture, (int) lBlendMode);   
                    }

                }
            }
			else
			{
				//no layered texture simply get on the property
                FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                if(lTexture)
                {
                    //display connected Material header only at the first time
                    if(pDisplayHeader){                    
                        DisplayInt("    Textures connected to Material ", pMaterialIndex);
                        pDisplayHeader = false;
                    }             

                    DisplayString("    Textures for ", pProperty.GetName());
                    DisplayInt("        Texture ", j);  
                    DisplayTextureInfo(lTexture, -1);
                }
            }
        }
    }//end if pProperty

}


void DisplayTexture(FbxGeometry* pGeometry)
{
    int lMaterialIndex;
    FbxProperty lProperty;
    if(pGeometry->GetNode()==NULL)
        return;
    int lNbMat = pGeometry->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();
    for (lMaterialIndex = 0; lMaterialIndex < lNbMat; lMaterialIndex++){
        FbxSurfaceMaterial *lMaterial = pGeometry->GetNode()->GetSrcObject<FbxSurfaceMaterial>(lMaterialIndex);
        bool lDisplayHeader = true;

        //go through all the possible textures
        if(lMaterial){

            int lTextureIndex;
            FBXSDK_FOR_EACH_TEXTURE(lTextureIndex)
            {
                lProperty = lMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lTextureIndex]);
                FindAndDisplayTextureInfoByProperty(lProperty, lDisplayHeader, lMaterialIndex); 
            }

        }//end if(lMaterial)

    }// end for lMaterialIndex     
}
