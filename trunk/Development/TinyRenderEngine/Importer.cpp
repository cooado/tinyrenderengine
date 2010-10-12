#include "Prerequisites.h"

namespace TRE
{
	INT32 Importer::ImportFromDaeFile( std::string f )
	{		
		COLLADASaxFWL::Loader loader;
		COLLADAFW::Root root( &loader, this );

		mRun = IR_RESOURCE;
		if( !root.loadDocument( f ) )
		{
			return 1;
		};

		ConnectInstances();

		return 0;
	};

	void Importer::ConnectInstances()
	{
		// Scene
		for( std::map< Scene*, COLLADAFW::UniqueId >::iterator i = mapScene2VisualNodes.begin(); i != mapScene2VisualNodes.end(); i++ )
		{
			i->first->pRootNode = mapNodes.find( i->second )->second;
		};

		// Node
		for( std::map< COLLADAFW::UniqueId, Node* >::iterator i = mapNodes.begin(); i != mapNodes.end(); i++ )
		{
			Node* node = i->second;
			node->pEntity = m_pRFC->CreateNewEntity();

			// Camera
			std::map< Node*, COLLADAFW::UniqueId >::iterator iCam = mapNodes2Cameras.find( node );
			if( iCam != mapNodes2Cameras.end() )
			{
				Element* elem = m_pRFC->CreateNewElement();
				elem->eType = Element::ET_Camera;
				elem->pCamera = mapCameras.find( iCam->second )->second;
				node->pEntity->setElements.insert( elem );
			};

			// Light
			std::map< Node*, COLLADAFW::UniqueId >::iterator iLight = mapNodes2Lights.find( node );
			if( iLight != mapNodes2Lights.end() )
			{
				Element* elem = m_pRFC->CreateNewElement();
				elem->eType = Element::ET_Light;
				std::map< COLLADAFW::UniqueId, Light* >::iterator iL = mapLights.find( iLight->second );
				if( iL != mapLights.end() )
				{
					elem->pLight = iL->second;
				}
				else
				{
					elem->pLight = NULL;
				};
				node->pEntity->setElements.insert( elem );
			};

			// Static mesh
			std::map< Node*, COLLADAFW::UniqueId >::iterator iGeom = mapNodes2Geometries.find( node );
			if( iGeom != mapNodes2Geometries.end() )
			{
				Element* elem = m_pRFC->CreateNewElement();
				elem->eType = Element::ET_StaticMesh;
				StaticGeometry* geom = mapStaticMeshes.find( iGeom->second )->second;

				// Material
				for( std::vector< Mesh* >::iterator iMesh = geom->vMesh.begin(); iMesh != geom->vMesh.end(); iMesh++ )
				{
					Mesh* mesh = *iMesh;
					std::map< Mesh*, COLLADAFW::MaterialId >::iterator iMaterial = mapStaticMeshes2Material.find( mesh );
					if( iMaterial != mapStaticMeshes2Material.end() )
					{
						COLLADAFW::UniqueId refmat = mapMatID2Mat.find( iMaterial->second )->second;
						PhongParam* param = mapPhongParams.find( mapMaterials2Effects.find( refmat )->second )->second;
						// Diffuse texture
						std::map< COLLADAFW::UniqueId, COLLADAFW::UniqueId >::iterator iDiffuse = mapEffectDiffuse2Images.find( mapMaterials2Effects.find( refmat )->second );
						if( iDiffuse != mapEffectDiffuse2Images.end() )
						{
							std::string ImageURI = mapImageID2ImageURL.find( iDiffuse->second )->second;
							HRESULT result;
							D3DX10CreateShaderResourceViewFromFileA( g_Renderer.pDevice, ImageURI.c_str(), NULL, NULL, &mesh->pParam->pDiffuse, &result );
						};
					};
				};

				elem->pStaticMesh = geom;
				node->pEntity->setElements.insert( elem );
			};

			// SkeletalMesh
			std::map< Node*, COLLADAFW::UniqueId >::iterator iController = mapNodes2Controllers.find( node );
			if( iController != mapNodes2Controllers.end() )
			{
				Element* elem = m_pRFC->CreateNewElement();
				elem->eType = Element::ET_SkeletalMesh;
				SkeletalGeometry* skel = mapSkeletalControllers.find( iController->second )->second;
				elem->pSkeletalMesh = skel;
				node->pEntity->setElements.insert( elem );

				// Geometry
				std::map< SkeletalGeometry*, COLLADAFW::UniqueId >::iterator iGeometry = mapSkeletalMesh2Geometry.find( skel );
				skel->Mesh = mapStaticMeshes.find( iGeometry->second )->second;

				// Material
				for( std::vector< Mesh* >::iterator iMesh = skel->Mesh->vMesh.begin(); iMesh != skel->Mesh->vMesh.end(); iMesh++ )
				{
					Mesh* mesh = *iMesh;
					std::map< Mesh*, COLLADAFW::MaterialId >::iterator iMaterial = mapStaticMeshes2Material.find( mesh );
					if( iMaterial != mapStaticMeshes2Material.end() )
					{
						COLLADAFW::UniqueId refmat = mapMatID2Mat.find( iMaterial->second )->second;
						PhongParam* param = mapPhongParams.find( mapMaterials2Effects.find( refmat )->second )->second;
						// Diffuse texture
						std::map< COLLADAFW::UniqueId, COLLADAFW::UniqueId >::iterator iDiffuse = mapEffectDiffuse2Images.find( mapMaterials2Effects.find( refmat )->second );
						if( iDiffuse != mapEffectDiffuse2Images.end() )
						{
							std::string ImageURI = mapImageID2ImageURL.find( iDiffuse->second )->second;
							HRESULT result;
							D3DX10CreateShaderResourceViewFromFileA( g_Renderer.pDevice, ImageURI.c_str(), NULL, NULL, &mesh->pParam->pDiffuse, &result );
						};
					};
				};

				// Skindata
				skel->SkinData = mapSkinDatas.find( mapSkeletalMesh2SkinData.find( skel )->second )->second;

				// Skeleton
				COLLADAFW::UniqueIdArray& arrJoints = mapSkeletalMesh2Joints.find( skel )->second;
				skel->Skel->SkeletonRoot = mapNodes.find( arrJoints[ 0 ] )->second ;
				for( UINT32 iJ = 0; iJ < arrJoints.getCount(); iJ++ )
				{
					skel->Skel->Joints.push_back( mapNodes.find( arrJoints[ iJ ] )->second );
				};
			};
		};
	};

