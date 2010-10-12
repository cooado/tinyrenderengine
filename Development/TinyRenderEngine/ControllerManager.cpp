#include "Prerequisites.h"
#include "OIS.h"

namespace TRE
{

	ResourceDatabase Controller::m_ResourceDB;

	INT32 ControllerManager::UpdateControllers( FLOAT64 fTime, FLOAT32 fElapsed )
	{
		m_pTC->Update( fTime, fElapsed );
		m_pAC->UpdateAnimations( fElapsed );
		m_pIC->Update();

		return 0;
	};


	INT32 ControllerManager::Initialize()
	{
		m_pRFC = new ResourceFactoryController();
		m_pRRC = new ResourceRetrieveController();
		m_pCC = new CameraController();
		m_pAC = new AnimationController();
		m_pIC = new InputController();
		m_pRCC = new RayCastController();
		m_pTC = new TimeController();
		m_pLC = new LightController();
		m_pRFC->Initialize();
		m_pRRC->Initialize();
		m_pCC->Initialize();
		m_pAC->Initialize();
		m_pIC->Initialize();
		m_pRCC->Initialize();
		m_pTC->Initialize();
		m_pLC->Initialize();

		m_pIC->AppendListener( new DefaultMouseListener() );
		m_pIC->AppendListener( new DefaultKeyboardListener() );

		return 0;
	};

	INT32 ControllerManager::Finalize()
	{
		m_pRFC->Finalize();
		m_pRRC->Finalize();
		m_pCC->Finalize();
		m_pAC->Finalize();
		m_pIC->Finalize();
		m_pRCC->Finalize();
		m_pTC->Finalize();
		m_pLC->Finalize();
		delete m_pRFC;
		delete m_pRRC;
		delete m_pCC;
		delete m_pAC;
		delete m_pIC;
		delete m_pRCC;
		delete m_pTC;
		delete m_pLC;

		return 0;
	};

	INT32 ResourceFactoryController::Initialize()
	{
		return 0;
	};

	INT32 ResourceFactoryController::Finalize()
	{
		std::set< ID3D10Buffer* > setBuffers;
		std::set< ID3D10ShaderResourceView* > setSRVs;
		std::set< ID3D10Resource* > setResources;

		// Gather DirectX objects for destroying when app ends
		for( std::set< Mesh* >::iterator i = m_ResourceDB.setMeshes.begin(); i!= m_ResourceDB.setMeshes.end(); i++ )
		{
			Mesh* m = *i;
			for( UINT32 ib = 0; ib < Mesh::BS_END; ib++ )
			{
				if( m->Buffers[ ib ] != NULL )
				{
					setBuffers.insert( m->Buffers[ ib ] );
				};
			};
		};

		for( std::set< PhongParam* >::iterator i = m_ResourceDB.setPhongParams.begin(); i != m_ResourceDB.setPhongParams.end(); i++ )
		{
			PhongParam* p = *i;
			if( p->pDiffuse != NULL )
			{
				ID3D10Resource* resource = NULL;
				p->pDiffuse->GetResource( &resource );
				setResources.insert( resource );
				setSRVs.insert( p->pDiffuse );
			};
		};

		for( std::set< Skin* >::iterator i = m_ResourceDB.setSkins.begin(); i != m_ResourceDB.setSkins.end(); i++ )
		{
			Skin* s = *i;
			for( UINT32 ib = 0; ib < Skin::SBS_End; ib++ )
			{
				if( s->SkeletalBuffers[ ib ] != NULL )
				{
					setBuffers.insert( s->SkeletalBuffers[ ib ] );
				};
				if( s->SkeletalBufferSRVs[ ib ] != NULL )
				{
					ID3D10Resource* resource = NULL;
					s->SkeletalBufferSRVs[ ib ]->GetResource( &resource );
					setResources.insert( resource );
					setSRVs.insert( s->SkeletalBufferSRVs[ ib ] );
				};
			};
		};

		UINT32 rc;
		
		for( std::set< ID3D10Buffer* >::iterator i = setBuffers.begin(); i!= setBuffers.end(); i++ )
		{
			rc = ( *i )->Release();
		};

		for( std::set< ID3D10Resource* >::iterator i = setResources.begin(); i != setResources.end(); i++ )
		{
			rc = ( *i )->Release();
		};

		for( std::set< ID3D10ShaderResourceView* >::iterator i = setSRVs.begin(); i != setSRVs.end(); i++ )
		{
			rc = ( *i )->Release();
		};

		return 0;
	};

