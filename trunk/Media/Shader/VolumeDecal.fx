cbuffer cb0
{
	float4x4 g_mWorldViewProjection : WORLDVIEWPROJECTION;    // World * View * Projection matrix
	float4x4 g_mViewProjection : VIEWPROJECTION;
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

cbuffer cb2
{
	// Decal position
	float3 Pos;
	
	// Decal radius
	float Radius;
	
	// Decal color
	float4 Color;
	
	float4x4 ScreenToLocal;
	
	float2 PixelSize;
};

Texture2D Base;

Texture2D Depth;

Texture3D Decal;

RasterizerState rs_face_ccw
{
	CullMode = None;
};

RasterizerState DecalRS
{
	CullMode = None;
};

BlendState DecalBlend
{
	BlendEnable[ 0 ] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
};

DepthStencilState DecalDepth
{
	DepthEnable = FALSE;
};

SamplerState DecalFilter
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border;
	AddressV = Border;
	AddressW = Border;
	BorderColor = float4( 0, 0, 0, 0 );
};

SamplerState DepthFilter
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};

SamplerState DiffuseTextureSampler
{
	//Filter = MIN_MAG_MIP_LINEAR;
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct VsIn_Scene
{
	float3 position : Position;
	float3 texCoord : TexCoord;
	float3 normal   : Normal;
};

struct PsIn_Scene
{
	float4 position : SV_Position;
	float2 texCoord : TexCoord0;
	float z   : TexCoord1;
};

// Pass scene shaders
PsIn_Scene SceneVS(VsIn_Scene In)
{
	PsIn_Scene Out;

	Out.position = mul( float4( In.position, 1.0f ), g_mWorldViewProjection );
	Out.texCoord = In.texCoord;
	Out.z = Out.position.z / Out.position.w;
	
	return Out;
}

struct PS_OUT
{
	float4 Color : SV_Target0;
	
	float Depth : SV_Target1;
};

PS_OUT ScenePS(PsIn_Scene In)
{
	PS_OUT Out;
	
	Out.Color = Base.Sample( DiffuseTextureSampler, In.texCoord.xy );
	Out.Depth = In.z;
	
	return Out;
}

struct PsIn_Decal
{
	float4 Position : SV_Position;
};

// Pass decal shaders

PsIn_Decal DecalVS( VsIn_Scene In )
{
	PsIn_Decal Out;

	float4 P = float4( In.position, 1.0f );
	P.xyz *= Radius;
	P.xyz += Pos;

	Out.Position = mul( P, g_mWorldViewProjection );

	return Out;
}

float4 DecalPS(PsIn_Decal In) : SV_Target
{
    // Compute normalized screen position
	float2 texCoord = In.Position.xy * PixelSize;

    // Compute local position of scene geometry
	float depth = Depth.Sample(DepthFilter, texCoord).x;
	texCoord.y = 1.0f - texCoord.y;
	texCoord = texCoord * 2.0f - 1.0f;
	float4 scrPos = float4(texCoord, depth, 1.0f);
	float4 wPos = mul( scrPos, ScreenToLocal );

    // Sample decal
	float3 coord = wPos.xyz / wPos.w;
	float decal = Decal.Sample(DecalFilter, coord).r;

	return float4( Color.rgb, decal );
}

// Volume decal technique
technique10 RenderDecal
{
    pass Scene
    {       
		SetVertexShader( CompileShader( vs_4_0, SceneVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ScenePS() ) );
        
        SetRasterizerState( rs_face_ccw );
    }
		
	pass Decal
	{
		SetVertexShader( CompileShader( vs_4_0, DecalVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, DecalPS() ) );
        
        SetRasterizerState( DecalRS );
		SetDepthStencilState( DecalDepth, 0 );
		SetBlendState( DecalBlend, float4( 0, 0, 0, 0 ), 0xffffffff );
	}
}