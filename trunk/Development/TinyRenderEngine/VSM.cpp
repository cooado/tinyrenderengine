#include "Prerequisites.h"

namespace TRE
{

	INT32 VSMRender::Initialize()
	{
		ShadowTechnique = 0;
		pEffect = NULL;
		HRESULT result = D3DX10CreateEffectFromFile( L"..\\..\\Media\\Shader\\VSM.fx", NULL, NULL, "fx_4_0", NULL, NULL, g_Renderer.pDevice, NULL, NULL, &pEffect, NULL, NULL );
		pCreateVSM = pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 0 );
		pSkeletalVSM = pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 1 );
		pStaticVSM = pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 2 );

		pShadowTechnique = pEffect->GetVariableByName( "ShadowMethod" )->AsScalar();
		pWorld = pEffect->GetVariableByName( "g_mWorld" )->AsMatrix();
		pView = pEffect->GetVariableByName( "g_mView" )->AsMatrix();
		pWorldViewProjection = pEffect->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();
		pLightView = pEffect->GetVariableByName( "g_mLightView" )->AsMatrix();
		pLightProjection = pEffect->GetVariableByName( "g_mLightProjection" )->AsMatrix();
		pLightWorldViewProjection = pEffect->GetVariableByName( "g_mLightWorldViewProjection" )->AsMatrix();
		pLightViewProjection = pEffect->GetVariableByName( "g_mLightViewProjection" )->AsMatrix();
		pDirLight = pEffect->GetVariableByName( "g_DirLight" )->AsVector();
		pLightDiffuse = pEffect->GetVariableByName( "g_LightDiffuse" )->AsVector();

		pEmission = pEffect->GetVariableByName( "Emission" )->AsVector();
		pAmbient = pEffect->GetVariableByName( "Ambient" )->AsVector();
		pDiffuse = pEffect->GetVariableByName( "Diffuse" )->AsVector();
		pSpecular = pEffect->GetVariableByName( "Specular" )->AsVector();
		pShininess = pEffect->GetVariableByName( "Shininess" )->AsScalar();
		pUseDiffuseTexture = pEffect->GetVariableByName( "UseDiffuseTexture" )->AsScalar();

		pDiffuseTex = pEffect->GetVariableByName( "g_DiffuseTex" )->AsShaderResource();
		pVSM = pEffect->GetVariableByName( "g_VSM" )->AsShaderResource();

		pJointIndexBuffer = pEffect->GetVariableByName( "JointIndex" )->AsShaderResource();
		pJointWeightBuffer = pEffect->GetVariableByName( "JointWeight" )->AsShaderResource();
		pJointBindBuffer = pEffect->GetVariableByName( "JointBind" )->AsShaderResource();

		// Static mesh input layout
		D3D10_INPUT_ELEMENT_DESC pDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 }
		};
		D3D10_PASS_DESC PassDesc;
		result = pEffect->GetTechniqueByIndex( 0 )->GetPassByName( "StaticUseVSM" )->GetDesc( &PassDesc );
		pInputLayout = NULL;
		result = g_Renderer.pDevice->CreateInputLayout( pDescs, 3, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout );

		// Skeletal mesh input layout
		D3D10_INPUT_ELEMENT_DESC pSMDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_UINT, 3, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		};
		result = pEffect->GetTechniqueByIndex( 0 )->GetPassByName( "SkeletalUseVSM" )->GetDesc( &PassDesc );
		pInputLayoutSkeletal = NULL;
		result = g_Renderer.pDevice->CreateInputLayout( pSMDescs, 4, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayoutSkeletal );
				
		// Create shadow map
		VSMWidth = VSMHeight = 512;
		D3D10_TEXTURE2D_DESC TexDesc;
		TexDesc.ArraySize = 1;
		TexDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		TexDesc.CPUAccessFlags = 0;
		TexDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		TexDesc.Height = VSMHeight;
		TexDesc.MipLevels = 1;
		TexDesc.MiscFlags = 0;
		TexDesc.SampleDesc.Count = 1;
		TexDesc.SampleDesc.Quality = 0;
		TexDesc.Usage = D3D10_USAGE_DEFAULT;
		TexDesc.Width = VSMWidth;
		result = g_Renderer.pDevice->CreateTexture2D( &TexDesc, NULL, &pVSMTex );
		TexDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		TexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		result = g_Renderer.pDevice->CreateTexture2D( &TexDesc, NULL, &pVSMDepthStencilTex );
		D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
		RTVDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;
		result = g_Renderer.pDevice->CreateRenderTargetView( pVSMTex, &RTVDesc, &pVSMRTV );
		D3D10_DEPTH_STENCIL_VIEW_DESC DSVDesc;
		DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		DSVDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		DSVDesc.Texture2D.MipSlice = 0;
		result = g_Renderer.pDevice->CreateDepthStencilView( pVSMDepthStencilTex, &DSVDesc, &pVSMDSV );
		D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		SRVDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		result = g_Renderer.pDevice->CreateShaderResourceView( pVSMTex, &SRVDesc, &pVSMSRV );

		pVSM->SetResource( pVSMSRV );

		// Prepare light because OpenCollada couldn't handle well
		Light* lit = ControllerManager::GetInstance()->m_pRRC->GetLightByName( "TPhotometricLight01-light" );
		Node* node = ControllerManager::GetInstance()->m_pRRC->GetNodeByName( "TPhotometricLight01.Target" );
		lit->Target = node;		
		
		// Add keyboard listener
		ControllerManager::GetInstance()->m_pIC->AppendListener( this );

		return 0;
	};

	INT32 VSMRender::Finalize()
	{
		pVSMTex->Release();
		pVSMDepthStencilTex->Release();
		pVSMRTV->Release();
		pVSMSRV->Release();
		pVSMDSV->Release();

		pEffect->Release();

		pInputLayout->Release();
		pInputLayoutSkeletal->Release();

		return 0;
	};

	void VSMRender::OnKeyboard( OIS::Keyboard* kb )
	{
		static FLOAT64 freeze = 0;

		if( kb->isKeyDown( OIS::KC_T ) )
		{
			if( !ControllerManager::GetInstance()->m_pTC->GetFreezeState( freeze ) )
			{
				ShadowTechnique = ( ++ShadowTechnique ) % 3;
			};
		};
	};

	INT32 VSMRender::Render()
	{
		HRESULT result;

		D3DXMATRIX world;
		D3DXMATRIX view = ControllerManager::GetInstance()->m_pCC->GetViewMatrix();
		D3DXMATRIX projection = ControllerManager::GetInstance()->m_pCC->GetProjectionMatrix();
		D3DXMATRIX worldviewprojection;
		D3DXMATRIX lightview;
		D3DXMATRIX lightprojection;
		D3DXMATRIX lightworldviewprojection;
		D3DXMATRIX lightviewprojection;
		ControllerManager::GetInstance()->m_pLC->GetViewMatrix( "TPhotometricLight01-light", lightview );
		D3DXMatrixOrthoLH( &lightprojection, 6, 6, 0.1f, 10.0f );
		D3D10_VIEWPORT Viewport;

		g_Renderer.pDevice->ClearDepthStencilView( pVSMDSV, D3D10_CLEAR_DEPTH, 1, 0 );
		FLOAT32 ClearColor[ 4 ] = { 1, 1, 1, 1 };
		g_Renderer.pDevice->ClearRenderTargetView( pVSMRTV, ClearColor );

		pShadowTechnique->SetInt( ShadowTechnique );

		// Create VSM
		g_Renderer.pDevice->OMSetRenderTargets( 1, &pVSMRTV, pVSMDSV );
		Viewport.Width = VSMWidth;
		Viewport.Height = VSMHeight;
		Viewport.TopLeftX = 0;
		Viewport.TopLeftY = 0;
		Viewport.MaxDepth = 1;
		Viewport.MinDepth = 0;
		g_Renderer.pDevice->RSSetViewports( 1, &Viewport );
		const std::set< Element* >& skeletalmeshes = ControllerManager::GetInstance()->m_pRRC->GetSceneSkeletalGeometries();
		for( std::set< Element* >::const_iterator iE = skeletalmeshes.begin(); iE != skeletalmeshes.end(); iE++ )
		{
			g_Renderer.pDevice->IASetInputLayout( pInputLayoutSkeletal );
			if( ( *iE )->eType == Element::ET_SkeletalMesh )
			{
				SkeletalGeometry* geom = ( *iE )->pSkeletalMesh;

				result = pJointIndexBuffer->SetResource( geom->SkinData->SkeletalBufferSRVs[ Skin::SBS_Joints ] );
				result = pJointWeightBuffer->SetResource( geom->SkinData->SkeletalBufferSRVs[ Skin::SBS_Weight ] );
				result = pJointBindBuffer->SetResource( geom->SkinData->SkeletalBufferSRVs[ Skin::SBS_Bind ] );

				world = ( *iE )->pNode->mAccumulatedTransform;
				pWorld->SetMatrix( ( float* )&world );
				pView->SetMatrix( ( float* )&view );
				D3DXMatrixMultiply( &worldviewprojection, &world, &view );
				D3DXMatrixMultiply( &worldviewprojection, &worldviewprojection, &projection );
				pWorldViewProjection->SetMatrix( ( float* )&worldviewprojection );
				D3DXMatrixMultiply( &lightworldviewprojection, &world, &lightview );
				D3DXMatrixMultiply( &lightworldviewprojection, &lightworldviewprojection, &lightprojection );
				pLightView->SetMatrix( ( float* )&lightview );
				pLightProjection->SetMatrix( ( float* )&lightprojection );
				pLightWorldViewProjection->SetMatrix( ( float* )&lightworldviewprojection );
				pLightViewProjection->SetMatrix( ( float* )&lightviewprojection );
				for( UINT32 iM = 0; iM < geom->Mesh->pGeometry->vMesh.size(); iM++ )
				{
					Mesh* mesh = geom->Mesh->pGeometry->vMesh[ iM ];
					g_Renderer.pDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

					ID3D10Buffer* pVB[ 4 ] = 
					{ 
						mesh->Buffers[ Mesh::BS_POSITION ], 
						mesh->Buffers[ Mesh::BS_NORMAL ], 
						mesh->Buffers[ Mesh::BS_TEXCOORD0 ],
						geom->SkinData->SkeletalBuffers[ Skin::SBS_VertexJoints ],
					};
					UINT pStrides[ 4 ] = 
					{
						mesh->BufferStrides[ Mesh::BS_POSITION ],
						mesh->BufferStrides[ Mesh::BS_NORMAL ],
						mesh->BufferStrides[ Mesh::BS_TEXCOORD0 ],
						geom->SkinData->SkeletalBufferStrides[ Skin::SBS_VertexJoints ],
					};
					UINT pOffsets[ 4 ] = { 0, 0, 0, 0 };
					g_Renderer.pDevice->IASetVertexBuffers( 0, 4, pVB, pStrides, pOffsets );
					g_Renderer.pDevice->IASetIndexBuffer( mesh->Buffers[ Mesh::BS_INDEX ], DXGI_FORMAT_R32_UINT, 0 );

					// Set material parameters
					FLOAT32 color[ 4 ] = { 0.0313726f, 0.117647f, 0.0f, 1.0f };
					color[ 0 ] = geom->Mesh->pMaterial->vParams[ iM ]->cEmission.r;
					color[ 1 ] = geom->Mesh->pMaterial->vParams[ iM ]->cEmission.g;
					color[ 2 ] = geom->Mesh->pMaterial->vParams[ iM ]->cEmission.b;
					color[ 3 ] = geom->Mesh->pMaterial->vParams[ iM ]->cEmission.a;
					pEmission->SetFloatVector( color );
					color[ 0 ] = geom->Mesh->pMaterial->vParams[ iM ]->cAmbient.r;
					color[ 1 ] = geom->Mesh->pMaterial->vParams[ iM ]->cAmbient.g;
					color[ 2 ] = geom->Mesh->pMaterial->vParams[ iM ]->cAmbient.b;
					color[ 3 ] = geom->Mesh->pMaterial->vParams[ iM ]->cAmbient.a;
					pAmbient->SetFloatVector( color );
					if( geom->Mesh->pMaterial->vParams[ iM ]->pDiffuse == NULL )
					{
						color[ 0 ] = geom->Mesh->pMaterial->vParams[ iM ]->cDiffuse.r;
						color[ 1 ] = geom->Mesh->pMaterial->vParams[ iM ]->cDiffuse.g;
						color[ 2 ] = geom->Mesh->pMaterial->vParams[ iM ]->cDiffuse.b;
						color[ 3 ] = geom->Mesh->pMaterial->vParams[ iM ]->cDiffuse.a;
						pDiffuse->SetFloatVector( color );
						pUseDiffuseTexture->SetBool( false );
					}
					else
					{
						result = pDiffuseTex->SetResource( geom->Mesh->pMaterial->vParams[ iM ]->pDiffuse );
						pUseDiffuseTexture->SetBool( true );
					};
					color[ 0 ] = geom->Mesh->pMaterial->vParams[ iM ]->cSpecular.r;
					color[ 1 ] = geom->Mesh->pMaterial->vParams[ iM ]->cSpecular.g;
					color[ 2 ] = geom->Mesh->pMaterial->vParams[ iM ]->cSpecular.b;
					color[ 3 ] = geom->Mesh->pMaterial->vParams[ iM ]->cSpecular.a;
					pSpecular->SetFloatVector( color );
					pShininess->SetFloat( geom->Mesh->pMaterial->vParams[ iM ]->fShininess );

					pCreateVSM->Apply( 0 );
					g_Renderer.pDevice->DrawIndexed( mesh->IndexCount, mesh->IndexStartLocation, 0 );
				};
			};
		};

		g_Renderer.pDevice->OMSetRenderTargets( 1, &g_Renderer.pRTV, g_Renderer.pDSV );
		Viewport.Width = 640;
		Viewport.Height = 480;
		g_Renderer.pDevice->RSSetViewports( 1, &Viewport );

		// Skeletal Mesh
		for( std::set< Element* >::const_iterator iE = skeletalmeshes.begin(); iE != skeletalmeshes.end(); iE++ )
		{
			g_Renderer.pDevice->IASetInputLayout( pInputLayoutSkeletal );
			if( ( *iE )->eType == Element::ET_SkeletalMesh )
			{
				SkeletalGeometry* geom = ( *iE )->pSkeletalMesh;

				result = pJointIndexBuffer->SetResource( geom->SkinData->SkeletalBufferSRVs[ Skin::SBS_Joints ] );
				result = pJointWeightBuffer->SetResource( geom->SkinData->SkeletalBufferSRVs[ Skin::SBS_Weight ] );
				result = pJointBindBuffer->SetResource( geom->SkinData->SkeletalBufferSRVs[ Skin::SBS_Bind ] );

				world = ( *iE )->pNode->mAccumulatedTransform;
				result = pWorld->SetMatrix( ( float* )&world );
				result = pView->SetMatrix( ( float* )&view );
				D3DXMatrixMultiply( &worldviewprojection, &world, &view );
				D3DXMatrixMultiply( &worldviewprojection, &worldviewprojection, &projection );
				result = pWorldViewProjection->SetMatrix( ( float* )&worldviewprojection );
				D3DXMatrixMultiply( &lightworldviewprojection, &world, &lightview );
				D3DXMatrixMultiply( &lightworldviewprojection, &lightworldviewprojection, &lightprojection );
				D3DXMatrixMultiply( &lightviewprojection, &lightview, &lightprojection );
				result = pLightView->SetMatrix( ( float* )&lightview );
				result = pLightProjection->SetMatrix( ( float* )&lightprojection );
				result = pLightWorldViewProjection->SetMatrix( ( float* )&lightworldviewprojection );
				result = pLightViewProjection->SetMatrix( ( float* )&lightviewprojection );
				for( UINT32 iM = 0; iM < geom->Mesh->pGeometry->vMesh.size(); iM++ )
				{
					Mesh* mesh = geom->Mesh->pGeometry->vMesh[ iM ];
					g_Renderer.pDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

					ID3D10Buffer* pVB[ 4 ] = 
					{ 
						mesh->Buffers[ Mesh::BS_POSITION ], 
						mesh->Buffers[ Mesh::BS_NORMAL ], 
						mesh->Buffers[ Mesh::BS_TEXCOORD0 ],
						geom->SkinData->SkeletalBuffers[ Skin::SBS_VertexJoints ],
					};
					UINT pStrides[ 4 ] = 
					{
						mesh->BufferStrides[ Mesh::BS_POSITION ],
						mesh->BufferStrides[ Mesh::BS_NORMAL ],
						mesh->BufferStrides[ Mesh::BS_TEXCOORD0 ],
						geom->SkinData->SkeletalBufferStrides[ Skin::SBS_VertexJoints ],
					};
					UINT pOffsets[ 4 ] = { 0, 0, 0, 0 };
					g_Renderer.pDevice->IASetVertexBuffers( 0, 4, pVB, pStrides, pOffsets );
					g_Renderer.pDevice->IASetIndexBuffer( mesh->Buffers[ Mesh::BS_INDEX ], DXGI_FORMAT_R32_UINT, 0 );

					// Set material parameters
					FLOAT32 color[ 4 ] = { 0.0313726f, 0.117647f, 0.0f, 1.0f };
					color[ 0 ] = geom->Mesh->pMaterial->vParams[ iM ]->cEmission.r;
					color[ 1 ] = geom->Mesh->pMaterial->vParams[ iM ]->cEmission.g;
					color[ 2 ] = geom->Mesh->pMaterial->vParams[ iM ]->cEmission.b;
					color[ 3 ] = geom->Mesh->pMaterial->vParams[ iM ]->cEmission.a;
					pEmission->SetFloatVector( color );
					color[ 0 ] = geom->Mesh->pMaterial->vParams[ iM ]->cAmbient.r;
					color[ 1 ] = geom->Mesh->pMaterial->vParams[ iM ]->cAmbient.g;
					color[ 2 ] = geom->Mesh->pMaterial->vParams[ iM ]->cAmbient.b;
					color[ 3 ] = geom->Mesh->pMaterial->vParams[ iM ]->cAmbient.a;
					pAmbient->SetFloatVector( color );
					if( geom->Mesh->pMaterial->vParams[ iM ]->pDiffuse == NULL )
					{
						color[ 0 ] = geom->Mesh->pMaterial->vParams[ iM ]->cDiffuse.r;
						color[ 1 ] = geom->Mesh->pMaterial->vParams[ iM ]->cDiffuse.g;
						color[ 2 ] = geom->Mesh->pMaterial->vParams[ iM ]->cDiffuse.b;
						color[ 3 ] = geom->Mesh->pMaterial->vParams[ iM ]->cDiffuse.a;
						pDiffuse->SetFloatVector( color );
						pUseDiffuseTexture->SetBool( false );
					}
					else
					{
						result = pDiffuseTex->SetResource( geom->Mesh->pMaterial->vParams[ iM ]->pDiffuse );
						pUseDiffuseTexture->SetBool( true );
					};
					color[ 0 ] = geom->Mesh->pMaterial->vParams[ iM ]->cSpecular.r;
					color[ 1 ] = geom->Mesh->pMaterial->vParams[ iM ]->cSpecular.g;
					color[ 2 ] = geom->Mesh->pMaterial->vParams[ iM ]->cSpecular.b;
					color[ 3 ] = geom->Mesh->pMaterial->vParams[ iM ]->cSpecular.a;
					pSpecular->SetFloatVector( color );
					pShininess->SetFloat( geom->Mesh->pMaterial->vParams[ iM ]->fShininess );

					pSkeletalVSM->Apply( 0 );
					g_Renderer.pDevice->DrawIndexed( mesh->IndexCount, mesh->IndexStartLocation, 0 );
				};
			};
		};

		// Static mesh
		const std::set< Element* >& staticmeshes = ControllerManager::GetInstance()->m_pRRC->GetSceneStaticGeometries();
		for( std::set< Element* >::const_iterator iE = staticmeshes.begin(); iE != staticmeshes.end(); iE++ )
		{
			g_Renderer.pDevice->IASetInputLayout( pInputLayout );
			if( ( *iE )->eType == Element::ET_StaticMesh )
			{
				StaticGeometry* geom = ( *iE )->pStaticMesh;
				for( UINT32 iM = 0; iM < geom->pGeometry->vMesh.size(); iM++ )
				{
					Mesh* mesh = geom->pGeometry->vMesh[ iM ];
					g_Renderer.pDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
					mesh->SetBuffersToDevice();
					world = ( *iE )->pNode->mAccumulatedTransform;
					pWorld->SetMatrix( ( float* )&world );
					pView->SetMatrix( ( float* )&view );
					D3DXMatrixMultiply( &worldviewprojection, &world, &view );
					D3DXMatrixMultiply( &worldviewprojection, &worldviewprojection, &projection );
					pWorldViewProjection->SetMatrix( ( float* )&worldviewprojection );
					D3DXMatrixMultiply( &lightworldviewprojection, &world, &lightview );
					D3DXMatrixMultiply( &lightworldviewprojection, &lightworldviewprojection, &lightprojection );
					pLightView->SetMatrix( ( float* )&lightview );
					pLightProjection->SetMatrix( ( float* )&lightprojection );
					pLightWorldViewProjection->SetMatrix( ( float* )&lightworldviewprojection );
					pLightViewProjection->SetMatrix( ( float* )&lightviewprojection );

					FLOAT32 color[ 4 ] = { 0.0313726f, 0.117647f, 0.0f, 1.0f };
					color[ 0 ] = geom->pMaterial->vParams[ iM ]->cEmission.r;
					color[ 1 ] = geom->pMaterial->vParams[ iM ]->cEmission.g;
					color[ 2 ] = geom->pMaterial->vParams[ iM ]->cEmission.b;
					color[ 3 ] = geom->pMaterial->vParams[ iM ]->cEmission.a;
					pEmission->SetFloatVector( color );
					color[ 0 ] = geom->pMaterial->vParams[ iM ]->cAmbient.r;
					color[ 1 ] = geom->pMaterial->vParams[ iM ]->cAmbient.g;
					color[ 2 ] = geom->pMaterial->vParams[ iM ]->cAmbient.b;
					color[ 3 ] = geom->pMaterial->vParams[ iM ]->cAmbient.a;
					pAmbient->SetFloatVector( color );
					if( geom->pMaterial->vParams[ iM ]->pDiffuse == NULL )
					{
						color[ 0 ] = geom->pMaterial->vParams[ iM ]->cDiffuse.r;
						color[ 1 ] = geom->pMaterial->vParams[ iM ]->cDiffuse.g;
						color[ 2 ] = geom->pMaterial->vParams[ iM ]->cDiffuse.b;
						color[ 3 ] = geom->pMaterial->vParams[ iM ]->cDiffuse.a;
						pDiffuse->SetFloatVector( color );
						pUseDiffuseTexture->SetBool( false );
					}
					else
					{
						result = pDiffuseTex->SetResource( geom->pMaterial->vParams[ iM ]->pDiffuse );
						pUseDiffuseTexture->SetBool( true );
					};
					color[ 0 ] = geom->pMaterial->vParams[ iM ]->cSpecular.r;
					color[ 1 ] = geom->pMaterial->vParams[ iM ]->cSpecular.g;
					color[ 2 ] = geom->pMaterial->vParams[ iM ]->cSpecular.b;
					color[ 3 ] = geom->pMaterial->vParams[ iM ]->cSpecular.a;
					pSpecular->SetFloatVector( color );
					pShininess->SetFloat( geom->pMaterial->vParams[ iM ]->fShininess );

					pStaticVSM->Apply( 0 );
					g_Renderer.pDevice->DrawIndexed( mesh->IndexCount, mesh->IndexStartLocation, 0 );
				};
			};
		};

		// Draw info
		g_Renderer.pTextHelper->Begin();
		g_Renderer.pTextHelper->SetInsertionPos( 5, 50 );
		g_Renderer.pTextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		if( ShadowTechnique == 0 )
		{
			g_Renderer.pTextHelper->DrawTextLine( L"Shadow Technique: VSM" );
		}
		else if( ShadowTechnique == 1 )
		{
			g_Renderer.pTextHelper->DrawTextLine( L"Shadow Technique: ESM" );
		}
		else if( ShadowTechnique == 2 )
		{
			g_Renderer.pTextHelper->DrawTextLine( L"Shadow Technique: Standard Shadow Map" );
		};
		g_Renderer.pTextHelper->End();


		return 0;
	};
};