	Node* ResourceFactoryController::CreateNewNode()
	{
		Node* node = new Node;
		m_ResourceDB.setNodes.insert( node );
		return node;
	};

	Scene* ResourceFactoryController::CreateNewScene()
	{
		Scene* scene = new Scene;
		m_ResourceDB.setScene.insert( scene );
		return scene;
	};

	Light* ResourceFactoryController::CreateNewLight()
	{
		Light* light = new Light;
		m_ResourceDB.setLights.insert( light );
		return light;
	};

	Camera* ResourceFactoryController::CreateNewCamera()
	{
		Camera* camera = new Camera;
		m_ResourceDB.setCameras.insert( camera );
		return camera;
	};

	Element* ResourceFactoryController::CreateNewElement()
	{
		Element* element = new Element;
		m_ResourceDB.setElements.insert( element );
		return element;
	};

	StaticGeometry* ResourceFactoryController::CreateNewStaticGeometry()
	{
		StaticGeometry* sm = new StaticGeometry;
		m_ResourceDB.setStaticGeometries.insert( sm );
		return sm;
	};

	SkeletalGeometry* ResourceFactoryController::CreateNewSkeletalGeometry()
	{
		SkeletalGeometry* skelm = new SkeletalGeometry;
		skelm->Skel = new Skeleton;
		m_ResourceDB.setSkeletalGeometries.insert( skelm );
		return skelm;
	};

	Skin* ResourceFactoryController::CreateNewSkin()
	{
		Skin* skin = new Skin;
		m_ResourceDB.setSkins.insert( skin );
		return skin;
	};

	Skeleton* ResourceFactoryController::CreateNewSkeleton()
	{
		Skeleton* skel = new Skeleton;
		m_ResourceDB.setSkeletons.insert( skel );
		return skel;
	};

	Mesh* ResourceFactoryController::CreateNewMesh()
	{
		Mesh* mesh = new Mesh;
		m_ResourceDB.setMeshes.insert( mesh );
		return mesh;
	};

	Animation* ResourceFactoryController::CreateNewAnimation()
	{
		Animation* anim = new Animation;
		m_ResourceDB.setAnimations.insert( anim );
		return anim;
	};

	PhongParam* ResourceFactoryController::CreateNewPhongParam()
	{
		PhongParam* param = new PhongParam;
		m_ResourceDB.setPhongParams.insert( param );
		return param;
	};

	Geometry* ResourceFactoryController::CreateNewGeometry()
	{
		Geometry* geom = new Geometry;
		m_ResourceDB.setGeometries.insert( geom );
		return geom;
	};

	Material* ResourceFactoryController::CreateNewMaterial()
	{
		Material* mat = new Material;
		m_ResourceDB.setMaterials.insert( mat );
		return mat;
	};

	void ResourceFactoryController::EndAddNewResource()
	{
		// Grouping elements
		for( std::set< Element* >::const_iterator iE = m_ResourceDB.setElements.begin(); iE != m_ResourceDB.setElements.end(); iE++ )
		{
			if( ( *iE )->eType == Element::ET_SkeletalMesh )
			{
				m_ResourceDB.setSceneSkeletalGeometries.insert( *iE );
			}
			else if( ( *iE )->eType == Element::ET_StaticMesh )
			{
				m_ResourceDB.setSceneStaticGeometries.insert( *iE );
			}
			else if( ( *iE )->eType == Element::ET_Camera )
			{
				m_ResourceDB.setSceneCameras.insert( *iE );
			}
			else if( ( *iE )->eType == Element::ET_Light )
			{
				m_ResourceDB.setSceneLights.insert( *iE );
			};
		};

		// Update node transform
		Scene* s = *m_ResourceDB.setScene.begin();
		s->pRootNode->UpdateAndPropagateTransform();

	};

