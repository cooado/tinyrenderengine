#include "Prerequisites.h"

namespace TRE
{
	/** 
	* Convert domFloat( 64-bit floating point ) to FLOAT32
	*/
#	define CF2F32( f ) static_cast< FLOAT32 >( f )

	/**
	* Convert domUint( 64-bit unsigned integer ) to UINT32
	*/
#	define CUI2UI32( i ) static_cast< UINT32 >( i )

	INT32 Importer_Dom::ImportFromDaeFile( std::string f )
	{
		DAE dae;
		domCOLLADA* root = dae.open( f );
		if (!root) {
			return -1;
		};

		// Light
		ImportLight( root );

		// Camera
		ImportCamera( root );

		// Geometry
		ImportGeometry( root );

		// Skin controller
		ImportSkinController( root );

		// Material
		ImportMaterial( root );

		// Node
		ImportNode( root );

		// Animation
		ImportAnimation( root );

		m_pRFC->EndAddNewResource();

		return 0;
	};

	INT32 Importer_Dom::ImportLight(domCOLLADA *root)
	{
		domLibrary_lights_Array &arrLibrary = root->getLibrary_lights_array();
		for( UINT32 iL = 0; iL < arrLibrary.getCount(); iL++ )
		{
			domLight_Array & arrLights = arrLibrary[ iL ]->getLight_array();
			for( UINT32 iLight = 0; iLight < arrLights.getCount(); iLight++ )
			{
				D3DXCOLOR lightcolor;
				lightcolor.r = 1;
				lightcolor.g = 1;
				lightcolor.b = 1;
				lightcolor.a = 1;
				domLightRef light = arrLights[ iLight ];
				if( light->getTechnique_common()->getAmbient().cast() != NULL )
				{
					domFloat3 &color = light->getTechnique_common()->getAmbient()->getColor()->getValue();
					lightcolor.r = CF2F32( color.get( 0 ) );
					lightcolor.g = CF2F32( color.get( 1 ) );
					lightcolor.b = CF2F32( color.get( 2 ) );
					lightcolor.a = 1.0f;
				};
				Light* l = m_pRFC->CreateNewLight();
				mapLights.insert( std::pair< domLight*, Light* >( light.cast(), l ) );
				l->eType = Light::LT_POINT;
				if( light->getName() != NULL )
				{
					l->sName = std::string( light->getName() );
				};
				l->cColor = lightcolor;
				l->Target = NULL;
			};
		};
		return 0;
	};

	INT32 Importer_Dom::ImportCamera(domCOLLADA *root)
	{
		domLibrary_cameras_Array &arrLibrary = root->getLibrary_cameras_array();
		for( UINT32 iL = 0; iL < arrLibrary.getCount(); iL++ )
		{
			domCamera_Array &arrCamera = arrLibrary[ iL ]->getCamera_array();
			for( UINT32 iC = 0; iC < arrCamera.getCount(); iC++ )
			{
				const domCamera::domOptics::domTechnique_common::domPerspectiveRef perspective = arrCamera[ iC ]->getOptics()->getTechnique_common()->getPerspective();
				Camera* cam = m_pRFC->CreateNewCamera();
				mapCameras.insert( std::pair< domCamera*, Camera* >( arrCamera[ iC ].cast(), cam ) );
				cam->eType = Camera::CT_PERSPECTIVE;
				cam->fNearClip = CF2F32( perspective->getZnear()->getValue() );
				cam->fFarClip = CF2F32( perspective->getZfar()->getValue() );
				cam->fFOVY = (FLOAT32)D3DXToRadian( 60 );
				cam->fAspect = 1.0f;
			};
		};
		return 0;
	};

