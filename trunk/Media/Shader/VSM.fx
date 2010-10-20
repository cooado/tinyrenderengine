//--------------------------------------------------------------------------------------
// File: ColladaCommonEffect.fx
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
cbuffer cb0
{
	float4x4 g_mWorld : WORLD;                  // World matrix for object
	float4x4 g_mView : VIEW;
	float4x4 g_mWorldViewProjection : WORLDVIEWPROJECTION;    // World * View * Projection matrix
	
	float4x4 g_mLightView;
	float4x4 g_mLightProjection;
	float4x4 g_mLightWorldViewProjection;
	float4x4 g_mLightViewProjection;
	
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
	
	/**
	*	0	VSM
	*	1	ESM
	*	2	SM
	*/
	uint ShadowMethod = 2;
}

Buffer< uint2 > JointIndex;
Buffer< float4 > JointBind;
Buffer< float > JointWeight;

Texture2D g_DiffuseTex;              // Color texture for mesh
Texture2D g_VSM;

RasterizerState rs_face_ccw
{
	CullMode = None;
};

SamplerState DiffuseTextureSampler
{
	//Filter = MIN_MAG_MIP_LINEAR;
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState VSMSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	//Filter = ANISOTROPIC;
	AddressU = Clamp;
	AddressV = Clamp;
};

SamplerState PointSampler
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
    float3 Norm : NORMAL; 	// world norm
    float3 TextureUV  : TEXCOORD0;  // vertex texture coords
	float3 WorldPos : TEXCOORD1;
};

VS_OUTPUT RenderStaticVS( float3 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float3 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    
    Output.Position = mul( float4( vPos, 1.0f ), g_mWorldViewProjection);
    Output.Norm = normalize(mul(vNormal, (float3x3)g_mWorld)); // normal (world space)
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
							uint2 vSkeletalJoints : TEXCOORD1,
							uniform float4x4 wvp,
							uniform float4x4 world )
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
    Output.Position = mul( float4( SkinPos.xyz, 1.0f ), wvp);
	//Output.Position = float4( vPos, 0 );
    
    // Transform the normal from object space to world space    
    Output.Norm = normalize(mul(vNormal, (float3x3)wvp)); // normal (world space)
    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0;
	
	Output.WorldPos = mul( SkinPos, world ).xyz;
    
    return Output;   
};

float4 RenderVSMPS( VS_OUTPUT In ) : SV_Target
{ 
	// Stores in VSM the view space depth
	float2 moments = float2( In.Position.z, In.Position.z * In.Position.z );
	return moments.xyxy;
}

float4 RenderScenePS( VS_OUTPUT In ) : SV_Target
{ 
	// Shadowing
	float4 lightproj = mul( float4( In.WorldPos, 1.0f ), g_mLightViewProjection );
	float2 lightuv = lightproj.xy / lightproj.w * float2( 0.5f, -0.5f ) + 0.5f;

	float shadowfactor = 1.0f;

	if( dot( float2( 1, 1 ) >= lightuv, lightuv >= float2( 0, 0 ) ) == 2 )
	{
		if( ShadowMethod == 0 )
		{
			// VSM
			float2 moments = g_VSM.SampleLevel( VSMSampler, lightuv, 0 ).rg;
			moments += 0.5f * g_VSM.SampleLevel( VSMSampler, lightuv + float2( -2.0f/512.0f, 0), 0 ).rg;
			moments += 0.5f * g_VSM.SampleLevel( VSMSampler, lightuv + float2( 2.0f/512.0f, 0), 0 ).rg;
			moments += 0.5f * g_VSM.SampleLevel( VSMSampler, lightuv + float2( 0, -2.0f/512.0f), 0 ).rg;
			moments += 0.5f * g_VSM.SampleLevel( VSMSampler, lightuv + float2( 0, 2.0f/512.0f), 0 ).rg;
			moments /= 3.0f;
			
			const float light_shadow_bias = 0.001f;
			const float light_vsm_epsilon = 0.0001f;;
			float lit_factor = ( lightproj.z - light_shadow_bias <= moments.x);
				
			float E_x2 = moments.y;
			float Ex_2 = moments.x * moments.x;
			float variance = min(max(E_x2 - Ex_2, 0.0) + light_vsm_epsilon, 1.0);
			float m_d = ( moments.x - lightproj.z );
			float p = variance / (variance + m_d * m_d);
			
			// 0 for shadowed, 1 for unshadowed
			shadowfactor = 0.3f + max(lit_factor, p) * 0.7f;
		}
		else if( ShadowMethod == 1 )
		{
			// ESM
			float depth = g_VSM.SampleLevel( VSMSampler, lightuv, 0 ).r;
			depth += 0.5f * g_VSM.SampleLevel( VSMSampler, lightuv + float2( -2.0f/512.0f, 0 ), 0 ).r;
			depth += 0.5f * g_VSM.SampleLevel( VSMSampler, lightuv + float2( 2.0f/512.0f, 0 ), 0 ).r;
			depth += 0.5f * g_VSM.SampleLevel( VSMSampler, lightuv + float2( 0, -2.0f/512.0f), 0 ).r;
			depth += 0.5f * g_VSM.SampleLevel( VSMSampler, lightuv + float2( 0, 2.0f/512.0f), 0 ).r;
			depth /= 3.0f;
			const float light_shadow_bias = 0.001f;
			const float ESMFactor = -20.0f;
			//float lit_factor = ( lightproj.z - light_shadow_bias <= depth );
			shadowfactor = pow( 2.718281f, ESMFactor* ( lightproj.z - depth ) );
			//shadowfactor = min( lit_factor, shadowfactor );
			shadowfactor = saturate( shadowfactor );
			shadowfactor = 0.2f + shadowfactor * 0.8f;
		}
		else if( ShadowMethod == 2 )
		{
			// SM
			float depth = g_VSM.SampleLevel( PointSampler, lightuv, 0 ).r;
			const float light_shadow_bias = 0.01f;
			shadowfactor = ( lightproj.z - light_shadow_bias <= depth );
			shadowfactor = 0.3f + shadowfactor * 0.7f;
		};
	};
	
	// Phong shading
	float4 diffusecolor;
	if( UseDiffuseTexture == true )
	{
		diffusecolor = g_DiffuseTex.Sample( DiffuseTextureSampler, In.TextureUV.xy );
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

	float4 ret = diffusecolor;
    //float4 ret = Emission + Ambient + diffusecolor * diffusecoeff + Specular * specularcoeff;
	ret *= shadowfactor;
	ret = saturate( ret );
	return float4( pow( ret.xyz, 2.2 ), 1 );
}


//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique10 VSM
{
    pass CreateVSM
    {       
		SetVertexShader( CompileShader( vs_4_0, RenderSkeletalVS( g_mLightWorldViewProjection, g_mWorld ) ) );
// 		SetVertexShader( CompileShader( vs_4_0, RenderStaticVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderVSMPS() ) );
        
        SetRasterizerState( rs_face_ccw );
    }

	pass SkeletalUseVSM
	{
		SetVertexShader( CompileShader( vs_4_0, RenderSkeletalVS( g_mWorldViewProjection, g_mWorld ) ) );
// 		SetVertexShader( CompileShader( vs_4_0, RenderStaticVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderScenePS() ) );
        
        SetRasterizerState( rs_face_ccw );		
	}
	
	pass StaticUseVSM
	{
		SetVertexShader( CompileShader( vs_4_0, RenderStaticVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderScenePS() ) );
        
        SetRasterizerState( rs_face_ccw );		
	}
}