	/**********************************************************************************
	* ResourceRetrieveController
	***********************************************************************************/

	INT32 ResourceRetrieveController::Initialize()
	{
		return 0;
	};

	INT32 ResourceRetrieveController::Finalize()
	{
		return 0;
	};

	const std::set< Element* >& ResourceRetrieveController::GetSceneStaticGeometries()
	{
		return m_ResourceDB.setSceneStaticGeometries;
	};

	const std::set< Element* >& ResourceRetrieveController::GetSceneSkeletalGeometries()
	{
		return m_ResourceDB.setSceneSkeletalGeometries;
	};

	const std::set< Geometry* >& ResourceRetrieveController::GetGeometries()
	{
		return m_ResourceDB.setGeometries;
	};

	Geometry* ResourceRetrieveController::GetGeometryByName( std::string name )
	{
		for( std::set< Geometry* >::iterator i = m_ResourceDB.setGeometries.begin(); i != m_ResourceDB.setGeometries.end(); i++ )
		{
			if( ( *i )->Name == name )
			{
				return ( *i );
			};
		};
		return NULL;
	};

	Node* ResourceRetrieveController::GetNodeByName( std::string name )
	{
		for( std::set< Node* >::iterator iN = m_ResourceDB.setNodes.begin(); iN != m_ResourceDB.setNodes.end(); iN++ )
		{
			if( ( *iN )->sName == name )
			{
				return ( *iN );
			};
		};
		return NULL;
	};

	Light* ResourceRetrieveController::GetLightByName(std::string name)
	{
		for( std::set< Light* >::iterator iL = m_ResourceDB.setLights.begin(); iL != m_ResourceDB.setLights.end(); iL++ )
		{
			if( ( *iL )->sName == name )
			{
				return ( *iL );
			};
		};
		return NULL;
	};

	/********************************************************************************************
	* LightController
	********************************************************************************************/

	INT32 LightController::Initialize()
	{
		return 0;
	};

	INT32 LightController::Finalize()
	{
		return 0;
	};

	bool LightController::GetWorldPos( std::string name, D3DXVECTOR3& pos )
	{
		Element* l = NULL;
		for( std::set< Element* >::iterator i = m_ResourceDB.setSceneLights.begin(); i != m_ResourceDB.setSceneLights.end(); i++ )
		{
			if( ( *i )->pLight->sName == name )
			{
				l = ( *i );
				break;
			};
		};
		if( l != NULL && l->pNode != NULL )
		{
			pos.x = l->pNode->mAccumulatedTransform._41;
			pos.y = l->pNode->mAccumulatedTransform._42;
			pos.z = l->pNode->mAccumulatedTransform._43;
			return true;
		}
		else
		{
			return false;
		};
	};

	bool LightController::GetViewMatrix( std::string name, D3DXMATRIX& m )
	{
		Element* l = NULL;
		for( std::set< Element* >::iterator i = m_ResourceDB.setSceneLights.begin(); i != m_ResourceDB.setSceneLights.end(); i++ )
		{
			if( ( *i )->pLight->sName == name )
			{
				l = ( *i );
				break;
			};
		};
		if( l != NULL && l->pNode != NULL )
		{
			l->pNode->GetLookAtMatrix( l->pLight->Target, m );
			return true;
		}
		else
		{
			D3DXMatrixIdentity( &m );
			return false;
		};
	};

	bool LightController::GetProjectionMatrix( std::string name, D3DXMATRIX& m )
	{
		D3DXMatrixPerspectiveFovLH( &m, (FLOAT32)D3DX_PI / 2, 1.0f, 1.0f, 1000.0f );
		return true;
	};

	/***************************************************************************************************
	* CameraController
	***************************************************************************************************/

	INT32 CameraController::Initialize()
	{
		eState = CCS_Imported;
		m_pImportedNode = NULL;
		m_pCameraLocalNode = NULL;
		m_pCameraPitchNode = NULL;

		return 0;
	};

