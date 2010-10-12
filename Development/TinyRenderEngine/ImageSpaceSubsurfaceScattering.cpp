#include "Prerequisites.h"

namespace TRE
{
	INT32 ImageSpaceSubsurfaceScattering::Initialize()
	{
		LightSourceWidth = 48;
		LightSourceHeight = 48;
		ScatterWidth = 320;
		ScatterHeight = 240;
		m_bUseSSSS = true;

		pEffect = NULL;
		HRESULT result = D3DX10CreateEffectFromFile( L"..\\..\\Media\\Shader\\ImageSpaceSubsurfaceScattering.fx", NULL, NULL, "fx_4_0", NULL, NULL, g_Renderer.pDevice, NULL, NULL, &pEffect, NULL, NULL );

		pWorld = pEffect->GetVariableByName( "g_mWorld" )->AsMatrix();
		pView = pEffect->GetVariableByName( "g_mView" )->AsMatrix();
		pProjection = pEffect->GetVariableByName( "g_mProjection" )->AsMatrix();
		pWorldViewProjection = pEffect->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();
		pLightPos = pEffect->GetVariableByName( "g_LightPos" )->AsVector();
		pLightDiffuseColor = pEffect->GetVariableByName( "g_LightDiffuseColor" )->AsVector();

		pUseSSSS = pEffect->GetVariableByName( "UseSSSS" )->AsScalar();
		pEmission = pEffect->GetVariableByName( "Emission" )->AsVector();
		pAmbient = pEffect->GetVariableByName( "Ambient" )->AsVector();
		pDiffuse = pEffect->GetVariableByName( "Diffuse" )->AsVector();
		pSpecular = pEffect->GetVariableByName( "Specular" )->AsVector();
		pShininess = pEffect->GetVariableByName( "Shininess" )->AsScalar();
		pUseDiffuseTexture = pEffect->GetVariableByName( "UseDiffuseTexture" )->AsScalar();
		pcatterRadiusSquared = pEffect->GetVariableByName( "g_ScatterRadiusSquared" )->AsVector();
		pLightSourceResolution = pEffect->GetVariableByName( "LightSourceResolution" )->AsVector();
		pSplatRadius = pEffect->GetVariableByName( "SplatRadius" )->AsScalar();
		pTangentHalfFOVXY = pEffect->GetVariableByName( "TangentHalfFOVXY" )->AsVector();

		pScatteredSubsurfaceLight = pEffect->GetVariableByName( "g_ScatteredSubsurfaceLight" )->AsShaderResource();
		pLightSource = pEffect->GetVariableByName( "g_LightSource" )->AsShaderResource();
		pLightDiffuse = pEffect->GetVariableByName( "g_LightDiffuse" )->AsShaderResource();
		pGBuffer = pEffect->GetVariableByName( "g_GBuffer" )->AsShaderResource();
		pScatterTexture = pEffect->GetVariableByName( "g_ScatterTexture" )->AsShaderResource();

		pSRV = NULL;
		D3DX10CreateShaderResourceViewFromFile( g_Renderer.pDevice, L"../../Media/Collada/Images/translucency.jpg", NULL, NULL, &pSRV, &result);
		pScatterTexture->SetResource( pSRV );


		// Create depth stencil views
		D3D10_TEXTURE2D_DESC TextureDesc;
		TextureDesc.Width = LightSourceWidth;
		TextureDesc.Height = LightSourceHeight;
		TextureDesc.MipLevels = 1;
		TextureDesc.ArraySize = 1;
		TextureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		TextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.Usage = D3D10_USAGE_DEFAULT;
		TextureDesc.CPUAccessFlags = 0;
		TextureDesc.MiscFlags = 0;
		result = g_Renderer.pDevice->CreateTexture2D( &TextureDesc, NULL, &pDSLightSource );
		TextureDesc.Width = ScatterWidth;
		TextureDesc.Height = ScatterHeight;
		result = g_Renderer.pDevice->CreateTexture2D( &TextureDesc, NULL, &pDSScattered );

		D3D10_DEPTH_STENCIL_VIEW_DESC DSDesc;
		DSDesc.Format = DXGI_FORMAT_D32_FLOAT;
		DSDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		DSDesc.Texture2D.MipSlice = 0;
		result = g_Renderer.pDevice->CreateDepthStencilView( pDSLightSource, &DSDesc, &pDSVLightSource );
		result = g_Renderer.pDevice->CreateDepthStencilView( pDSScattered, &DSDesc, &pDSVScattered );

		// Create textures
		D3D10_TEXTURE2D_DESC ShaderDesc;
		ShaderDesc.Width = ScatterWidth;
		ShaderDesc.Height = ScatterHeight;
		ShaderDesc.MipLevels = 1;
		ShaderDesc.ArraySize = 1;
		ShaderDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		ShaderDesc.SampleDesc.Count = 1;
		ShaderDesc.SampleDesc.Quality = 0;
		ShaderDesc.Usage = D3D10_USAGE_DEFAULT;
		ShaderDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		ShaderDesc.CPUAccessFlags = 0;
		ShaderDesc.MiscFlags = 0;
		result = g_Renderer.pDevice->CreateTexture2D( &ShaderDesc, NULL, &pTexScatteredSubsurfaceLight);

		ShaderDesc.Width = LightSourceWidth;
		ShaderDesc.Height = LightSourceHeight;
		result = g_Renderer.pDevice->CreateTexture2D( &ShaderDesc, NULL, &pTexLightSource);
		
		ShaderDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		ShaderDesc.Width = LightSourceWidth;
		ShaderDesc.Height = LightSourceHeight;
		result = g_Renderer.pDevice->CreateTexture2D( &ShaderDesc, NULL, &pTexLightDiffuse );
		
		ShaderDesc.Width = ScatterWidth;
		ShaderDesc.Height = ScatterHeight;
		ShaderDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		result = g_Renderer.pDevice->CreateTexture2D( &ShaderDesc, NULL, &pTexGBuffer );


		// Create render target veiws
		D3D10_RENDER_TARGET_VIEW_DESC ShaderRTVDesc;
		ShaderRTVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		ShaderRTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
		ShaderRTVDesc.Texture2D.MipSlice = 0;
		result = g_Renderer.pDevice->CreateRenderTargetView( pTexScatteredSubsurfaceLight, &ShaderRTVDesc, &pRTVScatteredSubsurfaceLight );
		result = g_Renderer.pDevice->CreateRenderTargetView( pTexLightSource, &ShaderRTVDesc, &pRTVLightSource );
		ShaderRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		result = g_Renderer.pDevice->CreateRenderTargetView( pTexLightDiffuse, &ShaderRTVDesc, &pRTVLightDiffuse );
		ShaderRTVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		result = g_Renderer.pDevice->CreateRenderTargetView( pTexGBuffer, &ShaderRTVDesc, &pRTVGBuffer );


		// Create shader resource view
		D3D10_SHADER_RESOURCE_VIEW_DESC ShaderSRVDesc;
		ShaderSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		ShaderSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		ShaderSRVDesc.Texture2D.MipLevels = 1;
		ShaderSRVDesc.Texture2D.MostDetailedMip = 0;
		result = g_Renderer.pDevice->CreateShaderResourceView( pTexScatteredSubsurfaceLight, &ShaderSRVDesc, &pSRVScatteredSubsurfaceLight );
		result = pScatteredSubsurfaceLight->SetResource( pSRVScatteredSubsurfaceLight );

		result = g_Renderer.pDevice->CreateShaderResourceView( pTexLightSource, &ShaderSRVDesc, &pSRVLightSource );
		result = pLightSource->SetResource( pSRVLightSource );

		ShaderSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		result = g_Renderer.pDevice->CreateShaderResourceView( pTexLightDiffuse, &ShaderSRVDesc, &pSRVLightDiffuse );
		result = pLightDiffuse->SetResource( pSRVLightDiffuse );

		ShaderSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		result = g_Renderer.pDevice->CreateShaderResourceView( pTexGBuffer, &ShaderSRVDesc, &pSRVGBuffer );
		result = pGBuffer->SetResource( pSRVGBuffer );

		// Create input layout
		D3D10_INPUT_ELEMENT_DESC pDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 }
		};
		D3D10_PASS_DESC PassDesc;
		result = pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 0 )->GetDesc( &PassDesc );
		pInputLayout = NULL;
		result = g_Renderer.pDevice->CreateInputLayout( pDescs, 3, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout );

		// Prepare light because OpenCollada couldn't handle well
		Light* lit = ControllerManager::GetInstance()->m_pRRC->GetLightByName( "TPhotometricLight01Mesh" );
		Node* node = ControllerManager::GetInstance()->m_pRRC->GetNodeByName( "TPhotometricLight01.Target" );
		lit->Target = node;

		// Add to keyboard controller
		ControllerManager::GetInstance()->m_pIC->AppendListener( this );

		return 0;
	};

	INT32 ImageSpaceSubsurfaceScattering::Finalize()
	{
		pEffect->Release();

		pSRV->Release();
		pTexScatteredSubsurfaceLight->Release();
		pTexLightSource->Release();
		pTexLightDiffuse->Release();
		pTexGBuffer->Release();

		pRTVScatteredSubsurfaceLight->Release();
		pRTVLightSource->Release();
		pRTVLightDiffuse->Release();
		pRTVGBuffer->Release();

		pSRVScatteredSubsurfaceLight->Release();
		pSRVLightSource->Release();
		pSRVLightDiffuse->Release();
		pSRVGBuffer->Release();

		// Depth stencil buffer
		pDSLightSource->Release();
		pDSVLightSource->Release();
		pDSScattered->Release();
		pDSVScattered->Release();

		pInputLayout->Release();

		return 0;
	};

	INT32 ImageSpaceSubsurfaceScattering::Render()
	{
		g_Renderer.pDevice->IASetInputLayout( pInputLayout );
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
		D3DXMATRIX worldviewprojection;

		const std::set< Element* >& staticmeshes = ControllerManager::GetInstance()->m_pRRC->GetSceneStaticGeometries();
		for( std::set< Element* >::const_iterator iElem = staticmeshes.begin(); iElem != staticmeshes.end(); iElem++ )
		{
			if( ( *iElem )->eType == Element::ET_StaticMesh )
			{
				StaticGeometry* geom = ( *iElem )->pStaticMesh;
				Mesh* mesh = geom->pGeometry->vMesh[ 0 ];
				g_Renderer.pDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

				// Clear render targets and depth stencils
				float ClearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
				g_Renderer.pDevice->ClearRenderTargetView( pRTVScatteredSubsurfaceLight, ClearColor);
				g_Renderer.pDevice->ClearRenderTargetView( pRTVLightSource, ClearColor);
				g_Renderer.pDevice->ClearRenderTargetView( pRTVLightDiffuse, ClearColor);
				g_Renderer.pDevice->ClearRenderTargetView( pRTVGBuffer, ClearColor);

				g_Renderer.pDevice->ClearDepthStencilView( pDSVLightSource, D3D10_CLEAR_DEPTH, 1.0f, 0 );
				g_Renderer.pDevice->ClearDepthStencilView( pDSVScattered, D3D10_CLEAR_DEPTH, 1.0f, 0 );

				// SetViewport
				D3D10_VIEWPORT viewport;
				viewport.Width = LightSourceWidth;
				viewport.Height = LightSourceHeight;
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;
				viewport.MaxDepth = 1.0f;
				viewport.MinDepth = 0.0f;
				g_Renderer.pDevice->RSSetViewports(1, &viewport );

				// Create light source
				mesh->SetBuffersToDevice();

				ID3D10RenderTargetView* pCLSViews[ 2 ] = { pRTVLightDiffuse, pRTVLightSource };
				g_Renderer.pDevice->OMSetRenderTargets( 2, pCLSViews, pDSVLightSource );

				world = ( *iElem )->pNode->GetWorldMatrix();
				ControllerManager::GetInstance()->m_pLC->GetViewMatrix( "TPhotometricLight01Mesh", view );
				ControllerManager::GetInstance()->m_pLC->GetProjectionMatrix( "TPhotometricLight01Mesh", projection );
				D3DXVECTOR3 lightpos;
				ControllerManager::GetInstance()->m_pLC->GetWorldPos( "TPhotometricLight01Mesh", lightpos );

				D3DXMatrixMultiply( &worldviewprojection, &world, &view );
				D3DXMatrixMultiply( &worldviewprojection, &worldviewprojection, &projection );
				pWorld->SetMatrix( ( float* )&world );
				pView->SetMatrix( ( float* )&view );
				pWorldViewProjection->SetMatrix( ( float* )&worldviewprojection );
				pLightPos->SetFloatVector( ( float* )&lightpos.x );
				pUseSSSS->SetBool( m_bUseSSSS );

				FLOAT32 diffuse[ 4 ] = { 0.0313726f, 0.117647f, 0.0f, 1.0f };
				pDiffuse->SetFloatVector( diffuse );

				pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 0 )->Apply( 0 );
				g_Renderer.pDevice->DrawIndexed( mesh->IndexCount, mesh->IndexStartLocation, 0 );
/*
				ID3D10RenderTargetView* pQuadViews[ 2 ] = { DXUTGetD3D10RenderTargetView(), NULL };
				g_Renderer.pDevice->OMSetRenderTargets( 2, pQuadViews, DXUTGetD3D10DepthStencilView() );

									pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 4 )->Apply( 0 );
									g_Renderer.pDevice->Draw( 6, 0 );
*/
				// Create GBuffer
				ID3D10RenderTargetView* pCGBViews[ 2 ] = { pRTVGBuffer, NULL };
				g_Renderer.pDevice->OMSetRenderTargets( 2, pCGBViews, pDSVScattered );	

				viewport.Width = ScatterWidth;
				viewport.Height = ScatterHeight;
				g_Renderer.pDevice->RSSetViewports( 1, &viewport );

				view = ControllerManager::GetInstance()->m_pCC->GetViewMatrix();
				projection = ControllerManager::GetInstance()->m_pCC->GetProjectionMatrix();
				D3DXMatrixMultiply( &worldviewprojection, &world, &view );
				D3DXMatrixMultiply( &worldviewprojection, &worldviewprojection, &projection );
				pWorld->SetMatrix( ( float* )&world );
				pView->SetMatrix( ( float* )&view );
				pProjection->SetMatrix( ( float* )&projection );
				pWorldViewProjection->SetMatrix( ( float* )&worldviewprojection );

				pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 1 )->Apply( 0 );
				g_Renderer.pDevice->DrawIndexed( mesh->IndexCount, mesh->IndexStartLocation, 0 );

				// Splatting
				ID3D10RenderTargetView* pSplatViews[ 1 ] = { pRTVScatteredSubsurfaceLight };
				g_Renderer.pDevice->OMSetRenderTargets( 1, pSplatViews, pDSVScattered );

				g_Renderer.pDevice->ClearDepthStencilView( pDSVScattered, D3D10_CLEAR_DEPTH, 1.0, 0 );

				pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 2 )->Apply( 0 );
				g_Renderer.pDevice->Draw( 48 * 48, 0 );

				// Composition
				ID3D10RenderTargetView* pCompViews[ 1 ] = { g_Renderer.pRTV };
				g_Renderer.pDevice->OMSetRenderTargets( 1, pCompViews, g_Renderer.pDSV );

				g_Renderer.pDevice->ClearDepthStencilView( g_Renderer.pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );

				viewport.Width = 640;
				viewport.Height = 480;
				g_Renderer.pDevice->RSSetViewports( 1, &viewport );

				pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 3 )->Apply( 0 );
				g_Renderer.pDevice->DrawIndexed( mesh->IndexCount, mesh->IndexStartLocation, 0 );
			};
		};

		return 0;
	};

	void ImageSpaceSubsurfaceScattering::OnKeyboard( OIS::Keyboard* kb )
	{
		if( kb->isKeyDown( OIS::KC_T ) )
		{
			static FLOAT64 freeze = 0;
			if( !ControllerManager::GetInstance()->m_pTC->GetFreezeState( freeze ) )
			{
				m_bUseSSSS = !m_bUseSSSS;
				freeze = ControllerManager::GetInstance()->m_pTC->StartFreeze();
			};
		};
	};
};