	INT32 Importer_Dom::ImportGeometry(domCOLLADA *root)
	{
		HRESULT result;
		domLibrary_geometries_Array &arrLibrary = root->getLibrary_geometries_array();
		for( UINT32 iL = 0; iL < arrLibrary.getCount(); iL++ )
		{
			domGeometry_Array &arrGeom = arrLibrary[ iL ]->getGeometry_array();
			for( UINT32 iG = 0; iG < arrGeom.getCount(); iG++ )
			{
				Geometry* geometry = m_pRFC->CreateNewGeometry();
				geometry->Name = std::string( arrGeom[ iG ]->getName() );
				mapGeometries.insert( std::pair< domGeometry*, Geometry* >( arrGeom[ iG ].cast(), geometry ) );

				// Sources
				std::map< domSource*, ID3D10Buffer* > mapSource2Buffer;
				std::map< domSource*, FLOAT32* > mapSource2RawBuffers;
				std::set< domSource* > setUsedBuffers;
				std::set< domSource* > setAllBuffers;
				domSource_Array &arrSources = arrGeom[ iG ]->getMesh()->getSource_array();
				for( UINT32 iS = 0; iS < arrSources.getCount(); iS++ )
				{
					domSourceRef source = arrSources[ iS ];
					domUint sstride = source->getTechnique_common()->getAccessor()->getStride();
					domListOfFloats &sdata = source->getFloat_array()->getValue();
					FLOAT32* buffer = new FLOAT32[ sdata.getCount() ];
					for( UINT iF = 0; iF < sdata.getCount(); iF++ )
					{
						buffer[ iF ] = CF2F32( sdata.get( iF ) );
					};
					D3D10_BUFFER_DESC BufferDesc;
					BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
					BufferDesc.ByteWidth = sdata.getCount() * sizeof( FLOAT32 );
					BufferDesc.CPUAccessFlags = 0;
					BufferDesc.MiscFlags = 0;
					BufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
					D3D10_SUBRESOURCE_DATA InitData;
					InitData.pSysMem = ( void* )buffer;
					InitData.SysMemPitch = 0;
					InitData.SysMemSlicePitch = 0;
					ID3D10Buffer* pBuffer = NULL;
					result = g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
					mapSource2Buffer.insert( std::pair< domSource*, ID3D10Buffer* >( source.cast(), pBuffer ) );
					mapSource2RawBuffers.insert( std::pair< domSource*, FLOAT32* >( source.cast(), buffer ) );
					setAllBuffers.insert( source.cast() );
					//delete buffer;
				}

				// Triangles
				domTriangles_Array &arrTriangle = arrGeom[ iG ]->getMesh()->getTriangles_array();
				for( UINT32 iT = 0; iT < arrTriangle.getCount(); iT++ )
				{
					domTrianglesRef tri = arrTriangle[ iT ];
					Mesh* mesh = m_pRFC->CreateNewMesh();
					geometry->vMesh.push_back( mesh );
					for( UINT32 ib = 0; ib < Mesh::BS_END; ib++ )
					{
						mesh->Buffers[ ib ] = NULL;
					};
					mesh->MaterialSymbol = std::string( arrTriangle[ iT ]->getMaterial() );

					// Index buffer
					domListOfUInts &tIndex = tri->getP()->getValue();
					UINT32* pIndex = new UINT32[ tIndex.getCount() ];
					for( UINT32 iI = 0; iI < tIndex.getCount(); iI++ )
					{
						pIndex[ iI ] = CUI2UI32( tIndex.get( iI ) );
					};
					D3D10_BUFFER_DESC BufferDesc;
					BufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
					BufferDesc.ByteWidth = tIndex.getCount() * sizeof( UINT32 );
					BufferDesc.CPUAccessFlags = 0;
					BufferDesc.MiscFlags = 0;
					BufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
					D3D10_SUBRESOURCE_DATA InitData;
					InitData.pSysMem = ( void* )pIndex;
					InitData.SysMemPitch = 0;
					InitData.SysMemSlicePitch = 0;
					ID3D10Buffer* pBuffer = NULL;
					result = g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
					mesh->Buffers[ Mesh::BS_INDEX ] = pBuffer;
					mesh->BufferStrides[ Mesh::BS_INDEX ] = sizeof( UINT32 );
					mesh->IndexStartLocation = 0;
					mesh->IndexCount = tIndex.getCount();
					mesh->RawIndex = pIndex;
					//delete pIndex;

					// Vertex buffer
					domInputLocalOffset_Array &arrInputs = tri->getInput_array();
					for( UINT32 iInput = 0; iInput < arrInputs.getCount(); iInput++ )
					{
						if( std::string( arrInputs[ iInput ]->getSemantic() ) == std::string( "NORMAL" ) )
						{
							domSource* source = static_cast< domSource* >( arrInputs[ iInput ]->getSource().getElement().cast() );
							mesh->Buffers[ Mesh::BS_NORMAL ] = mapSource2Buffer.find( source )->second;
							mesh->RawBuffers[ Mesh::BS_NORMAL ] = mapSource2RawBuffers.find( source )->second;
							setUsedBuffers.insert( source );
							mesh->BufferStrides[ Mesh::BS_NORMAL ] = sizeof( FLOAT32 ) * CUI2UI32( source->getTechnique_common()->getAccessor()->getStride() );
						}
						else if( std::string( arrInputs[ iInput ]->getSemantic() ) == std::string( "TEXCOORD" ) )
						{
							domSource* source = static_cast< domSource* >( arrInputs[ iInput ]->getSource().getElement().cast() );
							mesh->Buffers[ Mesh::BS_TEXCOORD0 ] = mapSource2Buffer.find( source )->second;
							mesh->RawBuffers[ Mesh::BS_TEXCOORD0 ] = mapSource2RawBuffers.find( source )->second;
							setUsedBuffers.insert( source );
							mesh->BufferStrides[ Mesh::BS_TEXCOORD0 ] = sizeof( FLOAT32 ) * CUI2UI32( source->getTechnique_common()->getAccessor()->getStride() );
						}
						else if( std::string( arrInputs[ iInput ]->getSemantic() ) == std::string( "VERTEX" ) )
						{
							domVertices* vertices = static_cast< domVertices* >( arrInputs[ iInput ]->getSource().getElement().cast() );
							domInputLocal_Array &arrVInputs = vertices->getInput_array();
							for( UINT32 iVI = 0; iVI < arrVInputs.getCount(); iVI++ )
							{
								if( std::string( arrVInputs[ iVI ]->getSemantic() ) == std::string( "POSITION" ) )
								{
									domSource* source = static_cast< domSource* >( arrVInputs[ iVI ]->getSource().getElement().cast() );
									mesh->Buffers[ Mesh::BS_POSITION ] = mapSource2Buffer.find( source )->second;
									mesh->RawBuffers[ Mesh::BS_POSITION ] = mapSource2RawBuffers.find( source )->second;
									mesh->VertexBufferCount = CUI2UI32( source->getTechnique_common()->getAccessor()->getCount() );
									setUsedBuffers.insert( source );
									mesh->BufferStrides[ Mesh::BS_POSITION ] = sizeof( FLOAT32 ) * CUI2UI32( source->getTechnique_common()->getAccessor()->getStride() );
								}
								else if( std::string( arrVInputs[ iVI ]->getSemantic() ) == std::string( "NORMAL" ) )
								{
									domSource* source = static_cast< domSource* >( arrVInputs[ iVI ]->getSource().getElement().cast() );
									mesh->Buffers[ Mesh::BS_NORMAL ] = mapSource2Buffer.find( source )->second;
									mesh->RawBuffers[ Mesh::BS_NORMAL ] = mapSource2RawBuffers.find( source )->second;
									setUsedBuffers.insert( source );
									mesh->BufferStrides[ Mesh::BS_NORMAL ] = sizeof( FLOAT32 ) * CUI2UI32( source->getTechnique_common()->getAccessor()->getStride() );
								};
							};
						};
					};
				};
				// Clear unused buffers
				for( std::set< domSource* >::iterator ib = setAllBuffers.begin(); ib != setAllBuffers.end(); ib++ )
				{
					if( setUsedBuffers.find( *ib ) == setUsedBuffers.end() )
					{
						mapSource2Buffer.find( *ib )->second->Release();
						delete mapSource2RawBuffers.find( *ib )->second;
					};
				};
			};
		};

		return 0;
	};