	INT32 CameraController::Finalize()
	{
		return 0;
	};

	D3DXMATRIX& CameraController::GetViewMatrix()
	{
		Element* elem = *m_ResourceDB.setSceneCameras.begin();
		D3DXMatrixInverse( &m_View, NULL, &elem->pNode->mAccumulatedTransform );
		return m_View;
	};

	D3DXMATRIX& CameraController::GetProjectionMatrix()
	{
		Element* elem = *m_ResourceDB.setSceneCameras.begin();
		Camera* cam = elem->pCamera ;
		cam->fFOVY = (FLOAT32)D3DXToRadian( 60.0f );
		D3DXMatrixPerspectiveFovLH( &m_Projection, cam->fFOVY, 1.0f, cam->fNearClip, cam->fFarClip ); 
		return m_Projection;
	};

	void CameraController::SetCameraControllerState(TRE::CameraController::CameraControllerState state)
	{
		eState = state;
		if( eState == CCS_Imported )
		{
			Element* elem = *m_ResourceDB.setSceneCameras.begin();
			elem->pNode = m_pImportedNode;
		}
		else if( eState == CCS_UserInput )
		{
			if( m_pCameraLocalNode == NULL )
			{
				Element* elem = *m_ResourceDB.setSceneCameras.begin();
				m_pImportedNode = elem->pNode;

				m_pCameraLocalNode = ControllerManager::GetInstance()->m_pRFC->CreateNewNode();
				m_pCameraLocalNode->pParent = ( *m_ResourceDB.setScene.begin() )->pRootNode;
				m_pCameraPitchNode = ControllerManager::GetInstance()->m_pRFC->CreateNewNode();
				m_pCameraPitchNode->pParent = m_pCameraLocalNode;
				m_pCameraLocalNode->setChildren.insert( m_pCameraPitchNode );
				D3DXVECTOR3 scale;
				D3DXQUATERNION quat;
				D3DXVECTOR3 translate;
				D3DXMatrixDecompose( &scale, &quat, &translate, &elem->pNode->mAccumulatedTransform );
				quat.z = 0;
				quat.x = 0;
				D3DXMATRIX mat1, mat2;
				D3DXMatrixRotationQuaternion( &mat1, &quat );
				D3DXMatrixTranslation( &mat2, translate.x, translate.y, translate.z );
				D3DXMatrixMultiply( &m_pCameraLocalNode->mLocalTransform, &mat1, &mat2 );;
				D3DXMatrixIdentity( &m_pCameraPitchNode->mLocalTransform );
				D3DXMatrixIdentity( &m_pCameraPitchNode->mAccumulatedTransform );
				
				m_pCameraLocalNode->UpdateAndPropagateTransform();
			};

			Element* elem = *m_ResourceDB.setSceneCameras.begin();
			elem->pNode = m_pCameraPitchNode;
		};
	};

	void CameraController::Move(const D3DXVECTOR3 &v)
	{
		if( eState == CameraController::CCS_Imported )
		{
			return;
		};

		D3DXMATRIX mat;
		D3DXMatrixTranslation( &mat, v.x, v.y, v.z );
		D3DXMatrixMultiply( &m_pCameraLocalNode->mLocalTransform, &mat, &m_pCameraLocalNode->mLocalTransform );
		m_pCameraLocalNode->UpdateAndPropagateTransform();
	};

	void CameraController::AddYaw(const TRE::FLOAT32 y)
	{
		if( eState == CameraController::CCS_Imported )
		{
			return;
		};

		D3DXMATRIX mat;
		D3DXMatrixRotationY( &mat, y );
		D3DXMatrixMultiply( &m_pCameraLocalNode->mLocalTransform, &mat, &m_pCameraLocalNode->mLocalTransform );
		m_pCameraLocalNode->UpdateAndPropagateTransform();		
	};

