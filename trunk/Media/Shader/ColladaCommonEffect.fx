//--------------------------------------------------------------------------------------
// File: ColladaCommonEffect.fx
//
// The effect file for the SimpleSample sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
cbuffer cb0
{
	float4x4 g_mWorld : WORLD;                  // World matrix for object
	float4x4 g_mView : VIEW;
	float4x4 g_mWorldViewProjection : WORLDVIEWPROJECTION;    // World * View * Projection matrix
	
	// Pointing to where light is coming from
	float3 g_DirLight = float3( 1, 1, 1 );
	float4 g_LightDiffuse = float4( 1, 1, 1, 1 );              // Light's diffuse color
};

cbuffer cb1
{
	float4 Emission 
	<
		string UIName = "emission";
		string UIWidget = "color";
	> = { 0.0f, 0.0f, 0.0f, 1.0f };
	
	float4 Ambient 	
	<
		string UIName = "ambient";
		string UIWidget = "color";
	> = { 0.0132f, 0.2f, 0.121f, 1.0f };
	
	float4 Diffuse 	
	<
		string UIName = "diffuse";
		string UIWidget = "color";
	> = { 0.333f, 1.0f, 0.0371f, 1.0f };
	
	float4 Specular 	
	<
		string UIName = "specular";
		string UIWidget = "color";
	> = { 0.333f, 1.0f, 0.0371f, 1.0f };	
	
	float Shininess = 3.0f;
	
	bool UseDiffuseTexture = false;
}

Buffer< uint2 > JointIndex;
Buffer< float4 > JointBind;
Buffer< float > JointWeight;

Texture2D g_DiffuseTex;              // Color texture for mesh

RasterizerState rs_face_ccw
{
	CullMode = None;
};

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
SamplerState DiffuseTextureSampler
{
	//Filter = MIN_MAG_MIP_LINEAR;
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};


//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : SV_Position;   // vertex position 
    float3 Norm : NORMAL; 	// world norm
    float3 TextureUV  : TEXCOORD0;  // vertex texture coords
	float3 WorldPos : TEXCOORD1;
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float3 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float3 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul( float4( vPos, 1.0f ), g_mWorldViewProjection);
    
    // Transform the normal from object space to world space    
    Output.Norm = normalize(mul(vNormal, (float3x3)g_mWorld)); // normal (world space)
    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0; 
	
	Output.WorldPos = mul( vPos, g_mWorld ).xyz;
    
    return Output;   
}

/**
* vSkeletalJoints:
*	uint2( joint_count, joint_offset )
*/
VS_OUTPUT RenderSkeletalVS( float3 vPos : POSITION,
							float3 vNormal : NORMAL,
							float3 vTexCoord0 : TEXCOORD0,
							uint2 vSkeletalJoints : TEXCOORD1 )
{
	VS_OUTPUT Output;
	
	float4 SkinPos = float4( 0, 0, 0, 0 );
	float totalw = 0;
	for( uint i = 0; i < vSkeletalJoints.x; i++ )
	{
		uint2 index = JointIndex.Load( vSkeletalJoints.y + i );
		float weight = JointWeight.Load( index.y );
		float4 r0 = JointBind.Load( index.x * 4 + 0 );
		float4 r1 = JointBind.Load( index.x * 4 + 1 );
		float4 r2 = JointBind.Load( index.x * 4 + 2 );
		float4 r3 = JointBind.Load( index.x * 4 + 3 );
		float4x4 bind = float4x4( r0, r1, r2, r3 );
		SkinPos += mul( float4( vPos, 1.0f ), bind ) * weight;
		totalw += weight;
	};
	SkinPos /= totalw;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul( float4( SkinPos.xyz, 1.0f ), g_mWorldViewProjection);
//    Output.Position = mul( float4( vPos, 1.0f ), g_mWorldViewProjection);
    
    // Transform the normal from object space to world space    
    Output.Norm = normalize(mul(vNormal, (float3x3)g_mWorld)); // normal (world space)
    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0; 
	
	Output.WorldPos = mul( vPos, g_mWorld ).xyz;
    
    return Output;   
};

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
float4 RenderScenePS( VS_OUTPUT In ) : SV_Target
{ 
	float4 diffusecolor;
	if( UseDiffuseTexture == true )
	{
		diffusecolor = g_DiffuseTex.Sample( DiffuseTextureSampler, In.TextureUV.xy );
//		diffusecolor = g_DiffuseTex.SampleGrad( DiffuseTextureSampler, In.TextureUV.xy, ddx( In.TextureUV.x ), ddy( In.TextureUV.y ) );
	}
	else
	{
		diffusecolor = Diffuse;
	};
	
	float3 camerapos = float3( g_mView[3][0], g_mView[3][1], g_mView[3][2] );
    float3 vL = normalize( g_DirLight );
	float3 vH = normalize( normalize( camerapos - In.WorldPos ) + vL );
	float3 vNorm = normalize( In.Norm );
    float NdotL = dot( vL, vNorm );
    float diffusecoeff = max( NdotL, 0 );
	// Blin phong actually
	float NdotH = dot( vH, vNorm );
	float specularcoeff = pow( max( NdotH, 0 ), Shininess );

	//float4 ret = Emission + Ambient + diffusecolor;
	float4 ret = diffusecolor;
    //float4 ret = Emission + Ambient + diffusecolor * diffusecoeff + Specular * specularcoeff;
	return float4( pow( ret.xyz, 2.2 ), 1 );
	//return pow( ret, 2.2 );
}


//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique10 RenderScene
{
    pass P0
    {       
		SetVertexShader( CompileShader( vs_4_0, RenderSceneVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderScenePS() ) );
        
        SetRasterizerState( rs_face_ccw );
    }
}

technique10 RenderSceneSkeletal
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, RenderSkeletalVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderScenePS() ) );
        
        SetRasterizerState( rs_face_ccw );		
	}
}