	INT32 Importer_Dom::ImportSkinController(domCOLLADA *root)
	{
		HRESULT result;
		domLibrary_controllers_Array &arrLibrary = root->getLibrary_controllers_array();
		for( UINT32 iL = 0; iL < arrLibrary.getCount(); iL++ )
		{
			domController_Array &arrControllers = arrLibrary[ iL ]->getController_array();
			for( UINT32 iC = 0; iC < arrControllers.getCount(); iC++ )
			{
				const domSkinRef skin = arrControllers[ iC ]->getSkin();
				if( skin.cast() != NULL )
				{
					Skin* sd = m_pRFC->CreateNewSkin();
					mapSkinData.insert( std::pair< domController*, Skin* >( arrControllers[ iC ].cast(), sd ) );
					for( UINT32 ib = 0; ib < Skin::SBS_End; ib++ )
					{
						sd->SkeletalBuffers[ ib ] = NULL;
						sd->SkeletalBufferSRVs[ ib ] = NULL;
					};

					// Bind shape matrix
					domFloat4x4& bsm = skin->getBind_shape_matrix()->getValue();
					CopyMatrixToDirectX( sd->Bind_Shape_Matrix, bsm, 0 );

					// Inverse bind pose
					domInputLocal_Array &arrInputs = skin->getJoints()->getInput_array();
					for( UINT iInput = 0; iInput < arrInputs.getCount(); iInput++ )
					{
						if( std::string( arrInputs[ iInput ]->getSemantic() ) == std::string( "INV_BIND_MATRIX" ) )
						{
							domSource* source = static_cast< domSource* >( arrInputs[ iInput ]->getSource().getElement().cast() );
							domListOfFloats &IBMs = source->getFloat_array()->getValue();
							domUint IBMCount = source->getTechnique_common()->getAccessor()->getCount();
							for( UINT32 iIBM = 0; iIBM < IBMCount; iIBM++ )
							{
								D3DXMATRIX m;
								CopyMatrixToDirectX( m, IBMs, 16 * iIBM );
								sd->Inv_Bind_Matrix.push_back( m );
							};
						};
					};

					// Weights
					domInputLocalOffset_Array &arrVWInputs = skin->getVertex_weights()->getInput_array();
					for( UINT32 iInput = 0; iInput < arrVWInputs.getCount(); iInput++ )
					{
						if( std::string( arrVWInputs[ iInput ]->getSemantic() ) == std::string( "WEIGHT" ) )
						{
							domSource* source = static_cast< domSource* >( arrVWInputs[ iInput ]->getSource().getElement().cast() );
							domListOfFloats &weights = source->getFloat_array()->getValue();
							FLOAT32* fWeights = new FLOAT32[ weights.getCount() ];
							for( UINT32 iW = 0; iW < weights.getCount(); iW++ )
							{
								fWeights[ iW ] = CF2F32( weights.get( iW ) );
							};
							D3D10_BUFFER_DESC BufferDesc;
							BufferDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
							BufferDesc.ByteWidth = weights.getCount() * sizeof( FLOAT32 );
							BufferDesc.CPUAccessFlags = 0;
							BufferDesc.MiscFlags = 0;
							BufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
							D3D10_SUBRESOURCE_DATA InitData;
							InitData.pSysMem = ( void* )fWeights;
							InitData.SysMemPitch = 0;
							InitData.SysMemSlicePitch = 0;
							ID3D10Buffer* pBuffer;
							result = g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
							sd->SkeletalBuffers[ Skin::SBS_Weight ] = pBuffer;
							D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
							SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
							SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_BUFFER;
							SRVDesc.Buffer.ElementOffset = 0;
							SRVDesc.Buffer.ElementWidth = weights.getCount();
							ID3D10ShaderResourceView* pSRV = NULL;
							result = g_Renderer.pDevice->CreateShaderResourceView( pBuffer, &SRVDesc, &pSRV );
							sd->SkeletalBufferSRVs[ Skin::SBS_Weight ] = pSRV;
							delete fWeights;
						};
					};

					// Joint and weight index
					domListOfUInts &arrNumOfJointsPerVertex = skin->getVertex_weights()->getVcount()->getValue();
					domListOfInts &arrJointWeightBindPerVertex = skin->getVertex_weights()->getV()->getValue();
					domListOfUInts arrNumOfJointsOffset;
					UINT32 VertexCount = CUI2UI32( skin->getVertex_weights()->getCount() );
					arrNumOfJointsOffset.append( 0 );
					for( UINT32 iV = 1; iV < VertexCount; iV++ )
					{
						arrNumOfJointsOffset.append( arrNumOfJointsOffset.get( iV - 1 ) + arrNumOfJointsPerVertex.get( iV - 1 ) );
					};
					UINT32* pJoints = new UINT32[ arrJointWeightBindPerVertex.getCount() ];
					for( UINT32 iJ = 0; iJ < arrJointWeightBindPerVertex.getCount() / 2; iJ++ )
					{
						pJoints[ 2 * iJ ] = CUI2UI32( arrJointWeightBindPerVertex.get( 2 * iJ ) + 1 );
						pJoints[ 2 * iJ + 1 ] = CUI2UI32( arrJointWeightBindPerVertex.get( 2 * iJ + 1 ) );
					};
					D3D10_BUFFER_DESC BufferDesc;
					BufferDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
					BufferDesc.ByteWidth = arrJointWeightBindPerVertex.getCount() * sizeof( UINT32 );
					BufferDesc.CPUAccessFlags = 0;
					BufferDesc.MiscFlags = 0;
					BufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
					D3D10_SUBRESOURCE_DATA InitData;
					InitData.pSysMem = ( void* )pJoints;
					InitData.SysMemPitch = 0;
					InitData.SysMemSlicePitch = 0;
					ID3D10Buffer* pBuffer = NULL;
					result = g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
					sd->SkeletalBuffers[ Skin::SBS_Joints ] = pBuffer;
					D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
					SRVDesc.Format = DXGI_FORMAT_R32G32_UINT;
					SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_BUFFER;
					SRVDesc.Buffer.ElementOffset = 0;
					SRVDesc.Buffer.ElementWidth = arrJointWeightBindPerVertex.getCount() / 2;
					ID3D10ShaderResourceView* pSRV = NULL;
					result = g_Renderer.pDevice->CreateShaderResourceView( pBuffer, &SRVDesc, &pSRV );
					sd->SkeletalBufferSRVs[ Skin::SBS_Joints ] = pSRV;
					delete pJoints;

					// Joints per vertex
					UINT32* pVertexJoints = new UINT32[ VertexCount * 2 ];
					for( UINT32 iV = 0; iV < VertexCount; iV++ )
					{
						pVertexJoints[ 2 * iV ] = CUI2UI32( arrNumOfJointsPerVertex.get( iV ) );
						pVertexJoints[ 2 * iV + 1 ] = CUI2UI32( arrNumOfJointsOffset.get( iV ) );
					};
					BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
					BufferDesc.ByteWidth = VertexCount * 2 * sizeof( UINT32 );
					BufferDesc.CPUAccessFlags = 0;
					BufferDesc.MiscFlags = 0;
					BufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
					InitData.pSysMem = ( void* )pVertexJoints;
					result = g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
					sd->SkeletalBuffers[ Skin::SBS_VertexJoints ] = pBuffer;
					sd->SkeletalBufferStrides[ Skin::SBS_VertexJoints ] = 2 * sizeof( UINT32 );
					delete pVertexJoints;

					// Bind buffer
					FLOAT32* pBind = new FLOAT32[ ( sd->Inv_Bind_Matrix.size() + 1 ) * 16 ];
					pBind[ 0 ] = sd->Bind_Shape_Matrix._11;
					pBind[ 1 ] = sd->Bind_Shape_Matrix._12;
					pBind[ 2 ] = sd->Bind_Shape_Matrix._13;
					pBind[ 3 ] = sd->Bind_Shape_Matrix._14;
					
					pBind[ 4 ] = sd->Bind_Shape_Matrix._21;
					pBind[ 5 ] = sd->Bind_Shape_Matrix._22;
					pBind[ 6 ] = sd->Bind_Shape_Matrix._23;
					pBind[ 7 ] = sd->Bind_Shape_Matrix._24;
					
					pBind[ 8 ] = sd->Bind_Shape_Matrix._31;
					pBind[ 9 ] = sd->Bind_Shape_Matrix._32;
					pBind[ 10 ] = sd->Bind_Shape_Matrix._33;
					pBind[ 11 ] = sd->Bind_Shape_Matrix._34;
					
					pBind[ 12 ] = sd->Bind_Shape_Matrix._41;
					pBind[ 13 ] = sd->Bind_Shape_Matrix._42;
					pBind[ 14 ] = sd->Bind_Shape_Matrix._43;
					pBind[ 15 ] = sd->Bind_Shape_Matrix._44;
					for( UINT32 iB = 0; iB < sd->Inv_Bind_Matrix.size(); iB++ )
					{
						pBind[ ( iB + 1 ) * 16 + 0 ] = sd->Inv_Bind_Matrix[ iB ]._11;
						pBind[ ( iB + 1 ) * 16 + 1 ] = sd->Inv_Bind_Matrix[ iB ]._12;
						pBind[ ( iB + 1 ) * 16 + 2 ] = sd->Inv_Bind_Matrix[ iB ]._13;
						pBind[ ( iB + 1 ) * 16 + 3 ] = sd->Inv_Bind_Matrix[ iB ]._14;

						pBind[ ( iB + 1 ) * 16 + 4 ] = sd->Inv_Bind_Matrix[ iB ]._21;
						pBind[ ( iB + 1 ) * 16 + 5 ] = sd->Inv_Bind_Matrix[ iB ]._22;
						pBind[ ( iB + 1 ) * 16 + 6 ] = sd->Inv_Bind_Matrix[ iB ]._23;
						pBind[ ( iB + 1 ) * 16 + 7 ] = sd->Inv_Bind_Matrix[ iB ]._24;

						pBind[ ( iB + 1 ) * 16 + 8 ] = sd->Inv_Bind_Matrix[ iB ]._31;
						pBind[ ( iB + 1 ) * 16 + 9 ] = sd->Inv_Bind_Matrix[ iB ]._32;
						pBind[ ( iB + 1 ) * 16 + 10 ] = sd->Inv_Bind_Matrix[ iB ]._33;
						pBind[ ( iB + 1 ) * 16 + 11 ] = sd->Inv_Bind_Matrix[ iB ]._34;

						pBind[ ( iB + 1 ) * 16 + 12 ] = sd->Inv_Bind_Matrix[ iB ]._41;
						pBind[ ( iB + 1 ) * 16 + 13 ] = sd->Inv_Bind_Matrix[ iB ]._42;
						pBind[ ( iB + 1 ) * 16 + 14 ] = sd->Inv_Bind_Matrix[ iB ]._43;
						pBind[ ( iB + 1 ) * 16 + 15 ] = sd->Inv_Bind_Matrix[ iB ]._44;
					};
					BufferDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
					BufferDesc.ByteWidth = ( sd->Inv_Bind_Matrix.size() + 1 ) * 16 * sizeof( FLOAT32 );
					BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
					BufferDesc.MiscFlags = 0;
					BufferDesc.Usage = D3D10_USAGE_DYNAMIC;
					InitData.pSysMem = ( void* )pBind;
					result = g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
					sd->SkeletalBuffers[ Skin::SBS_Bind ] = pBuffer;
					SRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_BUFFER;
					SRVDesc.Buffer.ElementOffset = 0;
					SRVDesc.Buffer.ElementWidth = ( sd->Inv_Bind_Matrix.size() + 1 ) * 4;
					result = g_Renderer.pDevice->CreateShaderResourceView( pBuffer, &SRVDesc, &pSRV );
					sd->SkeletalBufferSRVs[ Skin::SBS_Bind ] = pSRV;
					delete pBind;
				};
			};
		};
		return 0;
	};