	void CameraController::AddPitch(const TRE::FLOAT32 p)
	{
		if( eState == CameraController::CCS_Imported )
		{
			return;
		};

		static FLOAT32 CameraPitchMax = (FLOAT32)D3DX_PI / 3;
		static FLOAT32 CameraPitchMin = -1.0f * (FLOAT32)D3DX_PI / 3;
		static FLOAT32 Pitch = 0;

		Pitch += p;
		Pitch < CameraPitchMin ? Pitch = CameraPitchMin : ( Pitch > CameraPitchMax ? Pitch = CameraPitchMax : Pitch = Pitch );
		D3DXMATRIX mat;
		D3DXMatrixRotationX( &mat, Pitch );
		m_pCameraPitchNode->mLocalTransform = mat;
		m_pCameraPitchNode->UpdateTransform();
	};

	/******************************************************************************************
	* AnimationController
	*******************************************************************************************/

	INT32 AnimationController::Initialize()
	{ 
		return 0;
	};

	INT32 AnimationController::Finalize()
	{
		return 0;
	};

	void AnimationController::UpdateAnimations( FLOAT32 fElapsed )
	{
		// Update animations
		for( std::set< Animation* >::iterator i = m_ResourceDB.setAnimations.begin(); i != m_ResourceDB.setAnimations.end(); i++ )
		{
			( *i )->Advance( fElapsed );
		};

		// Update skeleton buffers
		for( std::set< SkeletalGeometry* >::iterator i = m_ResourceDB.setSkeletalGeometries.begin(); i != m_ResourceDB.setSkeletalGeometries.end(); i++ )
		{
			HRESULT result;
			Skeleton* skel = ( *i )->Skel;
			Skin* skin = ( *i )->SkinData;
			FLOAT32* pBind = NULL;
			skel->SkeletonRoot->UpdateAndPropagateTransform();
			result = skin->SkeletalBuffers[ Skin::SBS_Bind ]->Map( D3D10_MAP_WRITE_DISCARD, 0, ( void** )&pBind );
			memcpy( ( void* )pBind, ( void* )( &( skin->Bind_Shape_Matrix ) ), sizeof( FLOAT32 ) * 16 );
			for( UINT32 iBind = 0; iBind < skin->Inv_Bind_Matrix.size(); iBind++ )
			{
				D3DXMATRIX m;
				D3DXMatrixMultiply( &m, &skin->Bind_Shape_Matrix, &skin->Inv_Bind_Matrix[ iBind ] );
				D3DXMatrixMultiply( &m, &m, &skel->Joints[ iBind ]->mAccumulatedTransform );
				memcpy( ( void* )( pBind + 16 * ( iBind + 1 ) ), ( void* )&m, sizeof( FLOAT32 ) * 16 );
			};
			skin->SkeletalBuffers[ Skin::SBS_Bind ]->Unmap();
		};
	};

	/********************************************************************************************
	* RayCastController
	********************************************************************************************/

	INT32 RayCastController::Initialize()
	{
		return 0;
	};

	INT32 RayCastController::Finalize()
	{
		return 0;
	};

	void RayCastController::AddGeometry(TRE::Element *elem)
	{
		// Only static geometry with one mesh is supported for now
		if( elem->eType == Element::ET_StaticMesh )
		{
			m_vElements.push_back( elem );
			Mesh* m = elem->pStaticMesh->pGeometry->vMesh[ 0 ];
			UINT32 nVertices = m->VertexBufferCount;
			UINT32 nFaces = m->IndexCount / 3;
			UINT32 nStride = sizeof( FLOAT32 ) * 3;
			TinyRT::StridedMesh< UINT32 > mesh( (const TinyRT::uint8*) m->RawBuffers[ Mesh::BS_POSITION ], m->RawIndex, nVertices, nFaces, nStride );

			TinyRT::SahAABBTreeBuilder< TinyRT::StridedMesh< UINT32 > > builder( 0.7f );
			m_BVH.Build( &mesh, builder );
		};
	};

