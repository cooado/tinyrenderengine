#include "Prerequisites.h"

namespace TRE
{
	INT32 ForwardRenderPipeline::Initialize()
	{
		pEffect = NULL;
		HRESULT result = D3DX10CreateEffectFromFile( L"..\\..\\Media\\Shader\\ColladaCommonEffect.fx", NULL, NULL, "fx_4_0", NULL, NULL, g_Renderer.pDevice, NULL, NULL, &pEffect, NULL, NULL );

		pWorld = pEffect->GetVariableByName( "g_mWorld" )->AsMatrix();
		pView = pEffect->GetVariableByName( "g_mView" )->AsMatrix();
		pWorldViewProjection = pEffect->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();
		pDirLight = pEffect->GetVariableByName( "g_DirLight" )->AsVector();
		pLightDiffuse = pEffect->GetVariableByName( "g_LightDiffuse" )->AsVector();

		pEmission = pEffect->GetVariableByName( "Emission" )->AsVector();
		pAmbient = pEffect->GetVariableByName( "Ambient" )->AsVector();
		pDiffuse = pEffect->GetVariableByName( "Diffuse" )->AsVector();
		pSpecular = pEffect->GetVariableByName( "Specular" )->AsVector();
		pShininess = pEffect->GetVariableByName( "Shininess" )->AsScalar();
		pUseDiffuseTexture = pEffect->GetVariableByName( "UseDiffuseTexture" )->AsScalar();

		pDiffuseTex = pEffect->GetVariableByName( "g_DiffuseTex" )->AsShaderResource();

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
		result = pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 0 )->GetDesc( &PassDesc );
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
		result = pEffect->GetTechniqueByName( "RenderSceneSkeletal" )->GetPassByIndex( 0 )->GetDesc( &PassDesc );
		pInputLayoutSkeletal = NULL;
		result = g_Renderer.pDevice->CreateInputLayout( pSMDescs, 4, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayoutSkeletal );
		
		pSkeletalEffect = pEffect->GetTechniqueByName( "RenderSceneSkeletal" );
		
		return 0;
	};

	INT32 ForwardRenderPipeline::Finalize()
	{
		UINT32 rc;

		rc = pEffect->Release();
		
		rc = pInputLayout->Release();
		rc = pInputLayoutSkeletal->Release();

		return 0;
	};

	INT32 ForwardRenderPipeline::Render()
	{
		g_Renderer.pDevice->ClearRenderTargetView( g_Renderer.pRTV, D3DXVECTOR4(0.187f, 0.67f, 0.98f, 1) );
		g_Renderer.pDevice->ClearDepthStencilView( g_Renderer.pDSV, D3D10_CLEAR_DEPTH, 1.0f, 0 );

		HRESULT result;

		D3DXMATRIX world;
		D3DXMATRIX view = ControllerManager::GetInstance()->m_pCC->GetViewMatrix();
		D3DXMATRIX projection = ControllerManager::GetInstance()->m_pCC->GetProjectionMatrix();
		D3DXMATRIX worldviewprojection;

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
					ID3D10Buffer* pVB[ 3 ] = 
					{ 
						mesh->Buffers[ Mesh::BS_POSITION ], 
						mesh->Buffers[ Mesh::BS_NORMAL ], 
						mesh->Buffers[ Mesh::BS_TEXCOORD0 ] 
					};
					UINT pStrides[ 3 ] = 
					{
						mesh->BufferStrides[ Mesh::BS_POSITION ],
						mesh->BufferStrides[ Mesh::BS_NORMAL ],
						mesh->BufferStrides[ Mesh::BS_TEXCOORD0 ]
					};
					UINT pOffsets[ 3 ] = { 0, 0, 0 };
					g_Renderer.pDevice->IASetVertexBuffers( 0, 3, pVB, pStrides, pOffsets );
					g_Renderer.pDevice->IASetIndexBuffer( mesh->Buffers[ Mesh::BS_INDEX ], DXGI_FORMAT_R32_UINT, 0 );
					world = ( *iE )->pNode->mAccumulatedTransform;
					pWorld->SetMatrix( ( float* )&world );
					pView->SetMatrix( ( float* )&view );
					D3DXMatrixMultiply( &worldviewprojection, &world, &view );
					D3DXMatrixMultiply( &worldviewprojection, &worldviewprojection, &projection );
					pWorldViewProjection->SetMatrix( ( float* )&worldviewprojection );

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

					pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 0 )->Apply( 0 );
					g_Renderer.pDevice->DrawIndexed( mesh->IndexCount, mesh->IndexStartLocation, 0 );
				};
			};
		};

		// Skeletal mesh
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

					pSkeletalEffect->GetPassByIndex( 0 )->Apply( 0 );
					g_Renderer.pDevice->DrawIndexed( mesh->IndexCount, mesh->IndexStartLocation, 0 );
				};
			};
		};

		return 0;
	};

	/***************************************************************************************
	* MouseRender
	***************************************************************************************/

	INT32 MouseRender::Initialize()
	{
		pEffect = NULL;
		HRESULT result = D3DX10CreateEffectFromFile( L"..\\..\\Media\\Shader\\MouseShader.fx", NULL, NULL, "fx_4_0", NULL, NULL, g_Renderer.pDevice, NULL, NULL, &pEffect, NULL, NULL );
		pMouseTexture = pEffect->GetVariableByName( "Mouse" )->AsShaderResource();
		pMouseParam = pEffect->GetVariableByName( "MouseParam" )->AsVector();
		D3DX10CreateShaderResourceViewFromFileA( g_Renderer.pDevice, "D:\\Game_Development\\VS_Workspace\\TinyRenderEngine_v2.0\\Media\\Shader\\MouseCursor.png", NULL, NULL, &pSRVMouse, &result );
		pMouseTexture->SetResource( pSRVMouse );
		pPass = pEffect->GetTechniqueByIndex( 0 )->GetPassByIndex( 0 );

		ControllerManager::GetInstance()->m_pIC->AppendListener( this );

		return 0;
	};

	INT32 MouseRender::Finalize()
	{
		pEffect->Release();
		pSRVMouse->Release();

		return 0;
	};

	INT32 MouseRender::Render()
	{
		pPass->Apply( 0 );
		FLOAT32 param[ 4 ] = { fMousePosX, fMousePosY, fMouseWidth, fMouseHeight };
		pMouseParam->SetFloatVector( param );
		g_Renderer.pDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
		g_Renderer.pDevice->Draw( 4, 0 );
		g_Renderer.pDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		return 0;
	};

	void MouseRender::OnMouse( const OIS::Mouse* m )
	{
		const OIS::MouseState& state = m->getMouseState();
		INT32 width, height;
		TinyRenderEngine::GetInstance()->GetScreenResolution( width, height );
		fMousePosX = ( FLOAT32 )state.X.abs / ( FLOAT32 )width * 2.0f - 1.0f;
		fMousePosY = 1.0f - ( FLOAT32 )state.Y.abs / ( FLOAT32 )height * 2.0f;
		fMouseWidth = 32.0f / ( FLOAT32 )width * 2.0f;
		fMouseHeight = 32.0f / ( FLOAT32 )height * 2.0f;
	};
};