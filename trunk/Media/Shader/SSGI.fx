//--------------------------------------------------------------------------------------
// File: SSGI.fx
//--------------------------------------------------------------------------------------
float Script : STANDARDSGLOBAL <
	string UIWidget = "none";
	string ScriptClass = "sceneorobject";
	string ScriptOrder = "standard";
	string ScriptOutput = "color";
	string Script = "Technique=SSGI;";
> = 0.8; // version #

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

Texture2D GBuffer_Pos : RENDERCOLORTARGET <
    float2 ViewPortRatio = {1.0,1.0};
    string Format = "A32B32G32R32F" ; 
    int MipLevels = 1;
    string UIWidget = "None";
>;

Texture2D GBuffer_Normal : RENDERCOLORTARGET<
    float2 ViewPortRatio = {1.0,1.0};
    string Format = "A32B32G32R32F" ; 
    int MipLevels = 1;
    string UIWidget = "None";
>;

Texture2D Scene_Color : RENDERCOLORTARGET<
    float2 ViewPortRatio = {1.0,1.0};
    string Format = "A8R8G8B8" ; 
    int MipLevels = 1;
    string UIWidget = "None";
>;


Texture2D DepthBuffer : RENDERDEPTHSTENCILTARGET <
    float2 ViewPortRatio = {1.0,1.0};
    string Format = "D24S8";
    string UIWidget = "None";
>;

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

#define SamplesPerCircle 8
#define NumOfCircles 2
#define SamplingRadius 0.2f	// texture space
#define Distance_Threshold 0.2f

cbuffer cbConstant
{
	// @note: pointing to where the light is going, not coming
    float3 vLightDir <
	> = float3(-1, 0, 0);
	
	float2 SampleOffsets[ SamplesPerCircle * NumOfCircles ]<
	> = 
	{
			SamplingRadius / NumOfCircles * float2(1.0f, 0.0f),
			SamplingRadius / NumOfCircles * float2(0.707f, 0.707f),
			SamplingRadius / NumOfCircles * float2(0.0f, 1.0f),
			SamplingRadius / NumOfCircles * float2(-0.707f, 0.707f),
			SamplingRadius / NumOfCircles * float2(-1.0f, 0.0f),
			SamplingRadius / NumOfCircles * float2(-0.707f, -0.707f),
			SamplingRadius / NumOfCircles * float2(0.0f, -1.0f),
			SamplingRadius / NumOfCircles * float2(0.707f, -0.707f),

			2.0f * SamplingRadius / NumOfCircles * float2(1.0f, 0.0f),
			2.0f * SamplingRadius / NumOfCircles * float2(0.707f, 0.707f),
			2.0f * SamplingRadius / NumOfCircles * float2(0.0f, 1.0f),
			2.0f * SamplingRadius / NumOfCircles * float2(-0.707f, 0.707f),
			2.0f * SamplingRadius / NumOfCircles * float2(-1.0f, 0.0f),
			2.0f * SamplingRadius / NumOfCircles * float2(-0.707f, -0.707f),
			2.0f * SamplingRadius / NumOfCircles * float2(0.0f, -1.0f),
			2.0f * SamplingRadius / NumOfCircles * float2(0.707f, -0.707f),
	};
	
	
	float SampleWeights[ NumOfCircles ] = { 0.8f, 0.2f };
};

cbuffer cbChangesEveryFrame
{
    matrix World : World;
    matrix View : View;
    matrix Projection : Projection;
	matrix WVP : WorldViewProjection;
};

cbuffer cbUserChanges
{
	float3 Scene_Ambient <
		string UIName = "Ambient Color";
		string UIWidget = "Color";
	> = float3( 0.1f, 0.1f, 0.1f );
	
	float3 Scene_Diffuse <
		string UIName = "Diffuse Color";
		string UIWidget = "Color";
	> = float3( 0.9f, 0.05f, 0.05f );
	
	float4 Clear_Color <
	> = float4( 0, 0, 0, 0);
	
	float Clear_Depth <
	> = 1.0f;
};

struct VS_INPUT
{
    float3 Pos          : POSITION;         
    float3 Normal         : NORMAL;           
    float2 Tex          : TEXCOORD0;        
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 World_Pos : TEXCOORD0;
    float3 World_Normal : TEXCOORD1;
};

struct PS_GEOMETRY_OUTPUT
{
	float4 Color0 : SV_Target0;
	float4 Color1 : SV_Target1;
	float4 Color2 : SV_Target2;
};

struct QuadVertexOutput {
    float4 Position	: SV_POSITION;
    float2 UV	: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT Geometry_VS( VS_INPUT input )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Pos = mul( float4( input.Pos, 1 ), WVP );
    output.World_Pos = mul( float4( input.Pos, 1 ), World ).xyz;
    output.World_Normal = mul( input.Normal, ( float3x3 )World );
    
    return output;
};