	/**
	*	width / NearClip = tan( fovx / 2 )
	*	height / NearClip = tan( fovy / 2 )
	*	====>
	*	ViewSpace Direction: dir = ( width * clipspacex, height * clipspacey, NearClip, 0 )
	*	====>
	*	dir = ( tan( fovx / 2 ) * clipspacex, tan( fovy / 2 ) * clipspacey, 1, 0 )
	*	tan( fovx / 2 ) / tan( fovy / 2 ) = aspectratio
	*	====>
	*	dir = ( aspectratio * tan( fovy / 2 ) * clipspacex, tan( fovy / 2 ) * clipspacey, 1, 0 )
	*/
	bool RayCastController::RayTestScreenSpace(const TRE::UINT32 x, const TRE::UINT32 y, TRE::RayCastController::RayTestResult &result)
	{
		Element* elem = *m_ResourceDB.setSceneCameras.begin();
		FLOAT32 fovy = elem->pCamera->fFOVY;
		FLOAT32 aspectratio = elem->pCamera->fAspect;
		INT32 screenwidth, screenheight;
		TinyRenderEngine::GetInstance()->GetScreenResolution( screenwidth, screenheight );
		FLOAT32 clipspacex = ( FLOAT32 )x / ( FLOAT32 )screenwidth * 2.0f - 1.0f;
		FLOAT32 clipspacey = ( 1.0f - ( FLOAT32 )y / ( FLOAT32 )screenheight ) * 2.0f - 1.0f;
		D3DXVECTOR3 Origin = D3DXVECTOR3( 0, 0, 0 );
		D3DXVECTOR3 Direction = D3DXVECTOR3( aspectratio * tan( fovy / 2 ) * clipspacex, tan( fovy / 2 ) * clipspacey, 1 );
		return RayTestViewSpace( Origin, Direction, result );
	};

	bool RayCastController::RayTestViewSpace(const D3DXVECTOR3 &o, const D3DXVECTOR3 &d, TRE::RayCastController::RayTestResult &result)
	{
		if( m_vElements.size() == 0 )
		{
			return false;
		};

		Mesh* m = m_vElements[ 0 ]->pStaticMesh->pGeometry->vMesh[ 0 ];
		Node* node = m_vElements[ 0 ]->pNode;

		// Calculate ray origin and direction in object space
		D3DXVECTOR4 origin( o.x, o.y, o.z, 1 );
		D3DXVECTOR4 direction( d.x, d.y, d.z, 0 );
		D3DXMATRIX world = node->GetWorldMatrix();
		D3DXMATRIX view = TRE::ControllerManager::GetInstance()->m_pCC->GetViewMatrix();
		D3DXMATRIX proj = TRE::ControllerManager::GetInstance()->m_pCC->GetProjectionMatrix();
		D3DXMATRIX mat;
		D3DXMatrixMultiply( &mat, &world, &view );
		D3DXMatrixInverse( &mat, NULL, &mat );
		origin = VectorMatrixMultiply( origin, mat );
		direction = VectorMatrixMultiply( direction, mat );

		// Construct TinyRT mesh for ray casting
		UINT32 nVertices = m->VertexBufferCount;
		UINT32 nFaces = m->IndexCount / 3;
		UINT32 nStride = sizeof( FLOAT32 ) * 3;
		TinyRT::StridedMesh< UINT32 > mesh( (const TinyRT::uint8*)m->RawBuffers[ Mesh::BS_POSITION ], m->RawIndex, nVertices, nFaces, nStride );

		// cast a ray
		TinyRT::Ray ray( &origin.x, &direction.x );
	    
		TinyRT::TriangleRayHit triHit;
		triHit.nTriIdx = 0xffffffff;
		TinyRT::RaycastBVH( &m_BVH, &mesh, ray, triHit, m_BVH.GetRoot(), m_TRTScratch );

		if( triHit.nTriIdx != 0xffffffff )
		{
			UINT32 face = triHit.nTriIdx;
			FLOAT32 B0 = triHit.vUVCoords[0];
			FLOAT32 B1 = triHit.vUVCoords[1];
			FLOAT32 B2 = 1.0f - B0 - B1;
			UINT32 v0 = m->RawIndex[ m->IndexStartLocation + face * 3 ];
			UINT32 v1 = m->RawIndex[ m->IndexStartLocation + face * 3 + 1];
			UINT32 v2 = m->RawIndex[ m->IndexStartLocation + face * 3 + 2 ];
			D3DXVECTOR3 pos0;
			pos0.x = *( m->RawBuffers[ Mesh::BS_POSITION ] + v0 * 3 );
			pos0.y = *( m->RawBuffers[ Mesh::BS_POSITION ] + v0 * 3 + 1 );
			pos0.z = *( m->RawBuffers[ Mesh::BS_POSITION ] + v0 * 3 + 2 );
			D3DXVECTOR3 pos1;
			pos1.x = *( m->RawBuffers[ Mesh::BS_POSITION ] + v1 * 3 );
			pos1.y = *( m->RawBuffers[ Mesh::BS_POSITION ] + v1 * 3 + 1 );
			pos1.z = *( m->RawBuffers[ Mesh::BS_POSITION ] + v1 * 3 + 2 );
			D3DXVECTOR3 pos2;
			pos2.x = *( m->RawBuffers[ Mesh::BS_POSITION ] + v2 * 3 );
			pos2.y = *( m->RawBuffers[ Mesh::BS_POSITION ] + v2 * 3 + 1 );
			pos2.z = *( m->RawBuffers[ Mesh::BS_POSITION ] + v2 * 3 + 2 );
			D3DXVECTOR3 hitpoint = B1 * pos1 + B2 * pos2 + B0 * pos0;

			result.FaceIndex = face;
			result.BaryCentric1 = B1;
			result.BaryCentric2 = B2;
			result.HitPoint = hitpoint;

			return true;
		}
		else
		{
			return false;
		};
	};

