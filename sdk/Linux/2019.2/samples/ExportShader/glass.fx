/*********************************************************************NVMH3****
Path:  NVSDK\Common\media\cgfx
File:  $Id: glass.fx

Copyright NVIDIA Corporation 2002-2004
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
    Glass shader with reflection and refractive dispersion.

******************************************************************************/

float Script : STANDARDSGLOBAL <
    string UIWidget = "none";
    string ScriptClass = "object";
    string ScriptOrder = "standard";
    string ScriptOutput = "color";
    string Script = "Technique=dx9;";
> = 0.8;

/************* TWEAKABLES **************/

float4x4 worldIT : WorldInverseTranspose;
float4x4 wvp : WorldViewProjection;
float4x4 world : World;
float4x4 viewI : ViewInverse;

float reflectStrength
<
    string UIWidget = "slider";
    float UIMin = 0.0;
    float UIMax = 2.0;
    float UIStep = 0.01;
    string UIName =  "Reflection";
> = 1.0;

float refractStrength
<
    string UIWidget = "slider";
    float UIMin = 0.0;
    float UIMax = 2.0;
    float UIStep = 0.01;
    string UIName =  "Refraction";
> = 1.0;

half3 etas
<
    string UIName = "Refraction indices";
> = { 0.80, 0.82, 0.84 };

texture cubeMap : Environment
<
	string ResourceName = "default_reflection.dds";
	string ResourceType = "Cube";
>;

texture fresnelTex : Environment
<
	string ResourceType = "2D";
	string function = "generateFresnelTex";
	
	float2 Dimensions = { 256.0f, 1.0f};
>;

samplerCUBE environmentMapSampler = sampler_state
{
	Texture = <cubeMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

sampler2D fresnelSampler = sampler_state
{
	Texture = <fresnelTex>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = None;
};

/************* DATA STRUCTS **************/

/* data from application vertex buffer */
struct appdata {
    float4 Position	: POSITION;
    float4 UV		: TEXCOORD0;
    float3 Normal	: NORMAL;
};

/* data passed from vertex shader to pixel shader */
struct vertexOutput {
    float4 HPosition	: POSITION;
    float4 TexCoord		: TEXCOORD0;
    float3 WorldNormal	: TEXCOORD1;
    float3 WorldView	: TEXCOORD2;
};

/*********** vertex shader ******/

vertexOutput mainVS(appdata IN,
    uniform float4x4 WorldViewProj,
    uniform float4x4 WorldIT,
    uniform float4x4 World,
    uniform float4x4 viewI
) {
    vertexOutput OUT;
    float3 normal = normalize(IN.Normal);
    OUT.WorldNormal = mul(normal, (float3x3) WorldIT);
    float3 Pw = mul(IN.Position, World).xyz;
    OUT.TexCoord = IN.UV;
    OUT.WorldView = viewI[3].xyz - Pw;
    OUT.HPosition = mul(IN.Position, WorldViewProj);
    return OUT;
}

/********* pixel shader ********/

// modified refraction function that returns boolean for total internal reflection
float3
refract2( float3 I, float3 N, float eta, out bool fail )
{
	float IdotN = dot(I, N);
	float k = 1 - eta*eta*(1 - IdotN*IdotN);
//	return k < 0 ? (0,0,0) : eta*I - (eta*IdotN + sqrt(k))*N;
	fail = k < 0;
	return eta*I - (eta*IdotN + sqrt(k))*N;
}

// approximate Fresnel function
float fresnel(float NdotV, float bias, float power)
{
   return bias + (1.0-bias)*pow(1.0 - max(NdotV, 0), power);
}

// function to generate a texture encoding the Fresnel function
float4 generateFresnelTex(float NdotV : POSITION) : COLOR
{
	return fresnel(NdotV, 0.2, 4.0);
}

float4 mainPS(vertexOutput IN,
    uniform samplerCUBE EnvironmentMap, 
    uniform half reflectStrength,
    uniform half refractStrength,
    uniform half3 etas
    ) : COLOR
{
    half3 N = normalize(IN.WorldNormal);
    float3 V = normalize(IN.WorldView);
    
 	// reflection
    half3 R = reflect(-V, N);
    half4 reflColor = texCUBE(EnvironmentMap, R);

//	half fresnel = fresnel(dot(N, V), 0.2, 4.0);
	half fresnel = tex2D(fresnelSampler, dot(N, V));

	// wavelength colors
	const half4 colors[3] = {
    	{ 1, 0, 0, 0 },
    	{ 0, 1, 0, 0 },
    	{ 0, 0, 1, 0 },
	};
        
	// transmission
 	half4 transColor = 0;
  	bool fail = false;
    for(int i=0; i<3; i++) {
    	half3 T = refract2(-V, N, etas[i], fail);
    	transColor += texCUBE(EnvironmentMap, T) * colors[i];
	}

    return lerp(transColor*refractStrength, reflColor*reflectStrength, fresnel);
}

/*************/

technique dx9 <
	string Script = "Pass=p0;";
> {
	pass p0  <
		string Script = "Draw=geometry;";
	> {		
        VertexShader = compile vs_2_0 mainVS(wvp,worldIT,world,viewI);
        PixelShader = compile ps_2_0 mainPS(environmentMapSampler,
        									reflectStrength, refractStrength,
        									etas);
	}
}

/***************************** eof ***/