	void Importer_Dom::CopyMatrixToDirectX( D3DXMATRIX& mOut, domListOfFloats& m, UINT32 startindex )
	{
		mOut.m[ 0 ][ 0 ] = CF2F32( m.get( 0 + startindex ) );
		mOut.m[ 0 ][ 1 ] = CF2F32( m.get( 1 + startindex ) );
		mOut.m[ 0 ][ 2 ] = CF2F32( m.get( 2 + startindex ) );
		mOut.m[ 0 ][ 3 ] = CF2F32( m.get( 3 + startindex ) );

		mOut.m[ 1 ][ 0 ] = CF2F32( m.get( 4 + startindex ) );
		mOut.m[ 1 ][ 1 ] = CF2F32( m.get( 5 + startindex ) );
		mOut.m[ 1 ][ 2 ] = CF2F32( m.get( 6 + startindex ) );
		mOut.m[ 1 ][ 3 ] = CF2F32( m.get( 7 + startindex ) );

		mOut.m[ 2 ][ 0 ] = CF2F32( m.get( 8 + startindex ) );
		mOut.m[ 2 ][ 1 ] = CF2F32( m.get( 9 + startindex ) );
		mOut.m[ 2 ][ 2 ] = CF2F32( m.get( 10 + startindex ) );
		mOut.m[ 2 ][ 3 ] = CF2F32( m.get( 11 + startindex ) );
		
		mOut.m[ 3 ][ 0 ] = CF2F32( m.get( 12 + startindex ) );
		mOut.m[ 3 ][ 1 ] = CF2F32( m.get( 13 + startindex ) );
		mOut.m[ 3 ][ 2 ] = CF2F32( m.get( 14 + startindex ) );
		mOut.m[ 3 ][ 3 ] = CF2F32( m.get( 15 + startindex ) );
	};