	/****************************************************************************************
	* DefaultMouseListener
	*****************************************************************************************/

	void DefaultMouseListener::OnMouse( const OIS::Mouse* m )
	{
		/// Radian per pixel
		static FLOAT32 CameraYawSpeed = (FLOAT32)D3DX_PI / 800.0f;
		static FLOAT32 CameraPitchSpeed = (FLOAT32)D3DX_PI / 800.0f;

		const OIS::MouseState& state = m->getMouseState();
		if( state.buttonDown( OIS::MB_Left ) )
		{
			// Yaw
			ControllerManager::GetInstance()->m_pCC->AddYaw( state.X.rel * CameraYawSpeed );
			
			// Pitch
			ControllerManager::GetInstance()->m_pCC->AddPitch( state.Y.rel * CameraPitchSpeed );
		};
	};

	/***************************************************************************************
	* DefaultKeyboardListener
	***************************************************************************************/

	void DefaultKeyboardListener::OnKeyboard( OIS::Keyboard* kb )
	{
		static FLOAT32 CameraForwardSpeed = 50.0f;
		static FLOAT32 CameraBackwardSpeed = 50.0f;
		static FLOAT32 CameraStrafeLeftSpeed = 50.0f;
		static FLOAT32 CameraStrafeRightSpeed = 50.0f;
		static FLOAT32 CameraUpSpeed = 50.0f;
		static FLOAT32 CameraDownSpeed = 50.0f;

		if( kb->isKeyDown( OIS::KC_W ) )
		{
			// Forward
			D3DXVECTOR3 v( 0, 0, 0 );
			v.z = ControllerManager::GetInstance()->m_pTC->GetElapsed() * CameraForwardSpeed;
			//v.z = 10;
			ControllerManager::GetInstance()->m_pCC->Move( v );
		}
		else if( kb->isKeyDown( OIS::KC_S ) )
		{
			// Backward
			D3DXVECTOR3 v( 0, 0, 0 );
			v.z = -1.0f * ControllerManager::GetInstance()->m_pTC->GetElapsed() * CameraBackwardSpeed;
			//v.z = -10;
			ControllerManager::GetInstance()->m_pCC->Move( v );
		}
		else if( kb->isKeyDown( OIS::KC_A ) )
		{
			// Strafe left
			D3DXVECTOR3 v( 0, 0, 0 );
			v.x = -1.0f * ControllerManager::GetInstance()->m_pTC->GetElapsed() * CameraStrafeLeftSpeed;
			//v.x = -10;
			ControllerManager::GetInstance()->m_pCC->Move( v );
		}
		else if( kb->isKeyDown( OIS::KC_D ) )
		{
			// Strafe right
			D3DXVECTOR3 v( 0, 0, 0 );
			v.x = ControllerManager::GetInstance()->m_pTC->GetElapsed() * CameraStrafeRightSpeed;
			//v.x = 10;
			ControllerManager::GetInstance()->m_pCC->Move( v );
		}
		else if( kb->isKeyDown( OIS::KC_Q ) )
		{
			// Up
			D3DXVECTOR3 v( 0, 0, 0 );
			v.y = ControllerManager::GetInstance()->m_pTC->GetElapsed() * CameraUpSpeed;
			//v.y = 10;
			ControllerManager::GetInstance()->m_pCC->Move( v );
		}
		else if( kb->isKeyDown( OIS::KC_E ) )
		{
			// Down
			D3DXVECTOR3 v( 0, 0, 0 );
			v.y = -1.0f * ControllerManager::GetInstance()->m_pTC->GetElapsed() * CameraDownSpeed;
			//v.y = -10;
			ControllerManager::GetInstance()->m_pCC->Move( v );
		}
		else if( kb->isKeyDown( OIS::KC_C ) )
		{
			static FLOAT64 freeze = 0;
			if( !ControllerManager::GetInstance()->m_pTC->GetFreezeState( freeze ) )
			{
				freeze = ControllerManager::GetInstance()->m_pTC->StartFreeze();

				// Camera state
				static INT32 s = 0;
				if( s == 0 )
				{
					ControllerManager::GetInstance()->m_pCC->SetCameraControllerState( CameraController::CCS_UserInput );
					s = 1;
				}
				else if( s == 1 )
				{
					ControllerManager::GetInstance()->m_pCC->SetCameraControllerState( CameraController::CCS_Imported );
					s = 0;
				};
			};
		}
		else if ( kb->isKeyDown( OIS::KC_ESCAPE ))
		{
			PostMessage( g_Renderer.HWin, WM_QUIT, NULL, NULL );
		};
	};

