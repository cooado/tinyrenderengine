#ifndef _TRE_CONTROLLERMANAGER_H__
#define _TRE_CONTROLLERMANAGER_H__

#include "Platform.h"
#include <vector>
#undef min
#undef max
#include "TinyRT.h"
#include "OIS.h"

namespace TRE
{
	class Controller
	{
	public:

		virtual INT32 Initialize() = 0;

		virtual INT32 Finalize() = 0;

	protected:

		virtual ~Controller(){};

		static ResourceDatabase m_ResourceDB;
	};

	class KeyboardListener
	{
	public:
		virtual ~KeyboardListener() {};

		virtual void OnKeyboard( OIS::Keyboard* kb ) = 0;
	};

	class MouseListener
	{
	public:

		virtual ~MouseListener(){};

		virtual void OnMouse( const OIS::Mouse* m ) = 0;
	};

	class DefaultMouseListener : public MouseListener
	{
	public:

		void OnMouse( const OIS::Mouse* m );
	};

	class DefaultKeyboardListener : public KeyboardListener
	{
	public:
		
		void OnKeyboard( OIS::Keyboard* kb );
	};

	class InputController : public Controller
	{
	public:
		INT32 Initialize();

		INT32 Finalize();

		/**
		* Called once per frame to handle input events
		*/
		void Update();

		void AppendListener( MouseListener* ml )
		{
			m_vMouseListeners.push_back( ml );
		};

		void AppendListener( KeyboardListener* kl )
		{
			m_vKeyboardListeners.push_back( kl );
		};

	private:

		std::vector< KeyboardListener* > m_vKeyboardListeners;

		std::vector< MouseListener* > m_vMouseListeners;

		OIS::InputManager* m_Manager;

		OIS::Keyboard* m_Keyboard;

		OIS::Mouse* m_Mouse;
	};

	class LightController : public Controller
	{
	public:

		INT32 Initialize();

		INT32 Finalize();

		bool GetViewMatrix( std::string name, D3DXMATRIX& m );

		bool GetProjectionMatrix( std::string name, D3DXMATRIX& m );

		bool GetWorldPos( std::string name, D3DXVECTOR3& pos );
	};

	class CameraController : public Controller
	{
	public:

		INT32 Initialize();

		INT32 Finalize();

		D3DXMATRIX& GetViewMatrix();

		D3DXMATRIX& GetProjectionMatrix();

		/**
		*		Camera translation and yaw
		*					^
		*					|
		*				Camera pitch
		*
		* @param v Translation vector in camera's local space without pitch.
		*/
		void Move( const D3DXVECTOR3& v );

		/**
		* @param p Pitch in radian
		*/
		void AddPitch( const FLOAT32 p );

		/**
		* @param y Yaw in radian
		*/
		void AddYaw( const FLOAT32 y );

		enum CameraControllerState
		{
			CCS_Imported,

			CCS_UserInput,
		};

		void SetCameraControllerState( CameraControllerState state );

	private:

		D3DXMATRIX m_View;

		D3DXMATRIX m_Projection;

		CameraControllerState eState;

		Node* m_pImportedNode;

		Node* m_pCameraLocalNode;

		Node* m_pCameraPitchNode;
	};

	class ResourceFactoryController : public Controller
	{
	public:

		INT32 Initialize();

		INT32 Finalize();

		Node* CreateNewNode();

		Scene* CreateNewScene();

		Light* CreateNewLight();

		Camera* CreateNewCamera();

		Element* CreateNewElement();

		StaticGeometry* CreateNewStaticGeometry();

		SkeletalGeometry* CreateNewSkeletalGeometry();

		Skin* CreateNewSkin();

		Skeleton* CreateNewSkeleton();

		Mesh* CreateNewMesh();

		Animation* CreateNewAnimation();

		PhongParam* CreateNewPhongParam();

		Geometry* CreateNewGeometry();

		Material* CreateNewMaterial();

		void EndAddNewResource();

	private:


	};

	class ResourceRetrieveController : public Controller
	{
	public:
		INT32 Initialize();

		INT32 Finalize();

		const std::set< Element* >& GetSceneStaticGeometries();

		const std::set< Element* >& GetSceneSkeletalGeometries();

		const std::set< Geometry* >& GetGeometries();

		Geometry* GetGeometryByName( std::string name );

		Node* GetNodeByName( std::string name );

		Light* GetLightByName( std::string name );

	};

	class AnimationController : public Controller
	{
	public:
		INT32 Initialize();

		INT32 Finalize();

		void UpdateAnimations( FLOAT32 fElapsed );
	};


	class RayCastController : public Controller
	{
	public:
		struct RayTestResult
		{
			UINT32 FaceIndex;

			FLOAT32 BaryCentric1;

			FLOAT32 BaryCentric2;

			D3DXVECTOR3 HitPoint;
		};

		INT32 Initialize();

		INT32 Finalize();

		void AddGeometry( Element* elem );

		/**
		* @param o Ray origin in view space
		* @param d Ray direction in view space
		*
		* @return true if hit, false otherwise
		*/
		bool RayTestViewSpace( const D3DXVECTOR3& o, const D3DXVECTOR3& d, RayTestResult& result );

		bool RayTestScreenSpace( const UINT32 x, const UINT32 y, RayTestResult& result );

	private:
			
		TinyRT::AABBTree< TinyRT::StridedMesh< UINT32 > > m_BVH;

		TinyRT::ScratchMemory m_TRTScratch;

		std::vector< Element* > m_vElements;

	};

	class TimeController : public Controller
	{
	public:
		INT32 Initialize();

		INT32 Finalize();

		void Update( FLOAT64 fTime, FLOAT32 fElapsed );

		FLOAT64 GetTime() { return m_Time; };

		FLOAT32 GetElapsed() { return m_Elapsed; };

		FLOAT64 StartFreeze() { return m_Time; };

		/**
		* @return true if in freeze state, false otherwise
		*/
		bool GetFreezeState( FLOAT64& t ) 
		{ 
			static FLOAT64 FreezeDuration = 0.2f;
			FLOAT64 delta = m_Time - t;
			if( delta > FreezeDuration )
			{
				t = m_Time;
				return false;
			}
			else
			{
				return true;
			};
		};

	private:

		FLOAT64 m_Time;

		FLOAT32 m_Elapsed;
	};

	class ControllerManager
	{
		DECLARE_SINGLETON( ControllerManager )

	public:
		INT32 Initialize();

		INT32 Finalize();

		INT32 UpdateControllers( FLOAT64 fTime, FLOAT32 fElapsed );

		ResourceFactoryController* m_pRFC;

		ResourceRetrieveController* m_pRRC;

		CameraController* m_pCC;

		LightController* m_pLC;

		AnimationController* m_pAC;

		InputController* m_pIC;

		RayCastController* m_pRCC;

		TimeController* m_pTC;
	};
};

#endif