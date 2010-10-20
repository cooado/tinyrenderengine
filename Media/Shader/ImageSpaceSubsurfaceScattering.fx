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
	float4x4 g_mProjection : PROJECTION;
	float4x4 g_mWorldViewProjection : WORLDVIEWPROJECTION;    // World * View * Projection matrix
	
	// Pointing to where light is coming from
	float3 g_LightPos = float3( 15, 15, 15 );
	float4 g_LightDiffuseColor = float4( 1, 1, 1, 1 );              // Light's diffuse color
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
	
	float g_ScatterRadiusSquared = 25.0f;
	
	uint2 LightSourceResolution
	<
		string UIName = "Light Source Resolution";
	> = { 48, 48 };
	
	float SplatRadius = 5;
	
	float2 ScreenOffsets = float2( 1.0f / 640.0f, 1.0f / 480.0f );
	
	float2 TangentHalfFOVXY = float2( 0.57735f, 0.57735f );
	
	bool UseDiffuseTexture = false;
	
	bool UseSSSS = true;
}

Texture2D g_ScatteredSubsurfaceLight;              // Color texture for mesh

Texture2D g_LightSource;

Texture2D g_LightDiffuse;

Texture2D g_GBuffer;

Texture2D g_ScatterTexture;

BlendState Additive
{
	BlendEnable[0] = TRUE;
	SrcBlend = One;
	DestBlend = One;
	BlendOp = ADD;
	SrcBlendAlpha = One;
	DestBlendAlpha = One;
	BlendOpAlpha = ADD;
};

BlendState NoBlending
{
	BlendEnable[0] = FALSE;
};

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
SamplerState TriLinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState PointSampler
{
	Filter = MIN_MAG_MIP_POINT;
};

SamplerState ScatterSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : SV_Position;   // vertex position
	float3 Norm : NORMAL; // world normal
	float3 WorldPos : TEXCOORD0;
	float2 ScreenUV : TEXCOORD1;
};


VS_OUTPUT RenderSceneVS( float3 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float3 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;

    float4 HPos = mul( float4( vPos, 1.0f ), g_mWorldViewProjection);
	Output.Position = HPos;
	Output.Norm = mul( float4( vNormal, 0.0f ), g_mWorld ).xyz;
	Output.WorldPos = mul( float4( vPos, 1.0f ), g_mWorld ).xyz;
    Output.ScreenUV = HPos.xy /  HPos.w * float2( 0.5f, -0.5f ) + float2( 0.5f, 0.5f );
    
	return Output;    
}
struct Quad_VS_Out
{
	float4 HPos : SV_Position;
	float2 ScreenUV : TEXCOORD0;
};

// Triangle list is assumed
Quad_VS_Out RenderQuadVS( uint i : SV_VertexID )
{
	Quad_VS_Out Out;
	
	if( i == 0 )
	{
		Out.HPos = float4( -1, -1, 0, 1 );
		Out.ScreenUV = float2( 0, 1 );
	}
	else if( i == 1 )
	{
		Out.HPos = float4( -1, 1, 0, 1 );
		Out.ScreenUV = float2( 0, 0 );
	}
	else if( i == 2 )
	{
		Out.HPos = float4( 1, 1, 0, 1 );
		Out.ScreenUV = float2( 1, 0 );
	}
	else if( i == 3 )
	{
		Out.HPos = float4( -1, -1, 0, 1 );
		Out.ScreenUV = float2( 0, 1 );
	}
	else if( i == 4 )
	{
		Out.HPos = float4( 1, 1, 0, 1 );
		Out.ScreenUV = float2( 1, 0 );
	}
	else if( i == 5 )
	{
		Out.HPos = float4( 1, -1, 0, 1 );
		Out.ScreenUV = float2( 1, 1 );
	};
	
	return Out;
};

float4 RenderQuadPS( Quad_VS_Out In ) : SV_Target0
{
	//return float4( 1, 1, 1, 1 );
	return g_LightDiffuse.SampleLevel( PointSampler, In.ScreenUV, 0 ); 
};

struct SplatVS_Out
{
	float4 WorldPos : POSITON;
	float3 Color : Color;
};

SplatVS_Out SplatVS( uint i : SV_VertexID )
{
	SplatVS_Out Out;
	
	uint u = i % LightSourceResolution.x;
	uint v = i / LightSourceResolution.x;
	float2 uv = float2( float( u ) / float( LightSourceResolution.x ), float( v ) / float( LightSourceResolution.y ) );
	float2 offset = float2( 0.5f / float( LightSourceResolution.x ), 0.5f / float( LightSourceResolution.y ) );
	uv += offset;
	Out.WorldPos = g_LightSource.SampleLevel( PointSampler, uv, 0 );
	Out.Color = g_LightDiffuse.SampleLevel( PointSampler, uv, 0 ).rgb;
	
	return Out;
};