	/***************************************************************************
	* TimeController
	***************************************************************************/

	INT32 TimeController::Initialize()
	{
		m_Time = 0;
		m_Elapsed = 0;

		return 0;
	};

	INT32 TimeController::Finalize()
	{
		return 0;
	};

	void TimeController::Update( FLOAT64 fTime, FLOAT32 fElapsed )
	{
		m_Time = fTime;
		m_Elapsed = fElapsed;
	};

	/******************************************************************************
	* InputController
	******************************************************************************/

	INT32 InputController::Initialize()
	{
		m_Manager = OIS::InputManager::createInputSystem( ( size_t )g_Renderer.HWin );
		m_Keyboard = static_cast< OIS::Keyboard* >( m_Manager->createInputObject( OIS::OISKeyboard, false ) );
		m_Mouse = static_cast< OIS::Mouse* >( m_Manager->createInputObject( OIS::OISMouse, false ) );

		INT32 width, height;
		TinyRenderEngine::GetInstance()->GetScreenResolution( width, height );
		const OIS::MouseState& state = m_Mouse->getMouseState();
		state.width = width;
		state.height = height;

		return 0;
	};

	INT32 InputController::Finalize()
	{
		m_Manager->destroyInputObject( m_Keyboard );
		m_Manager->destroyInputObject( m_Mouse );
		OIS::InputManager::destroyInputSystem( m_Manager );

		return 0;
	};

	void InputController::Update()
	{
		m_Keyboard->capture();
		m_Mouse->capture();

		for( std::vector< MouseListener* >::iterator i = m_vMouseListeners.begin(); i != m_vMouseListeners.end(); i++ )
		{
			( *i )->OnMouse( m_Mouse );
		};

		for( std::vector< KeyboardListener* >::iterator i = m_vKeyboardListeners.begin(); i != m_vKeyboardListeners.end(); i++ )
		{
			( *i )->OnKeyboard( m_Keyboard );
		};
	};
};