	INT32 Importer_Dom::ImportOneNode( domNode* node, Node* parent )
	{
		Node* n = m_pRFC->CreateNewNode();
		mapNodes.insert( std::pair< domNode*, Node* >( node, n ) );
		if( node->getId() != NULL )
		{
			mapID2Nodes.insert( std::pair< std::string, Node* >( std::string( node->getId() ), n ) );
		};
		n->pParent = parent;
		n->pParent->setChildren.insert( n );
		if( node->getName() != NULL )
		{
			n->sName = std::string( node->getName() );
		};

		// Transforms
		if( node->getMatrix_array().getCount() > 0 )
		{
			domFloat4x4 &mat = node->getMatrix_array()[ 0 ]->getValue();
			CopyMatrixToDirectX( n->mLocalTransform, mat, 0 );
		}
		else
		{
			D3DXMatrixIdentity( &n->mLocalTransform );
		};

		// Instance light
		domInstance_light_Array &arrLights = node->getInstance_light_array();
		for( UINT32 iL = 0; iL < arrLights.getCount(); iL++ )
		{
			Element* elem = m_pRFC->CreateNewElement();
			elem->eType = Element::ET_Light;
			domLight* light = static_cast< domLight* >( arrLights[ iL ]->getUrl().getElement().cast() );
			elem->pLight = mapLights.find( light )->second;
			n->setElements.insert( elem );
			elem->pNode = n;
		};

		// Instance camera
		domInstance_camera_Array &arrCameras = node->getInstance_camera_array();
		for( UINT32 iC = 0; iC < arrCameras.getCount(); iC++ )
		{
			Element* elem = m_pRFC->CreateNewElement();
			elem->eType = Element::ET_Camera;
			elem->pCamera = mapCameras.find( static_cast< domCamera* >( arrCameras[ iC ]->getUrl().getElement().cast() ) )->second;
			n->setElements.insert( elem );
			elem->pNode = n;
		};

		// Instance static mesh
		domInstance_geometry_Array &arrGeoms = node->getInstance_geometry_array();
		for( UINT32 iG = 0; iG < arrGeoms.getCount(); iG++ )
		{
			Element* elem = m_pRFC->CreateNewElement();
			elem->eType = Element::ET_StaticMesh;
			StaticGeometry* sg = m_pRFC->CreateNewStaticGeometry();
			elem->pStaticMesh = sg;
			sg->pGeometry = mapGeometries.find( static_cast< domGeometry* >( arrGeoms[ iG ]->getUrl().getElement().cast() ) )->second;
			sg->pMaterial = m_pRFC->CreateNewMaterial();
			for( UINT32 iM = 0; iM < sg->pGeometry->vMesh.size(); iM++ )
			{
				sg->pMaterial->vParams.push_back( NULL );
			};
			domInstance_material_Array &arrMats = arrGeoms[ iG ]->getBind_material()->getTechnique_common()->getInstance_material_array();
			for( UINT32 iM = 0; iM < arrMats.getCount(); iM++ )
			{
				std::string symbol( arrMats[ iM ]->getSymbol() );
				for( UINT32 iMesh = 0; iMesh < sg->pGeometry->vMesh.size(); iMesh++ )
				{
					if( sg->pGeometry->vMesh[ iMesh ]->MaterialSymbol == symbol )
					{
						domMaterial* material = static_cast< domMaterial* >( arrMats[ iM ]->getTarget().getElement().cast() );
						sg->pMaterial->vParams[ iMesh ] = mapParams.find( material )->second;
					};
				};
			};
			n->setElements.insert( elem );
			elem->pNode = n;
		};

		// Instance skeletal mesh
		domInstance_controller_Array &arrControllers = node->getInstance_controller_array();
		for( UINT32 iC = 0; iC < arrControllers.getCount(); iC++ )
		{
			Element* elem = m_pRFC->CreateNewElement();
			elem->eType = Element::ET_SkeletalMesh;
			elem->pSkeletalMesh = m_pRFC->CreateNewSkeletalGeometry();
			elem->pSkeletalMesh->Skel = m_pRFC->CreateNewSkeleton();
			domController* controller = static_cast< domController* >( arrControllers[ iC ]->getUrl().getElement().cast() );
			elem->pSkeletalMesh->SkinData = mapSkinData.find( controller )->second;
			domInputLocal_Array &arrInputs = controller->getSkin()->getJoints()->getInput_array();
			for( UINT32 iI = 0; iI < arrInputs.getCount(); iI++ )
			{
				if( std::string( arrInputs[ iI ]->getSemantic() ) == std::string( "JOINT" ) )
				{
					domSource* source = static_cast< domSource* >( arrInputs[ iI ]->getSource().getElement().cast() );
					domListOfNames &arrNames = source->getName_array()->getValue();
					for( UINT32 iJN = 0; iJN < arrNames.getCount(); iJN++ )
					{
						std::string jointname( arrNames[ iJN ] );
						elem->pSkeletalMesh->Skel->JointNames.push_back( jointname );
						elem->pSkeletalMesh->Skel->Joints.push_back( NULL );
					};
				};
			};
			domNode* skelnode = static_cast< domNode* >( arrControllers[ iC ]->getSkeleton_array()[ 0 ]->getValue().getElement().cast() );
			mapSkeleton2Node.insert( std::pair< Skeleton*, domNode* >( elem->pSkeletalMesh->Skel, skelnode ) );
			elem->pSkeletalMesh->Mesh = m_pRFC->CreateNewStaticGeometry();
			domGeometry* geom = static_cast< domGeometry* >( controller->getSkin()->getSource().getElement().cast() );
			elem->pSkeletalMesh->Mesh->pGeometry = mapGeometries.find( geom )->second;
			elem->pSkeletalMesh->Mesh->pMaterial = m_pRFC->CreateNewMaterial();
			for( UINT32 iM = 0; iM < elem->pSkeletalMesh->Mesh->pGeometry->vMesh.size(); iM++ )
			{
				elem->pSkeletalMesh->Mesh->pMaterial->vParams.push_back( NULL );
			};
			domInstance_material_Array &arrMats = arrControllers[ iC ]->getBind_material()->getTechnique_common()->getInstance_material_array();
			for( UINT32 iM = 0; iM < arrMats.getCount(); iM++ )
			{
				std::string symbol( arrMats[ iM ]->getSymbol() );
				for( UINT32 iMesh = 0; iMesh < elem->pSkeletalMesh->Mesh->pGeometry->vMesh.size(); iMesh++ )
				{
					if( elem->pSkeletalMesh->Mesh->pGeometry->vMesh[ iMesh ]->MaterialSymbol == symbol )
					{
						domMaterial* material = static_cast< domMaterial* >( arrMats[ iM ]->getTarget().getElement().cast() );
						elem->pSkeletalMesh->Mesh->pMaterial->vParams[ iMesh ] = mapParams.find( material )->second;
					};
				};
			};
			n->setElements.insert( elem );
			elem->pNode = n;
		};

		// Load child nodes
		domNode_Array &arrNodes = node->getNode_array();
		for( UINT32 iN = 0; iN < arrNodes.getCount(); iN++ )
		{
			ImportOneNode( arrNodes[ iN ].cast(), n );
		};
		return 0;
	};