struct SplatGS_Out
{
	float4 Position : SV_Position;
	float3 WorldPos : POSITION;
	float3 Color : COLOR;
	float2 UV : TEXCOORD0;
};

/**
*	Transform mathematics:
*		View Space Radius
*		---------
*		|      /
*		|     /
*		|    /
*		| N /
*		|--/
*		| /
*		|/
*		
*		Proj_X = View_X / ( View_Z * tan( fovx / 2 ) );
*
*		Proj_Y = View_Y / ( View_Z * tan( fovy / 2 ) );
*
*
*	New triangles:
*		1----2
*		|  / |
*		| /  |
*		0----3
*/
[maxvertexcount(12)]
void SplatGS( triangle SplatVS_Out input[3], inout TriangleStream<SplatGS_Out> TriStream )
{
    SplatGS_Out output;
    
    for( int i=0; i<3; i++ )
    {
		if( input[ i ].WorldPos.w < 0.5f )
		{
			continue;
		};
		
        float3 viewpos = mul( float4( input[i].WorldPos.xyz, 1.0f ), g_mView ).xyz;
		
		float3 pos0 = viewpos + float3( -1.0f * SplatRadius, -1.0f * SplatRadius, 0 );
		float3 pos1 = viewpos + float3( -1.0f * SplatRadius, SplatRadius, 0 );
		float3 pos2 = viewpos + float3( SplatRadius, SplatRadius, 0 );
		float3 pos3 = viewpos + float3( SplatRadius, -1.0f * SplatRadius, 0 );
//		float4 View_MinX_MinY_MaxX_MaxY = viewpos.xyxy + float4( -1.0f * SplatRadius, -1.0f * SplatRadius, SplatRadius, SplatRadius );
//		float4 Proj_MinX_MinY_MaxX_MaxY = View_MinX_MinY_MaxX_MaxY / viewpos.zzzz * TangentHalfFOVXY.xyxy;
        
		
		// tri 0, v1
		//output.Position = float4( Proj_MinX_MinY_MaxX_MaxY.xw, 0.0f, 1.0f );
		output.Position = mul( float4( pos1, 1.0f ), g_mProjection );
		output.WorldPos = input[i].WorldPos.xyz;
		output.Color = input[i].Color;
		output.UV = output.Position.xy / output.Position.w * float2( 0.5f, -0.5f ) + float2( 0.5f, 0.5f );// + ScreenOffsets;
		TriStream.Append( output );
		// tri 0, v2
		//output.Position = float4( Proj_MinX_MinY_MaxX_MaxY.zw, 0.0f, 1.0f );
		output.Position = mul( float4( pos2, 1.0f ), g_mProjection );
		output.WorldPos = input[i].WorldPos.xyz;
		output.Color = input[i].Color;
		output.UV = output.Position.xy / output.Position.w * float2( 0.5f, -0.5f ) + float2( 0.5f, 0.5f );// + ScreenOffsets;
		TriStream.Append( output );
		// tri 0, v0
		//output.Position = float4( Proj_MinX_MinY_MaxX_MaxY.xy, 0.0f, 1.0f );
		output.Position = mul( float4( pos0, 1.0f ), g_mProjection );
		output.WorldPos = input[i].WorldPos.xyz;
		output.Color = input[i].Color;
		output.UV = output.Position.xy / output.Position.w * float2( 0.5f, -0.5f ) + float2( 0.5f, 0.5f );// + ScreenOffsets;
		TriStream.Append( output );		
		// tri 1, v3
		//output.Position = float4( Proj_MinX_MinY_MaxX_MaxY.zy, 0.0f, 1.0f );
		output.Position = mul( float4( pos3, 1.0f ), g_mProjection );
		output.WorldPos = input[i].WorldPos.xyz;
		output.Color = input[i].Color;
		output.UV = output.Position.xy / output.Position.w * float2( 0.5f, -0.5f ) + float2( 0.5f, 0.5f );// + ScreenOffsets;
		TriStream.Append( output );
        
        TriStream.RestartStrip();
    };    
};

float4 SplatPS( SplatGS_Out In ) : SV_Target
{
	float4 wpos = g_GBuffer.Sample( PointSampler, In.UV );
	
	if( wpos.w < 0.5f )
	{
		discard;
	};
	
	float3 scattervec = wpos - In.WorldPos;
	float u = dot( scattervec, scattervec ) / g_ScatterRadiusSquared;
	if( u > 1.0f )
	{
		discard;
	};
	float2 uv = float2( u, 0.5f );
	
	float3 scattercolor = g_ScatterTexture.SampleLevel( ScatterSampler, uv, 0 ).rgb;
	scattercolor *= In.Color;
	
	return float4( scattercolor, 1.0f );
	//return float4( 0, 0, 0, 0 );
};

