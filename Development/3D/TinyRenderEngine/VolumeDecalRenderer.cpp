#include "Prerequisites.h"

namespace TRE
{
	FLOAT32 saturate( FLOAT32 f )
	{
		f < 0 ? f = 0 : __noop;
		f > 1 ? f = 1 : __noop;
		return f;
	};

#	define B 0x1000
#	define BM 0xff

#	define N 0x1000
#	define NP 12
#	define NM 0xfff

#	define setup(i,b0,b1,r0,r1)\
	t = i + N;\
	b0 = ((int) t) & BM;\
	b1 = (b0 + 1) & BM;\
	r0 = t - (int) t;\
	r1 = r0 - 1;

#	define s_curve(t) (t * t * (3 - 2 * t))

	static int p[B + B + 2];
	static float g3[B + B + 2][3];
	static float g2[B + B + 2][2];
	static float g1[B + B + 2];

	static void normalize2(float v[2]){
		float s = 1.0f / sqrtf(v[0] * v[0] + v[1] * v[1]);
		v[0] *= s;
		v[1] *= s;
	}


	static void normalize3(float v[3]){
		float s = 1.0f / sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
	}

	void initNoise(){
		int i, j, k;

		for (i = 0; i < B; i++) {
			p[i] = i;

			g1[i] = (float) ((rand() % (B + B)) - B) / B;

			for (j = 0; j < 2; j++)
				g2[i][j] = (float) ((rand() % (B + B)) - B) / B;
			normalize2(g2[i]);

			for (j = 0; j < 3; j++)
				g3[i][j] = (float) ((rand() % (B + B)) - B) / B;
			normalize3(g3[i]);
		}

		while (--i) {
			k = p[i];
			p[i] = p[j = rand() % B];
			p[j] = k;
		}

		for (i = 0; i < B + 2; i++) {
			p[B + i] = p[i];
			g1[B + i] = g1[i];
			for (j = 0; j < 2; j++)
				g2[B + i][j] = g2[i][j];
			for (j = 0; j < 3; j++)
				g3[B + i][j] = g3[i][j];
		}
	}	

#	define at3(rx,ry,rz) (rx * q[0] + ry * q[1] + rz * q[2])
#	define lerp(t, a, b) (a + t * (b - a))
	float noise3(const float x, const float y, const float z){
		int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
		float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
		int i, j;

		setup(x, bx0,bx1, rx0,rx1);
		setup(y, by0,by1, ry0,ry1);
		setup(z, bz0,bz1, rz0,rz1);

		i = p[bx0];
		j = p[bx1];

		b00 = p[i + by0];
		b10 = p[j + by0];
		b01 = p[i + by1];
		b11 = p[j + by1];

		t  = s_curve(rx0);
		sy = s_curve(ry0);
		sz = s_curve(rz0);

		q = g3[b00 + bz0]; u = at3(rx0,ry0,rz0);
		q = g3[b10 + bz0]; v = at3(rx1,ry0,rz0);
		a = lerp(t, u, v);

		q = g3[b01 + bz0]; u = at3(rx0,ry1,rz0);
		q = g3[b11 + bz0]; v = at3(rx1,ry1,rz0);
		b = lerp(t, u, v);

		c = lerp(sy, a, b);

		q = g3[b00 + bz1]; u = at3(rx0,ry0,rz1);
		q = g3[b10 + bz1]; v = at3(rx1,ry0,rz1);
		a = lerp(t, u, v);

		q = g3[b01 + bz1]; u = at3(rx0,ry1,rz1);
		q = g3[b11 + bz1]; v = at3(rx1,ry1,rz1);
		b = lerp(t, u, v);

		d = lerp(sy, a, b);

		return lerp(sz, c, d);
	};