	void Importer::cancel(const COLLADAFW::String &errorMessage)
	{
		TRE_LOG( "Failed loading Collada File\n" );
	};

	void Importer::start()
	{
	};

	void Importer::finish()
	{
	};


	bool Importer::writeGlobalAsset ( const COLLADAFW::FileInfo* asset )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		return true;
	};

	bool Importer::writeScene ( const COLLADAFW::Scene* scene )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		Scene* s = m_pRFC->CreateNewScene();
		s->pRootNode = NULL;

		mapScene2VisualNodes.insert( std::pair< Scene*, COLLADAFW::UniqueId >( s, scene->getInstanceVisualScene()->getInstanciatedObjectId() ) );

		return true;
	};

	bool Importer::ImportVisualSceneNode( const COLLADAFW::Node* node , Node* parent )
	{
		Node* child = m_pRFC->CreateNewNode();
		child->pParent = parent;
		D3DXMatrixIdentity( &child->mLocalTransform );
		D3DXMatrixIdentity( &child->mAccumulatedTransform );
		parent->setChildren.insert( child );

		const COLLADAFW::TransformationPointerArray& arrTrans = node->getTransformations();
		D3DXMATRIX m;
		D3DXMatrixIdentity( &m );
		for( UINT32 iTrans = 0; iTrans < arrTrans.getCount(); iTrans++ )
		{
			COLLADAFW::Transformation* transform = arrTrans[ iTrans ];
			switch( transform->getTransformationType() )
			{
			case COLLADAFW::Transformation::MATRIX:
				{
					COLLADAFW::Matrix* matrix = static_cast< COLLADAFW::Matrix* >( transform );
					D3DXMATRIX mat;
					CopyMatrixToDirectX( mat, matrix->getMatrix() );
					D3DXMatrixMultiply( &m, &m, &mat );
				};
				break;

			case COLLADAFW::Transformation::TRANSLATE:
				{
					COLLADAFW::Translate* translate = static_cast< COLLADAFW::Translate* >( transform );
					COLLADABU::Math::Vector3& v3 = translate->getTranslation();
					D3DXMATRIX mat;
					D3DXMatrixTranslation( &mat, v3.x, v3.y, v3.z );
					D3DXMatrixMultiply( &m, &m, &mat );
				};
				break;

			case COLLADAFW::Transformation::ROTATE:
				{
					COLLADAFW::Rotate* rot = static_cast< COLLADAFW::Rotate* >( transform );
					D3DXVECTOR3 v;
					v.x = static_cast< float >( rot->getRotationAxis().x ); 
					v.y = static_cast< float >( rot->getRotationAxis().y ); 
					v.z = static_cast< float >( rot->getRotationAxis().z );
					D3DXMATRIX mat;
					D3DXMatrixRotationAxis( &mat, &v, D3DXToRadian( static_cast< float >( rot->getRotationAngle() ) );
					D3DXMatrixMultiply( &m, &m, &mat );
				};
				break;
			case COLLADAFW::Transformation::SCALE:
				// To-do
				break;
			case COLLADAFW::Transformation::LOOKAT:
				// To-do
				break;
			case COLLADAFW::Transformation::SKEW:
				// To-do
				break;
			default:
				break;
			};
		};

		// Instance camera
		const COLLADAFW::InstanceCameraPointerArray& cameras = node->getInstanceCameras();
		for( UINT32 iCam = 0; iCam < cameras.getCount(); iCam++ )
		{
			mapNodes2Cameras.insert( std::pair< Node*, COLLADAFW::UniqueId >( child, cameras[ 0 ]->getInstanciatedObjectId() ) );
			break;
		};

		// Instance geometry
		const COLLADAFW::InstanceGeometryPointerArray& geoms = node->getInstanceGeometries();
		for( UINT32 iGeom = 0; iGeom < geoms.getCount(); iGeom++ )
		{
			mapNodes2Geometries.insert( std::pair< Node*, COLLADAFW::UniqueId >( child, geoms[ iGeom ]->getInstanciatedObjectId() ) );
			COLLADAFW::InstanceGeometry::MaterialBindingArray& matbindings = geoms[ iGeom ]->getMaterialBindings();
			mapMatID2Mat.insert( std::pair< COLLADAFW::MaterialId, COLLADAFW::UniqueId >( matbindings[ 0 ].getMaterialId(), matbindings[ 0 ].getReferencedMaterial() ) );
			break;
		};

		// Instance light
		const COLLADAFW::InstanceLightPointerArray& lights = node->getInstanceLights();
		for( UINT32 iLight = 0; iLight < lights.getCount(); iLight++ )
		{
			mapNodes2Lights.insert( std::pair< Node*, COLLADAFW::UniqueId >( child, lights[ 0 ]->getInstanciatedObjectId() ) );
			break;
		};

		// Instance controller
		const COLLADAFW::InstanceControllerArray& controllers = node->getInstanceControllers();
		for( UINT32 iC = 0; iC < controllers.getCount(); iC++ )
		{
			mapNodes2Controllers.insert( std::pair< Node*, COLLADAFW::UniqueId >( child, controllers[ iC ]->getInstanciatedObjectId() ) );
			COLLADAFW::InstanceGeometry::MaterialBindingArray& matbindings = controllers[ iC ]->getMaterialBindings();
			mapMatID2Mat.insert( std::pair< COLLADAFW::MaterialId, COLLADAFW::UniqueId >( matbindings[ 0 ].getMaterialId(), matbindings[ 0 ].getReferencedMaterial() ) );
		};

		mapNodes.insert( std::pair< COLLADAFW::UniqueId, Node* >( node->getUniqueId(), child ) );

		const COLLADAFW::NodePointerArray& inputchildren = node->getChildNodes();
		for( UINT32 iInput = 0; iInput < inputchildren.getCount(); iInput++ )
		{
			COLLADAFW::Node* input = inputchildren[ iInput ];
			ImportVisualSceneNode( input, child );
		};

		return true;
	};

	bool Importer::writeVisualScene ( const COLLADAFW::VisualScene* visualScene )
	{	
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		Node* myrootnode = m_pRFC->CreateNewNode();
		myrootnode->pParent = NULL;
		D3DXMatrixIdentity( &myrootnode->mLocalTransform );
		D3DXMatrixIdentity( &myrootnode->mAccumulatedTransform );
		mapNodes.insert( std::pair< COLLADAFW::UniqueId, Node* >( visualScene->getUniqueId(), myrootnode ) );

		const COLLADAFW::NodePointerArray& roots = visualScene->getRootNodes();
		for( UINT32 i = 0; i < roots.getCount(); i++ )
		{
			const COLLADAFW::Node* node = roots[ i ];	
			ImportVisualSceneNode( node, myrootnode );
		};

		return true;
	};

	bool Importer::writeLibraryNodes ( const COLLADAFW::LibraryNodes* libraryNodes )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		return true;
	};

	bool Importer::writeGeometry ( const COLLADAFW::Geometry* geometry )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		switch( geometry->getType() )
		{
		case COLLADAFW::Geometry::GEO_TYPE_MESH:
			{
				HRESULT result;
				const COLLADAFW::Mesh* mesh = static_cast< const COLLADAFW::Mesh* >( geometry );
				const COLLADAFW::MeshVertexData& pos_data = mesh->getPositions();
				const COLLADAFW::MeshVertexData& norm_data = mesh->getNormals();
				const COLLADAFW::MeshVertexData& uv_data = mesh->getUVCoords();

				UINT32 pos_stride = 3;
				UINT32 norm_stride = 3;
				UINT32 nv_stride = uv_data.getStride( 0 );
				
				ID3D10Buffer* pVB[ 3 ];
				UINT32 VBStrides[ 3 ];
				if( pos_data.getType() == COLLADAFW::FloatOrDoubleArray::DATA_TYPE_FLOAT &&
					norm_data.getType() == COLLADAFW::FloatOrDoubleArray::DATA_TYPE_FLOAT &&
					uv_data.getType() == COLLADAFW::FloatOrDoubleArray::DATA_TYPE_FLOAT )
				{
					const FLOAT32* fPos = pos_data.getFloatValues()->getData();
					UINT32 pos_count = pos_data.getFloatValues()->getCount();
					const FLOAT32* fNorm = norm_data.getFloatValues()->getData();
					UINT32 norm_count = norm_data.getFloatValues()->getCount();
					const FLOAT32* fUV = uv_data.getFloatValues()->getData();
					UINT32 uv_count = uv_data.getFloatValues()->getCount();

					VBStrides[ 0 ] = pos_stride * sizeof( FLOAT32 );
					VBStrides[ 1 ] = norm_stride * sizeof( FLOAT32 );
					VBStrides[ 2 ] = nv_stride * sizeof( FLOAT32 );

					D3D10_BUFFER_DESC pVBDesc[] = 
					{
						{ pos_count * sizeof( FLOAT32 ), D3D10_USAGE_IMMUTABLE, D3D10_BIND_VERTEX_BUFFER, 0, 0 },
						{ norm_count * sizeof( FLOAT32 ), D3D10_USAGE_IMMUTABLE, D3D10_BIND_VERTEX_BUFFER, 0, 0 },
						{ uv_count * sizeof( FLOAT32 ), D3D10_USAGE_IMMUTABLE, D3D10_BIND_VERTEX_BUFFER, 0, 0 },
					};

					D3D10_SUBRESOURCE_DATA pVBInit[] = 
					{
						{ fPos, 0, 0 },
						{ fNorm, 0, 0 },
						{ fUV, 0, 0 },
					};

					result = g_Renderer.pDevice->CreateBuffer( &pVBDesc[ 0 ], &pVBInit[ 0 ], &pVB[ 0 ] );
					result = g_Renderer.pDevice->CreateBuffer( &pVBDesc[ 1 ], &pVBInit[ 1 ], &pVB[ 1 ] );
					result = g_Renderer.pDevice->CreateBuffer( &pVBDesc[ 2 ], &pVBInit[ 2 ], &pVB[ 2 ] );

				}
				else
				{
					return false;
				};

				ID3D10Buffer* pIB = NULL;
				StaticGeometry* g = m_pRFC->CreateNewStaticGeometry();
				const COLLADAFW::MeshPrimitiveArray& primitives = mesh->getMeshPrimitives();
				for( UINT32 iPrim = 0; iPrim < primitives.getCount(); iPrim++ )
				{
					COLLADAFW::MeshPrimitive* prim = primitives[ iPrim ];
					UINT32 index_count = prim->getPositionIndices().getCount();
					UINT32* indices = prim->getPositionIndices().getData();

					D3D10_BUFFER_DESC pIBDesc = 
						{ index_count * sizeof( UINT32 ), D3D10_USAGE_IMMUTABLE, D3D10_BIND_INDEX_BUFFER, 0, 0 };

					D3D10_SUBRESOURCE_DATA pIBInit = 
						{ indices, 0, 0 };

					result = g_Renderer.pDevice->CreateBuffer( &pIBDesc, &pIBInit, &pIB );

					Mesh* sm = m_pRFC->CreateNewMesh();

					sm->Buffers[ Mesh::BS_POSITION ] = pVB[ 0 ];
					sm->Buffers[ Mesh::BS_NORMAL ] = pVB[ 1 ];
					sm->Buffers[ Mesh::BS_TEXCOORD0 ] = pVB[ 2 ];
					sm->Buffers[ Mesh::BS_INDEX ] = pIB;
					sm->IndexCount = index_count;
					sm->IndexStartLocation = 0;
					sm->BufferStrides[ Mesh::BS_POSITION ] = VBStrides[ 0 ];
					sm->BufferStrides[ Mesh::BS_NORMAL ] = VBStrides[ 1 ];
					sm->BufferStrides[ Mesh::BS_TEXCOORD0 ] = VBStrides[ 2 ];
					sm->BufferStrides[ Mesh::BS_INDEX ] = 4;

					sm->pParam = NULL;

					mapStaticMeshes2Material.insert( std::pair< Mesh*, COLLADAFW::MaterialId >( sm, prim->getMaterialId() ) );
					g->vMesh.push_back( sm );
				};			
				mapStaticMeshes.insert( std::pair< COLLADAFW::UniqueId, StaticGeometry* >( geometry->getUniqueId(), g ) );

			};
			break;
		default:
			break;
		};

		return true;
	};

	bool Importer::writeMaterial( const COLLADAFW::Material* material )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		mapMaterials2Effects.insert( std::pair< COLLADAFW::UniqueId, COLLADAFW::UniqueId >( material->getUniqueId(), material->getInstantiatedEffect() ) );

		return true;
	};

	bool Importer::writeEffect( const COLLADAFW::Effect* effect )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		PhongParam* param = m_pRFC->CreateNewPhongParam();

		const COLLADAFW::CommonEffectPointerArray& commons = effect->getCommonEffects();
		COLLADAFW::EffectCommon* common = commons[ 0 ];

		// ambient
		if( COLLADAFW::ColorOrTexture::COLOR == common->getAmbient().getType() )
		{
			param->cAmbient.r = static_cast< FLOAT32 >( common->getAmbient().getColor().getRed() );
			param->cAmbient.g = static_cast< FLOAT32 >( common->getAmbient().getColor().getGreen() );
			param->cAmbient.b = static_cast< FLOAT32 >( common->getAmbient().getColor().getBlue() );
			param->cAmbient.a = static_cast< FLOAT32 >( common->getAmbient().getColor().getAlpha() );
		}
		else
		{
			param->cAmbient.r = 0.0f;
			param->cAmbient.b = 0.0f;
			param->cAmbient.b = 0.0f;
			param->cAmbient.a = 0.0f;
		};

		// diffuse
		if( COLLADAFW::ColorOrTexture::COLOR == common->getDiffuse().getType() )
		{
			param->cDiffuse.r = static_cast< FLOAT32 >( common->getDiffuse().getColor().getRed() );
			param->cDiffuse.g = static_cast< FLOAT32 >( common->getDiffuse().getColor().getGreen() );
			param->cDiffuse.b = static_cast< FLOAT32 >( common->getDiffuse().getColor().getBlue() );
			param->cDiffuse.a = static_cast< FLOAT32 >( common->getDiffuse().getColor().getAlpha() );
		}
		else
		{
			COLLADAFW::SamplerPointerArray& samplers = common->getSamplerPointerArray();
			COLLADAFW::Sampler* sampler = samplers[ common->getDiffuse().getTexture().getSamplerId() ];
			mapEffectDiffuse2Images.insert( std::pair< COLLADAFW::UniqueId, COLLADAFW::UniqueId >( effect->getUniqueId(), sampler->getSourceImage() ) );

			param->cDiffuse.r = 0.0f;
			param->cDiffuse.b = 0.0f;
			param->cDiffuse.b = 0.0f;
			param->cDiffuse.a = 0.0f;
		};
		param->pDiffuse = NULL;

		// specular
		if( COLLADAFW::ColorOrTexture::COLOR == common->getSpecular().getType() )
		{
			param->cSpecular.r = static_cast< FLOAT32 >( common->getSpecular().getColor().getRed() );
			param->cSpecular.g = static_cast< FLOAT32 >( common->getSpecular().getColor().getGreen() );
			param->cSpecular.b = static_cast< FLOAT32 >( common->getSpecular().getColor().getBlue() );
			param->cSpecular.a = static_cast< FLOAT32 >( common->getSpecular().getColor().getAlpha() );
		}
		else
		{
			param->cSpecular.r = 0.0f;
			param->cSpecular.b = 0.0f;
			param->cSpecular.b = 0.0f;
			param->cSpecular.a = 0.0f;
		};

		// emission
		if( COLLADAFW::ColorOrTexture::COLOR == common->getEmission().getType() )
		{
			param->cEmission.r = static_cast< FLOAT32 >( common->getEmission().getColor().getRed() );
			param->cEmission.g = static_cast< FLOAT32 >( common->getEmission().getColor().getGreen() );
			param->cEmission.b = static_cast< FLOAT32 >( common->getEmission().getColor().getBlue() );
			param->cEmission.a = static_cast< FLOAT32 >( common->getEmission().getColor().getAlpha() );
		}
		else
		{
			param->cEmission.r = 0.0f;
			param->cEmission.b = 0.0f;
			param->cEmission.b = 0.0f;
			param->cEmission.a = 0.0f;
		};

		// shininess
		if( COLLADAFW::FloatOrParam::FLOAT == common->getShininess().getType() )
		{
			param->fShininess = common->getShininess().getFloatValue();
		}
		else
		{
			param->fShininess = 1.0f;
		};

		mapPhongParams.insert( std::pair< COLLADAFW::UniqueId, PhongParam* >( effect->getUniqueId(), param ) );

		return true;
	};

	bool Importer::writeCamera( const COLLADAFW::Camera* camera )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		Camera* cam = m_pRFC->CreateNewCamera();
		cam->sName = camera->getName();
		cam->eType = Camera::CT_PERSPECTIVE;
		cam->fNearClip = static_cast< FLOAT32 >( camera->getNearClippingPlane().getValue() );
		cam->fFarClip = static_cast< FLOAT32 >( camera->getFarClippingPlane() );
		cam->fFOVY = static_cast< FLOAT32 >( camera->getYFov().getValue() );
		INT32 x, y;
		TRE::TinyRenderEngine::GetInstance()->GetScreenResolution( x, y );
		cam->fAspect = static_cast< FLOAT32 >( x ) / static_cast< FLOAT32 >( y );

		mapCameras.insert( std::pair< COLLADAFW::UniqueId, Camera* >( camera->getUniqueId(), cam ) );

		return true;
	};

	bool Importer::writeImage( const COLLADAFW::Image* image )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		mapImageID2ImageURL.insert( std::pair< COLLADAFW::UniqueId, std::string >( image->getUniqueId(), image->getImageURI().getURIString() ) );

		return true;
	};

	bool Importer::writeLight( const COLLADAFW::Light* light )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		Light* l = m_pRFC->CreateNewLight();
		l->sName = light->getName();
		switch( light->getLightType() )
		{
		case COLLADAFW::Light::DIRECTIONAL_LIGHT:
			l->eType = Light::LT_DIRECTIONAL;
			break;
		case COLLADAFW::Light::POINT_LIGHT:
			l->eType = Light::LT_POINT;
			break;
		default:
			l->eType = Light::LT_DIRECTIONAL;
			break;
		};
		const COLLADAFW::Color& color = light->getColor();
		l->cColor.r = static_cast< FLOAT32 >( color.getRed() );
		l->cColor.g = static_cast< FLOAT32 >( color.getGreen() );
		l->cColor.b = static_cast< FLOAT32 >( color.getBlue() );
		l->cColor.a = static_cast< FLOAT32 >( color.getAlpha() );

		mapLights.insert( std::pair< COLLADAFW::UniqueId, Light* >( light->getUniqueId(), l ) );

		return true;
	};

	bool Importer::writeAnimation( const COLLADAFW::Animation* animation )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		if( animation->getAnimationType() == COLLADAFW::Animation::ANIMATION_CURVE )
		{
			Animation* anim = m_pRFC->CreateNewAnimation();
			AnimationSampler< FLOAT32, D3DXMATRIX > sampler = anim->AddSampler();

			COLLADAFW::AnimationCurve* animcurve = ( COLLADAFW::AnimationCurve* )animation;
			COLLADAFW::AnimationCurve::InterpolationType type = animcurve->getInterpolationType();
			COLLADAFW::PhysicalDimension IPD = animcurve->getInPhysicalDimension();
			UINT32 keycount = animcurve->getInputValues().getValuesCount();
			COLLADAFW::FloatArray* inputs = animcurve->getInputValues().getFloatValues();

			for( UINT32 i = 0; i < keycount; i++ )
			{
				sampler.m_Inputs.push_back( ( *inputs )[ i ] );
			};

			UINT32 outdimension = animcurve->getOutDimension();
			COLLADAFW::FloatArray* outputs = animcurve->getOutputValues().getFloatValues();
			for( UINT32 i = 0; i < keycount; i++ )
			{
				D3DXMATRIX m( outputs->getData() + keycount * 16 );
				sampler.m_Outputs.push_back( m );
			};

			mapAnimations.insert( std::pair< COLLADAFW::UniqueId, Animation* >( animation->getUniqueId(), anim ) );
		};

		return true;
	};

	bool Importer::writeAnimationList( const COLLADAFW::AnimationList* animationList )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		const COLLADAFW::AnimationList::AnimationBindings& ab = animationList->getAnimationBindings();
		mapAnimationList2Animation.insert( std::pair< COLLADAFW::UniqueId, COLLADAFW::UniqueId >( animationList->getUniqueId(), ab[ 0 ].animation ) );

		return true;
	};

	void Importer::CopyMatrixToDirectX( D3DXMATRIX& mOut, const COLLADABU::Math::Matrix4& m )
	{
		mOut.m[ 0 ][ 0 ] = m.m[ 0 ][ 0 ];
		mOut.m[ 0 ][ 1 ] = m.m[ 1 ][ 0 ];
		mOut.m[ 0 ][ 2 ] = m.m[ 2 ][ 0 ];
		mOut.m[ 0 ][ 3 ] = m.m[ 3 ][ 0 ];

		mOut.m[ 1 ][ 0 ] = m.m[ 0 ][ 1 ];
		mOut.m[ 1 ][ 1 ] = m.m[ 1 ][ 1 ];
		mOut.m[ 1 ][ 2 ] = m.m[ 2 ][ 1 ];
		mOut.m[ 1 ][ 3 ] = m.m[ 3 ][ 1 ];

		mOut.m[ 2 ][ 0 ] = m.m[ 0 ][ 2 ];
		mOut.m[ 2 ][ 1 ] = m.m[ 1 ][ 2 ];
		mOut.m[ 2 ][ 2 ] = m.m[ 2 ][ 2 ];
		mOut.m[ 2 ][ 3 ] = m.m[ 3 ][ 2 ];
		
		mOut.m[ 3 ][ 0 ] = m.m[ 0 ][ 3 ];
		mOut.m[ 3 ][ 1 ] = m.m[ 1 ][ 3 ];
		mOut.m[ 3 ][ 2 ] = m.m[ 2 ][ 3 ];
		mOut.m[ 3 ][ 3 ] = m.m[ 3 ][ 3 ];
	};

	bool Importer::writeSkinControllerData( const COLLADAFW::SkinControllerData* skinControllerData )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		SkinData* skin = m_pRFC->CreateNewSkinData();
		mapSkinDatas.insert( std::pair< COLLADAFW::UniqueId, SkinData* >( skinControllerData->getUniqueId(), skin ) );

		// Bind shape matrix
		cons COLLADABU::Math::Matrix4& mbind = skinControllerData->getBindShapeMatrix();
		CopyMatrixToDirectX( skin->Bind_Shape_Matrix, mbind );

		const COLLADAFW::Matrix4Array& arrIBMs = skinControllerData->getInverseBindMatrices();
		const COLLADAFW::FloatOrDoubleArray& arrWeights = skinControllerData->getWeights();
		const COLLADAFW::IntValuesArray& arrJI = skinControllerData->getJointIndices();
		const UIntValuesArray& arrWI = skinControllerData->getWeightIndices();
		const COLLADAFW::UIntValuesArray& arrJPV = skinControllerData->getJointsPerVertex();
		UINT32 JCount = skinControllerData->getJointsCount();
		UINT32 VCount = arrJPV.getCount();
		UINT32 JIndexCount = arrJI.getCount();

		COLLADAFW::UIntValuesArray arrJointsOffset;
		arrJointsOffset.append( 0 );
		for( UINT32 i = 1; i < VCount; i++ )
		{
			arrJointsOffset.append( arrJoints[ i - 1 ] + arrJPV[ i - 1 ] );
		};

		// Weight buffer
		ID3D10Buffer* pBuffer = NULL;
		D3D10_BUFFER_DESC BufferDesc;
		BufferDesc.Usage = D3D10_USAGE_DEFAULT;
		BufferDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		BufferDesc.ByteWidth = sizeof( FLOAT32 ) * arrWeights.getValuesCount();
		BufferDesc.CPUAccessFlags = 0;
		BufferDesc.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = arrWeights.getFloatValues()->getData();
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;
		g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
		skin->SkeletalBuffers[ SBS_Weight ] = pBuffer;

		// Bind matrix buffer
		BufferDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		BufferDesc.ByteWidth = 4 * 16 * ( JCount + 1 );
		BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		BufferDesc.Usage = D3D10_USAGE_DYNAMIC;
		g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
		skin->SkeletalBuffers[ SBS_Bind ] = pBuffer;

		// Per vertex joints info
		BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		BufferDesc.ByteWidth = sizeof( UINT32 ) * 2 * VCount;
		BufferDesc.CPUAccessFlags = 0;
		BufferDesc.Usage = D3D10_USAGE_DEFAULT;
		UINT32* pVB4Joints = new UINT32[ 2 * VCount ];
		for( UINT32 i = 0; i < VCount; i++ )
		{
			pVB4Joints[ 2 * i ] = arrJPV[ i ];
			pVB4Joints[ 2 * i + 1 ] = arrJointsOffset[ i ];
		};
		InitData.pSysMem = ( void* )pVB4Joints;
		g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
		skin->SkeletalBuffers[ SBS_VertexJoints ] = pBuffer;

		// Joints index
		BufferDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		BufferDesc.ByteWidth = sizeof( UINT32 ) * 2 * JIndexCount;
		BufferDesc.CPUAccessFlags = 0;
		BufferDesc.Usage = D3D10_USAGE_DEFAULT;
		UINT32* pJointIndex = new UINT32[ 2 * JIndexCount ];
		for( UINT32 i = 0; i < JIndexCount; i++ )
		{
			pJointIndex[ 2 * i ] = arrJI[ i ] + 1;
			pJointIndex[ 2 * i + 1 ] = arrWI[ i ];
		};
		InitData.pSysMem = ( void* )pJointIndex;
		g_Renderer.pDevice->CreateBuffer( &BufferDesc, &InitData, &pBuffer );
		skin->SkeletalBuffers[ SBS_Joints ] = pBuffer;

		// Inverse bind matrix
		D3DXMATRIX m;
		for( UNIT32 i = 0; i < arrIBMs.getCount(); i++ )
		{
			CopyMatrixToDirectX( m, arrIBMs[ i ] );
			skin->Inv_Bind_Matrix.push_back( m );
		};

		return true;
	};

	bool Importer::writeController( const COLLADAFW::Controller* controller )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		if( controller->getControllerType() == COLLADAFW::Controller::CONTROLLER_TYPE_SKIN )
		{
			COLLADAFW::SkinController* skincontroller = ( COLLADAFW::SkinController* )controller;
			SkeletalGeometry* skel = m_pRFC->CreateNewSkeletalGeometry();
			mapSkeletalControllers.insert( std::pair< COLLADAFW::UniqueId, SkeletalGeometry* >( skincontroller->getUniqueId(), skel );

			mapSkeletalMesh2Geometry.insert( std::pair< SkeletalGeometry*, COLLADAFW::UniqueId >( skel, skincontroller->getSource() ) );
			mapSkeletalMesh2SkinData.insert( std::pair< SkeletalGeometry*, COLLADAFW::UniqueId >( skel, skincontroller->getSkinControllerData() ) );
			mapSkeletalMesh2Joints.insert( std::pair< SkeletalGeometry*, COLLADAFW::UniqueIdArray >( skel, skincontroller->getJoints() ) );
		};

		return true;
	};

    bool Importer::writeFormulas( const COLLADAFW::Formulas* formulas )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		return true;
	};

	bool Importer::writeKinematicsScene( const COLLADAFW::KinematicsScene* kinematicsScene )
	{
		if( mRun != IR_RESOURCE )
		{
			return true;
		};

		return true;
	};

};