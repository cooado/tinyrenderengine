cbuffer cb0
{
	/**
	* @MouseParam float4( MousePosX, MousePosY, MouseWidth, MouseHeight )
	* 	All params in projection space.
	*/
	float4 MouseParam = float4( 0, 0, 0.05, 0.06 );
};

Texture2D Mouse;

BlendState MouseBlend
{
	BlendEnable[ 0 ] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
};

SamplerState DiffuseTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
    //Filter = ANISOTROPIC;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct PsIn_Scene
{
	float4 position : SV_Position;
	float2 UV : TexCoord0;
};

PsIn_Scene MouseVS( uint i : SV_VertexID )
{
	PsIn_Scene Out;

	if( i == 0 )
	{
		Out.position = float4( MouseParam.x, MouseParam.y, 0, 1 );
		Out.UV = float2( 0, 0 );
	}
	else if( i == 1 )
	{
		Out.position = float4( MouseParam.x + MouseParam.z, MouseParam.y, 0, 1 );
		Out.UV = float2( 1, 0 );
	}
	else if( i == 2 )
	{
		Out.position = float4( MouseParam.x, MouseParam.y - MouseParam.w, 0, 1 );
		Out.UV = float2( 0, 1 );
	}
	else if( i == 3 )
	{
		Out.position = float4( MouseParam.x + MouseParam.z, MouseParam.y - MouseParam.w, 0, 1 );
		Out.UV = float2( 1, 1 );
	};
	
	return Out;
}

float4 MousePS(PsIn_Scene In) : SV_Target0
{	
	float4 Color = Mouse.Sample( DiffuseTextureSampler, In.UV );
	
	return Color;
}

// Volume decal technique
technique10 RenderMouse
{
    pass Mouse
    {       
		SetVertexShader( CompileShader( vs_4_0, MouseVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, MousePS() ) );
        
		SetBlendState( MouseBlend, float4( 0, 0, 0, 0 ), 0xffffffff );
    }
};