float4 GBufferPS( VS_OUTPUT In ) : SV_Target
{
	return float4( In.WorldPos, 1.0f );
};


//float PackColor2Float( float4 color )
//{
//	color = saturate( color );
//	float packed;
//	uint c = color.x * 255;
//	packed = 
//};

float4 BlinPhongShading( float3 worldpos, float3 worldnormal )
{
	float3 camerapos = float3( g_mView[3][0], g_mView[3][1], g_mView[3][2] );
    float3 vL = normalize( g_LightPos - worldpos );
	float3 vH = normalize( normalize( camerapos - worldpos ) + vL );
	float3 vNorm = normalize( worldnormal );
    float NdotL = dot( vL, vNorm );
    float diffusecoeff = max( NdotL, 0 ) * 5;
	// Blin phong actually
	float NdotH = dot( vH, vNorm );
	float specularcoeff = pow( max( NdotH, 0 ), Shininess );
	specularcoeff = 0;
    float4 color = Emission + Ambient + Diffuse * diffusecoeff + Specular * specularcoeff;

	return color;	
};

float4 BlinPhongShadingLightOnly( float3 worldpos, float3 worldnormal )
{
	const float lightradius = 4;
	float3 camerapos = float3( g_mView[3][0], g_mView[3][1], g_mView[3][2] );
    float3 vL = normalize( g_LightPos - worldpos );
	float4 falloff = max( 0.0f, 1.0f - dot( vL, vL ) / lightradius );
	float3 vH = normalize( normalize( camerapos - worldpos ) + vL );
	float3 vNorm = normalize( worldnormal );
    float NdotL = dot( vL, vNorm );
    float diffusecoeff = max( NdotL, 0 ) * falloff;
	// Blin phong actually
	float NdotH = dot( vH, vNorm );
	float specularcoeff = pow( max( NdotH, 0 ), Shininess ) * falloff;
	
    float4 color = Diffuse * diffusecoeff + Specular * specularcoeff;

	return color;	
};

struct PS_OUT
{
	float4 Color : SV_TARGET0;
	float4 WorldPos : SV_TARGET1;
};

PS_OUT CreateLightSourcePS( VS_OUTPUT In )
{ 
	PS_OUT Out;
	
	float4 c = BlinPhongShadingLightOnly( In.WorldPos, In.Norm );
	const float threshold = 0.01f;
	if( dot( In.WorldPos, In.WorldPos ) > threshold )
	{
		Out.WorldPos = float4( In.WorldPos, 1.0f );
	}
	else
	{
		Out.WorldPos = float4( In.WorldPos, 0.0f );
	};
	Out.Color = c;
	
	return Out;
}

float4 CompositionPS( VS_OUTPUT In ) : SV_Target0
{
	float4 scattered = g_ScatteredSubsurfaceLight.Sample( TriLinearTextureSampler, In.ScreenUV );
	float4 phong = BlinPhongShading( In.WorldPos, In.Norm );
	scattered *= 0.03f;
	//scattered /= scattered.w;
	//return phong;
	if( UseSSSS )
	{
		return float4( scattered.rgb + phong.rgb, 1.0f ); 
	}
	else
	{
		return float4( phong.rgb, 1.0f );
	}
	//return float4( phong.rgb, 1.0f );
};
//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
DepthStencilState NoDepth
{
	DepthEnable = False;
};

technique10 ImageSpaceSubsurfaceScattering
{
    pass CreateLightSource
    {       
		SetVertexShader( CompileShader( vs_4_0, RenderSceneVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, CreateLightSourcePS() ) );

        SetBlendState( NoBlending, float4(0, 0, 0, 0), 0xffffffff );
		SetDepthStencilState( NoDepth, 0 );
    }

    pass CreateGBuffer
    {       
		SetVertexShader( CompileShader( vs_4_0, RenderSceneVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, GBufferPS() ) );
        
    }
    
	pass Splatting
    {       
		SetVertexShader( CompileShader( vs_4_0, SplatVS() ) );
        SetGeometryShader( CompileShader( gs_4_0, SplatGS() ) );
        SetPixelShader( CompileShader( ps_4_0, SplatPS() ) );
        
        SetBlendState( Additive, float4(0, 0, 0, 0), 0xffffffff );
    }

    pass Composition
    {       
		SetVertexShader( CompileShader( vs_4_0, RenderSceneVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, CompositionPS() ) );
        
        SetBlendState( NoBlending, float4(0, 0, 0, 0), 0xffffffff );
    }
	
	pass DebugRenderQuad
	{
		SetVertexShader( CompileShader( vs_4_0, RenderQuadVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderQuadPS() ) );
        
        SetBlendState( NoBlending, float4(0, 0, 0, 0), 0xffffffff );		
	}
}