	INT32 VolumeDecalRenderer::Initialize()
	{
		HRESULT result;

		// Construct TinyRT mesh for ray casting
		const std::set< Element* >& sms = TRE::ControllerManager::GetInstance()->m_pRRC->GetSceneStaticGeometries();
		TRE::ControllerManager::GetInstance()->m_pRCC->AddGeometry( *sms.begin() );

		// Generate the decal texture
		initNoise();

		UINT32 size = 128;
		UINT8 *dest = new UINT8[ size * size * size ];
		UINT8* memstart = dest;

		D3DXVECTOR3 pos;
		for (UINT32 z = 0; z < size; z++)
		{
			pos.z = z * (2.0f / (size - 1)) - 1.0f;
			for (UINT32 y = 0; y < size; y++)
			{
				pos.y = y * (2.0f / (size - 1)) - 1.0f;
				for (UINT32 x = 0; x < size; x++)
				{
					pos.x = x * (2.0f / (size - 1)) - 1.0f;

					float dot = sqrt( pos.x * pos.x + pos.y * pos.y + pos.z * pos.z );
					float pattern = 0;
					float strip = 0.2f;
					if( dot - strip * floor( dot / strip ) <= strip / 2 )
					{
						pattern = 0.9f;
					}
					else
					{
						pattern = 0.1f;
					};
					*dest++ = UINT8(pattern * 255.0f + 0.5f);
					//*dest++ = 255;
				}
			}
		}

		pDecalTexture = NULL;
		D3D10_TEXTURE3D_DESC Desc;
		Desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		Desc.CPUAccessFlags = 0;
		Desc.Depth = size;
		Desc.Height = size;
		Desc.Width = size;
		Desc.Format = DXGI_FORMAT_R8_UNORM;
		Desc.MipLevels = 1;
		Desc.MiscFlags = 0;
		Desc.Usage = D3D10_USAGE_IMMUTABLE;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = ( void* )memstart;
		InitData.SysMemPitch = size;
		InitData.SysMemSlicePitch = size * size;
		result = g_Renderer.pDevice->CreateTexture3D( &Desc, &InitData, &pDecalTexture );

		// Depth texture
		D3D10_TEXTURE2D_DESC Tex2DDesc;
		Tex2DDesc.Width = 640;
		Tex2DDesc.Height = 480;
		Tex2DDesc.MipLevels = 1;
		Tex2DDesc.ArraySize = 1;
		Tex2DDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		Tex2DDesc.CPUAccessFlags = 0;
		Tex2DDesc.Format = DXGI_FORMAT_R32_FLOAT;
		Tex2DDesc.MiscFlags = 0;
		Tex2DDesc.SampleDesc.Count = 1;
		Tex2DDesc.SampleDesc.Quality = 0;
		Tex2DDesc.Usage = D3D10_USAGE_DEFAULT;
		result = g_Renderer.pDevice->CreateTexture2D( &Tex2DDesc, NULL, &pDepthTexture );
		D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
		RTVDesc.Format = DXGI_FORMAT_R32_FLOAT;
		RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;
		result = g_Renderer.pDevice->CreateRenderTargetView( pDepthTexture, &RTVDesc, &pRTVDepth );
		D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
		SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		result = g_Renderer.pDevice->CreateShaderResourceView( pDepthTexture, &SRVDesc, &pSRVDepth );
		
		// Effect variables
		result = D3DX10CreateEffectFromFile( L"..\\..\\Media\\Shader\\VolumeDecal.fx", NULL, NULL, "fx_4_0", NULL, NULL, g_Renderer.pDevice, NULL, NULL, &pEffect, NULL, NULL );
		pScenePass = pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 0 );
		pDecalPass = pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 1 );

		pWorldViewProjection = pEffect->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();
		pViewProjection = pEffect->GetVariableByName( "g_mViewProjection" )->AsMatrix();

		pDecalPos = pEffect->GetVariableByName( "Pos" )->AsVector();
		pDecalRadius = pEffect->GetVariableByName( "Radius" )->AsScalar();
		pDecalColor = pEffect->GetVariableByName( "Color" )->AsVector();
		pDecalMatrix = pEffect->GetVariableByName( "ScreenToLocal" )->AsMatrix();
		pPixelSize = pEffect->GetVariableByName( "PixelSize" )->AsVector();

		pBase = pEffect->GetVariableByName( "Base" )->AsShaderResource();
		pDepth = pEffect->GetVariableByName( "Depth" )->AsShaderResource();
		pDecal = pEffect->GetVariableByName( "Decal" )->AsShaderResource();

		SRVDesc.Format = DXGI_FORMAT_R8_UNORM;
		SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE3D;
		SRVDesc.Texture3D.MipLevels = 1;
		SRVDesc.Texture3D.MostDetailedMip = 0;
		result = g_Renderer.pDevice->CreateShaderResourceView( pDecalTexture, &SRVDesc, &pSRVDecal );

		pDepth->SetResource( pSRVDepth );
		pDecal->SetResource( pSRVDecal );

		INT32 x;
		INT32 y;
		TRE::TinyRenderEngine::GetInstance()->GetScreenResolution( x, y );
		fPixelSize[ 0 ] = 1.0f / x;
		fPixelSize[ 1 ] = 1.0f / y;

		// Input layout
		D3D10_INPUT_ELEMENT_DESC pDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 }
		};
		D3D10_PASS_DESC PassDesc;
		result = pScenePass->GetDesc( &PassDesc );
		result = g_Renderer.pDevice->CreateInputLayout( pDescs, 3, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pILScene );

		// Add to mouse controller
		TRE::ControllerManager::GetInstance()->m_pIC->AppendListener( this );

		return 0;
	};

	void VolumeDecalRenderer::OnMouse( const OIS::Mouse* m )
	{
		const OIS::MouseState& state = m->getMouseState();
		if ( state.buttonDown( OIS::MB_Right ) )
		{
			static FLOAT64 freeze = 0;
			if( ControllerManager::GetInstance()->m_pTC->GetFreezeState( freeze ) )
			{
				return;
			};
			freeze = ControllerManager::GetInstance()->m_pTC->StartFreeze();

			RayCastController::RayTestResult result;
			if ( TRE::ControllerManager::GetInstance()->m_pRCC->RayTestScreenSpace( state.X.abs, state.Y.abs, result ) )
			{
				const FLOAT32 radius = 15;

				Decal decal;
				decal.position = result.HitPoint;
				decal.radius = radius;
				decal.color = D3DXCOLOR( 0.984f, 1.0f, 0.288f, 1.0f );
				D3DXMATRIX trans1;
				D3DXMatrixTranslation( &trans1, -1 * result.HitPoint.x, -1 * result.HitPoint.y, -1 * result.HitPoint.z );
				D3DXMATRIX scale;
				D3DXMatrixScaling( &scale, 0.5f / radius, 0.5f / radius, 0.5f / radius );
				D3DXMATRIX trans2;
				D3DXMatrixTranslation( &trans2, 0.5f, 0.5f, 0.5f );
				D3DXMATRIX mat;
				D3DXMatrixMultiply( &mat, &trans1, &scale );
				D3DXMatrixMultiply( &mat, &mat, &trans2 );
				decal.matrix = mat;
				//decal.matrix = translate(0.5f, 0.5f, 0.5f) * scale(0.5f / radius, 0.5f / radius, 0.5f / radius) * rotateZXY(x, y, z) * translate(-pos);

				m_vDecals.push_back( decal );
			}
		}
	};

	INT32 VolumeDecalRenderer::Finalize()
	{
		pDecalTexture->Release();
		pDepthTexture->Release();
		pRTVDepth->Release();
		pEffect->Release();
		pSRVDepth->Release();
		pSRVDecal->Release();

		pILScene->Release();

		return 0; 
	};

	INT32 VolumeDecalRenderer::Render()
	{
		//HRESULT result;

		D3DXMATRIX View = ControllerManager::GetInstance()->m_pCC->GetViewMatrix();
		D3DXMATRIX Projection = ControllerManager::GetInstance()->m_pCC->GetProjectionMatrix();
		D3DXMATRIX ViewProjection;
		D3DXMatrixMultiply( &ViewProjection, &View, &Projection );
		D3DXMATRIX InvViewProjection;
		D3DXMatrixInverse( &InvViewProjection, NULL, &ViewProjection );

		const std::set< Element* >& elems = ControllerManager::GetInstance()->m_pRRC->GetSceneStaticGeometries();
		Element* elem = *elems.begin();

		D3DXMATRIX WorldViewProjection;
		D3DXMATRIX World = elem->pNode->GetWorldMatrix();
		D3DXMatrixMultiply( &WorldViewProjection, &World, &ViewProjection );
		D3DXMATRIX InvWorldViewProjection;
		D3DXMatrixInverse( &InvWorldViewProjection, NULL, &WorldViewProjection );

		pWorldViewProjection->SetMatrix( ( float* )WorldViewProjection );
		pViewProjection->SetMatrix( ( float* )ViewProjection );

		// Render scene
		FLOAT32 ClearColor[ 1 ] = { 1.0f };
		g_Renderer.pDevice->ClearRenderTargetView( pRTVDepth, ClearColor );
		ID3D10RenderTargetView* RTVs[] = { g_Renderer.pRTV, pRTVDepth };
		g_Renderer.pDevice->OMSetRenderTargets( 2, RTVs, g_Renderer.pDSV );
		elem->pStaticMesh->pGeometry->vMesh[ 0 ]->SetBuffersToDevice();
		pBase->SetResource( elem->pStaticMesh->pMaterial->vParams[ 0 ]->pDiffuse );
		g_Renderer.pDevice->IASetInputLayout( pILScene );
		pScenePass->Apply( 0 );
		g_Renderer.pDevice->DrawIndexed( elem->pStaticMesh->pGeometry->vMesh[ 0 ]->IndexCount, elem->pStaticMesh->pGeometry->vMesh[ 0 ]->IndexStartLocation, 0 );


		// Render decal
		RTVs[ 1 ] = NULL;
		g_Renderer.pDevice->OMSetRenderTargets( 2, RTVs, g_Renderer.pDSV );
		Geometry* geom = ControllerManager::GetInstance()->m_pRRC->GetGeometryByName( std::string( "Sphere01" ) );
		geom->vMesh[ 0 ]->SetBuffersToDevice();
		const UINT32 decal_count = m_vDecals.size();
		for (UINT32 i = 0; i < decal_count; i++)
		{
			D3DXVECTOR3 pos = m_vDecals[ i ].position;
			D3DXCOLOR color = m_vDecals[ i ].color;
			FLOAT32 radius = m_vDecals[ i ].radius;
			pDecalPos->SetFloatVector( &pos.x );
			pDecalRadius->SetFloat( radius );
			pDecalColor->SetFloatVector( &color.r );
			pPixelSize->SetFloatVector( fPixelSize );
			D3DXMATRIX mat;
			D3DXMatrixMultiply( &mat, &InvWorldViewProjection, &m_vDecals[ i ].matrix );
			pDecalMatrix->SetMatrix( ( float* )mat );

			pDecalPass->Apply( 0 );
			g_Renderer.pDevice->DrawIndexed( geom->vMesh[ 0 ]->IndexCount, geom->vMesh[ 0 ]->IndexStartLocation, 0 );
		};

		return 0;
	};
};