	INT32 Importer_Dom::ImportJoint( Skeleton* skel, domNode* joint, bool isroot )
	{
		if( joint->getType() == NODETYPE_JOINT )
		{
			std::string sid( joint->getSid() );
			for( UINT32 iB = 0; iB < skel->JointNames.size(); iB++ )
			{
				if( skel->JointNames[ iB ] == sid )
				{
					skel->Joints[ iB ] = mapNodes.find( joint )->second;
					if( isroot == true )
					{
						skel->SkeletonRoot = skel->Joints[ iB ];
					};
					break;
				};
			};
		};

		// Child joints
		domNode_Array &arrNodes = joint->getNode_array();
		for( UINT32 iN = 0; iN < arrNodes.getCount(); iN++ )
		{
			ImportJoint( skel, arrNodes[ iN ].cast(), false );
		};
		return 0;
	};

	INT32 Importer_Dom::ImportNode(domCOLLADA *root)
	{
		domLibrary_visual_scenes_Array &arrLibrary = root->getLibrary_visual_scenes_array();
		for( UINT32 iL = 0; iL < arrLibrary.getCount(); iL++ )
		{
			domVisual_scene_Array &arrVisualScene = arrLibrary[ iL ]->getVisual_scene_array();
			// Only the first scene is loaded for now
			Node* node = m_pRFC->CreateNewNode();
			node->pParent = NULL;
			D3DXMatrixIdentity( &node->mLocalTransform );
			D3DXMatrixIdentity( &node->mAccumulatedTransform );
			Scene* scene = m_pRFC->CreateNewScene();
			scene->pRootNode = node;
			if( arrVisualScene[ 0 ]->getName() != NULL )
			{
				node->sName = std::string( arrVisualScene[ 0 ]->getName() );
			};
			domNode_Array &arrNodes = arrVisualScene[ 0 ]->getNode_array();
			for( UINT32 iN = 0; iN < arrNodes.getCount(); iN++ )
			{
				ImportOneNode( arrNodes[ iN ].cast(), node );
			};

			// Populate skeleton after all nodes are loaded
			for( std::map< Skeleton*, domNode* >::iterator i = mapSkeleton2Node.begin(); i != mapSkeleton2Node.end(); i++ )
			{
				Skeleton* skel = ( *i ).first;
				domNode* joint = ( *i ).second;

				ImportJoint( skel, joint, true );
			};
		};
		return 0;
	};