QuadVertexOutput GI_VS(
    float3 Position : POSITION, 
    float3 TexCoord : TEXCOORD0
) 
{    QuadVertexOutput OUT;
    OUT.Position = float4( Position, 1 );
	OUT.UV = float2( TexCoord.xy ); 
    
	return OUT;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
PS_GEOMETRY_OUTPUT Geometry_PS( VS_OUTPUT Input)
{
	PS_GEOMETRY_OUTPUT Out = ( PS_GEOMETRY_OUTPUT )0;
	
	Out.Color0 = float4( Input.World_Pos, 1 );
	Out.Color1 = float4( Input.World_Normal, 1 );
	
	// Basic phong
	float3 LDir = normalize( -1.0f * vLightDir );
	float3 phong = Scene_Ambient + max( 0, dot( normalize( Input.World_Normal ), LDir ) ) * Scene_Diffuse;
    Out.Color2 = float4( phong, 1.0f );
	return Out;
}

float4 GI_PS( QuadVertexOutput In ) : SV_Target
{
	float4 SSGI = float4(0, 0, 0, 1);
	
	float3 pos = GBuffer_Pos.SampleLevel( samPoint, In.UV, 0 ).xyz;
	float3 norm = GBuffer_Normal.SampleLevel( samPoint, In.UV, 0 ).xyz;
	float4 color = Scene_Color.SampleLevel( samPoint, In.UV, 0 );
	norm = normalize( norm );
	for( uint i = 0; i < SamplesPerCircle * NumOfCircles; i++ )
	{
		float3 s_pos = GBuffer_Pos.SampleLevel( samPoint, In.UV + SampleOffsets[ i ], 0 ).xyz;
		float3 s_norm = GBuffer_Normal.SampleLevel( samPoint, In.UV + SampleOffsets[ i ], 0 ).xyz;
		s_norm = normalize( s_norm );
		float4 s_color = Scene_Color.SampleLevel( samPoint, In.UV + SampleOffsets[ i ], 0 );
		
		float3 vL = normalize( s_pos - pos );	
		float lambert = max( 0, dot( vL, norm ) ) * max( 0, dot( -1.0f * vL, s_norm ) );
		float scale = pow( max( 1, length( s_pos - pos ) / Distance_Threshold ), -3.0f );
		SSGI += lambert * s_color * scale * SampleWeights[ i / SamplesPerCircle ];
	};
	SSGI /= SamplesPerCircle * NumOfCircles * 0.2f;
	
	return SSGI + color;
};

void Test_PS( VS_OUTPUT In, out float4 c0 : SV_Target0, out float4 c1 : SV_Target1, out float4 c2 : SV_Target2 )
{	
	// Basic phong
	float3 LDir = normalize( vLightDir );
	float3 phong = Scene_Ambient + max( 0, dot( normalize( In.World_Normal ), LDir ) ) * Scene_Diffuse;
    c0 = float4( 0.5f, 0.9f, 0.3f, 1.0f );
	c1 = float4( 1234.5f, 0.2f, 0.3f, 1.0f );
	c2 = float4( phong, 1.0f );
};

//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState DisableDepth
{
	DepthEnable = FAlSE;
};

BlendState NoBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[ 0 ] = FALSE;
	BlendEnable[ 1 ] = FALSE;
	BlendEnable[ 2 ] = FALSE;	
};

RasterizerState RasterizerSettings
{
	CullMode = NONE;
};



//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique10 SSGI 
<
	string Script = 
		"Pass=Geometry;"
		"Pass=GI;";
> {
    pass Geometry 
	<
  		string Script = 
			"RenderColorTarget0=GBuffer_Pos;"
			"RenderColorTarget1=GBuffer_Normal;"
			"RenderColorTarget2=Scene_Color;"
			"ClearSetColor=Clear_Color;"
			"ClearSetDepth=Clear_Depth;"
			"Clear=Color;"
			"Clear=Depth;"
			"Draw=geometry;";  
	> {
        SetVertexShader( CompileShader( vs_4_0, Geometry_VS() ) );
        SetPixelShader( CompileShader( ps_4_0, Geometry_PS() ) );
        
        SetDepthStencilState( EnableDepth, 0 );
        SetRasterizerState(RasterizerSettings); 
    }
	
	pass GI 
	<
	  	string Script = 
			"RenderColorTarget0=;"
			"RenderColorTarget1=;"
			"RenderColorTarget2=;"
			"RenderDepthStencilTarget=;"
			"ClearSetColor=Clear_Color;"
			"ClearSetDepth=Clear_Depth;"
			"Clear=Color;"
			"Clear=Depth;"
			"Draw=buffer;";  
	> {
        SetVertexShader( CompileShader( vs_4_0, GI_VS() ) );
        SetPixelShader( CompileShader( ps_4_0, GI_PS() ) );
		
		SetDepthStencilState( DisableDepth, 0 );
       SetRasterizerState(RasterizerSettings); 
 	}
}