	INT32 Importer_Dom::ImportOneAnimation( domAnimation* anim, Animation* parent )
	{
		// Only skletal animation is supported, which requires the target to be "node-id/matirx"
		Animation* animation = m_pRFC->CreateNewAnimation();
		if( parent != NULL )
		{
			parent->m_vChildAnims.push_back( animation );
		};
		domSampler_Array &arrSamplers = anim->getSampler_array();
		std::map< domSampler*, UINT32 > mapSamplerIDs;
		for( UINT32 iS = 0; iS < arrSamplers.getCount(); iS++ )
		{
			AnimationSampler< FLOAT32, D3DXMATRIX >&samp = animation->AddSampler();
			mapSamplerIDs.insert( std::pair< domSampler*, UINT32 >( arrSamplers[ iS ].cast(), iS ) );
			domInputLocal_Array &arrInputs = arrSamplers[ iS ]->getInput_array();
			for( UINT32 iInput = 0; iInput < arrInputs.getCount(); iInput++ )
			{
				if( std::string( arrInputs[ iInput ]->getSemantic() ) == std::string( "INPUT" ) )
				{
					domSource* source = static_cast< domSource* >( arrInputs[ iInput ]->getSource().getElement().cast() );
					domListOfFloats &inputs = source->getFloat_array()->getValue();
					if( animation->m_fBeginTime > CF2F32( inputs.get( 0 ) ) )
					{
						animation->m_fBeginTime = CF2F32( inputs.get( 0 ) );
						animation->m_fCurrentTime = animation->m_fBeginTime;
					};
					if( animation->m_fEndTime < CF2F32( inputs.get( inputs.getCount() - 1 ) ) )
					{
						animation->m_fEndTime = CF2F32( inputs.get( inputs.getCount() - 1 ) );
					};
					for( UINT32 i = 0; i < inputs.getCount(); i++ )
					{
						samp.m_Inputs.push_back( CF2F32( inputs.get( i ) ) );
					};
				}
				else if( std::string( arrInputs[ iInput ]->getSemantic() ) == std::string( "OUTPUT" ) )
				{
					domSource* source = static_cast< domSource* >( arrInputs[ iInput ]->getSource().getElement().cast() );
					domListOfFloats &outputs = source->getFloat_array()->getValue();
					UINT32 MatrixCount = outputs.getCount() / 16;
					for( UINT32 i = 0; i < MatrixCount; i++ )
					{
						D3DXMATRIX mat;
						CopyMatrixToDirectX( mat, outputs, i * 16 );
						samp.m_Outputs.push_back( mat );
					};
				}
			};
		};
		// Animation targets
		domChannel_Array &arrChannels = anim->getChannel_array();
		for( UINT32 iC = 0; iC < arrChannels.getCount(); iC++ )
		{
			std::string target( arrChannels[ iC ]->getTarget() );
			std::vector< std::string > splits;
			SplitString( target, '/', splits );
			D3DXMATRIX* mattarget = &mapID2Nodes.find( splits[ 0 ] )->second->mLocalTransform;
			domSampler* sampler = static_cast< domSampler* >( arrChannels[ iC ]->getSource().getElement().cast() );
			animation->m_vSamplerTargets[ mapSamplerIDs.find( sampler )->second ].push_back( mattarget );
		};

		// Children
		domAnimation_Array &arrAnims = anim->getAnimation_array();
		for( UINT32 iA = 0; iA < arrAnims.getCount(); iA++ )
		{
			ImportOneAnimation( arrAnims[ iA ].cast(), animation );
		};
		return 0;
	};

	INT32 Importer_Dom::ImportAnimation(domCOLLADA *root)
	{
		domLibrary_animations_Array &arrLibrary = root->getLibrary_animations_array();
		for( UINT32 iL = 0; iL < arrLibrary.getCount(); iL++ )
		{
			domAnimation_Array &arrAnims = arrLibrary[ iL ]->getAnimation_array();
			for( UINT32 iA = 0; iA < arrAnims.getCount(); iA++ )
			{
				domAnimation* anim = arrAnims[ iA ].cast();
				ImportOneAnimation( anim, NULL );
			};
		};
		return 0;
	};

	void Importer_Dom::CopyColorToDirectX(D3DXCOLOR &cOut, domListOfFloats &color)
	{
		cOut.r = CF2F32( color.get( 0 ) );
		cOut.g = CF2F32( color.get( 1 ) );
		cOut.b = CF2F32( color.get( 2 ) );
		cOut.a = CF2F32( color.get( 3 ) );
	};

	INT32 Importer_Dom::ImportMaterial( domCOLLADA* root )
	{
		HRESULT result;
		domLibrary_materials_Array &arrLibrary = root->getLibrary_materials_array();
		for( UINT32 iL = 0; iL < arrLibrary.getCount(); iL++ )
		{
			domMaterial_Array &arrMats = arrLibrary[ iL ]->getMaterial_array();
			for( UINT32 iMat = 0; iMat < arrMats.getCount(); iMat++ )
			{
				domEffect* effect = static_cast< domEffect* >( arrMats[ iMat ]->getInstance_effect()->getUrl().getElement().cast() );
				domFx_profile_abstract_Array &arrProfiles = effect->getFx_profile_abstract_array();
				for( UINT32 iP = 0; iP < arrProfiles.getCount(); iP++ )
				{
					if( arrProfiles[ iP ]->getElementType() == COLLADA_TYPE::PROFILE_COMMON )
					{
						domProfile_COMMON* common = static_cast< domProfile_COMMON* >( arrProfiles[ iP ].cast() );
						domCommon_newparam_type_Array &arrNewParams = common->getNewparam_array();
						std::map< std::string, domCommon_newparam_type* > mapNewParams;
						for( UINT32 iNP = 0; iNP < arrNewParams.getCount(); iNP++ )
						{
							mapNewParams.insert( std::pair< std::string, domCommon_newparam_type* >( arrNewParams[ iNP ]->getSid(), arrNewParams[ iNP ].cast() ) );
						};
						const domProfile_COMMON::domTechnique::domPhongRef phong = common->getTechnique()->getPhong();
						const domProfile_COMMON::domTechnique::domBlinnRef blinn = common->getTechnique()->getBlinn();
						PhongParam* param = m_pRFC->CreateNewPhongParam();
						param->cAmbient = D3DXCOLOR( 0, 0, 0, 1 );
						param->cDiffuse = D3DXCOLOR( 0, 0, 0, 1 );
						param->cEmission = D3DXCOLOR( 0, 0, 0, 1 );
						param->cReflective = D3DXCOLOR( 0, 0, 0, 1 );
						param->cSpecular = D3DXCOLOR( 0, 0, 0, 1 );
						param->cTransparent = D3DXCOLOR( 0, 0, 0, 1 );
						param->fIndexOfRefraction = 1;
						param->fReflectivity = 1;
						param->fShininess = 1;
						param->fTransparency = 0;
						param->pDiffuse = NULL;
						mapParams.insert( std::pair< domMaterial*, PhongParam* >( arrMats[ iMat ].cast(), param ) );
						if( phong.cast() != NULL )
						{
							// Ambient
							if( phong->getAmbient().cast() != NULL && phong->getAmbient()->getColor().cast() != NULL )
							{
								domFx_color_common &ambient = phong->getAmbient()->getColor()->getValue();
								CopyColorToDirectX( param->cAmbient, ambient );
							};
							// Diffuse
							if( phong->getDiffuse().cast() != NULL && phong->getDiffuse()->getTexture().cast() != NULL )
							{
								std::string tex( phong->getDiffuse()->getTexture()->getTexture() );
								std::string sampler( mapNewParams.find( tex )->second->getSampler2D()->getSource()->getValue() );
								domImage* image = static_cast< domImage* >( mapNewParams.find( sampler )->second->getSurface()->getFx_surface_init_common()->getInit_from_array()[ 0 ]->getValue().getElement() );
								const std::string& imagepathdae = image->getInit_from()->getValue().str();
								const std::string& imagepath = imagepathdae.substr( 6, imagepathdae.size() - 6 );
								D3DX10_IMAGE_LOAD_INFO info;
								info.BindFlags = D3DX10_DEFAULT;
								info.CpuAccessFlags = D3DX10_DEFAULT;
								info.Depth = D3DX10_DEFAULT;
								info.Filter = D3DX10_DEFAULT;
								info.FirstMipLevel = D3DX10_DEFAULT;
								info.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
								info.Height = D3DX10_DEFAULT;
								info.MipFilter = D3DX10_DEFAULT;
								info.MipLevels = D3DX10_DEFAULT;
								info.MiscFlags = D3DX10_DEFAULT;
								info.pSrcInfo = NULL;
								info.Usage = D3D10_USAGE_DEFAULT;
								info.Width = D3DX10_DEFAULT;
								result = D3DX10CreateShaderResourceViewFromFileA( g_Renderer.pDevice, imagepath.c_str(), NULL, NULL, &param->pDiffuse, &result );
								param->DiffuseTexPath = imagepath;
								//ID3D10Texture2D* ptexture = NULL;
								//param->pDiffuse->GetResource((ID3D10Resource **) &ptexture );
								//D3D10_TEXTURE2D_DESC desc;
								//ptexture->GetDesc( &desc );
							}
							else if( phong->getDiffuse().cast() != NULL && phong->getDiffuse()->getColor().cast() != NULL )
							{
								CopyColorToDirectX( param->cDiffuse, phong->getDiffuse()->getColor()->getValue() );
							};
							// Emission
							if( phong->getEmission().cast() != NULL && phong->getEmission()->getColor().cast() != NULL )
							{
								CopyColorToDirectX( param->cEmission, phong->getEmission()->getColor()->getValue() );
							};
							// Specular
							if( phong->getSpecular().cast() != NULL && phong->getSpecular()->getColor().cast() != NULL )
							{
								CopyColorToDirectX( param->cSpecular, phong->getSpecular()->getColor()->getValue() );
							};
							// Shininess
							if( phong->getSpecular().cast() != NULL )
							{
								param->fShininess = CF2F32( phong->getShininess()->getFloat()->getValue() );
							};
						}
						else if( blinn.cast() != NULL )
						{
							// Ambient
							if( blinn->getAmbient().cast() != NULL && blinn->getAmbient()->getColor().cast() != NULL )
							{
								domFx_color_common &ambient = blinn->getAmbient()->getColor()->getValue();
								CopyColorToDirectX( param->cAmbient, ambient );
							};
							// Diffuse
							if( blinn->getDiffuse().cast() != NULL && blinn->getDiffuse()->getTexture().cast() != NULL )
							{
								std::string tex( blinn->getDiffuse()->getTexture()->getTexture() );
								std::string sampler( mapNewParams.find( tex )->second->getSampler2D()->getSource()->getValue() );
								domImage* image = static_cast< domImage* >( mapNewParams.find( sampler )->second->getSurface()->getFx_surface_init_common()->getInit_from_array()[ 0 ]->getValue().getElement() );
								const std::string& imagepathdae = image->getInit_from()->getValue().str();
								const std::string& imagepath = imagepathdae.substr( 6, imagepathdae.size() - 6 );
								D3DX10_IMAGE_LOAD_INFO info;
								info.BindFlags = D3DX10_DEFAULT;
								info.CpuAccessFlags = D3DX10_DEFAULT;
								info.Depth = D3DX10_DEFAULT;
								info.Filter = D3DX10_DEFAULT;
								info.FirstMipLevel = D3DX10_DEFAULT;
								info.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
								info.Height = D3DX10_DEFAULT;
								info.MipFilter = D3DX10_DEFAULT;
								info.MipLevels = D3DX10_DEFAULT;
								info.MiscFlags = D3DX10_DEFAULT;
								info.pSrcInfo = NULL;
								info.Usage = D3D10_USAGE_DEFAULT;
								info.Width = D3DX10_DEFAULT;
								result = D3DX10CreateShaderResourceViewFromFileA( g_Renderer.pDevice, imagepath.c_str(), NULL, NULL, &param->pDiffuse, &result );
								param->DiffuseTexPath = imagepath;
								ID3D10Texture2D* ptexture = NULL;
								//param->pDiffuse->GetResource((ID3D10Resource **) &ptexture );
								//D3D10_TEXTURE2D_DESC desc;
								//ptexture->GetDesc( &desc );
							}
							else if( blinn->getDiffuse().cast() != NULL && blinn->getDiffuse()->getColor().cast() != NULL )
							{
								CopyColorToDirectX( param->cDiffuse, blinn->getDiffuse()->getColor()->getValue() );
							};
							// Emission
							if( blinn->getEmission().cast() != NULL && blinn->getEmission()->getColor().cast() != NULL )
							{
								CopyColorToDirectX( param->cEmission, blinn->getEmission()->getColor()->getValue() );
							};
							// Specular
							if( blinn->getSpecular().cast() != NULL && blinn->getSpecular()->getColor().cast() != NULL )
							{
								CopyColorToDirectX( param->cSpecular, blinn->getSpecular()->getColor()->getValue() );
							};
							// Shininess
							if( blinn->getSpecular().cast() != NULL )
							{
								param->fShininess = CF2F32( blinn->getShininess()->getFloat()->getValue() );
							};
						};
					};
				};
			};
		};
		return 0;
	};
};