#ifndef _TRE_RESOURCEMANAGER_H__
#define _TRE_RESOURCEMANAGER_H__

#include "Platform.h"
#include <string>

namespace TRE
{
	class Light
	{
	public:

		enum LightType
		{
			LT_POINT,
			LT_DIRECTIONAL,
		};

		std::string sName;
		
		LightType eType;
		
		D3DXCOLOR cColor;

		Node* Target;

		D3DXMATRIX& GetProjectionMatrix();
	};

	struct Camera
	{
		enum CameraType
		{
			CT_PERSPECTIVE,
		};

		std::string sName;
		
		CameraType eType;
		
		FLOAT32 fNearClip;
		
		FLOAT32 fFarClip;
		
		/**
		* Radian
		*/
		FLOAT32 fFOVY;

		FLOAT32 fAspect;
	};

	struct PhongParam
	{
		D3DXCOLOR cEmission;
		
		D3DXCOLOR cAmbient;
		
		D3DXCOLOR cDiffuse;
		
		D3DXCOLOR cSpecular;
		
		FLOAT32 fShininess;
		
		D3DXCOLOR cReflective;
		
		FLOAT32 fReflectivity;
		
		D3DXCOLOR cTransparent;
		
		FLOAT32 fTransparency;
		
		FLOAT32 fIndexOfRefraction;
		
		ID3D10ShaderResourceView* pDiffuse;

		std::string DiffuseTexPath;
	};

	class Mesh
	{
	public:

		enum BufferSemantic
		{
			BS_POSITION,
		
			BS_NORMAL,

			BS_TEXCOORD0,

			BS_TANGENT,

			BS_BINORMAL,

			BS_INFLUENCE,

			BS_INDEX,

			BS_END,
		};

		ID3D10Buffer* Buffers[BS_END];

		FLOAT32* RawBuffers[ BS_END ];

		UINT32* RawIndex;

		// Buffer strides in bytes
		UINT32	BufferStrides[BS_END];

		// Number of vertex in the vertex buffer
		UINT32 VertexBufferCount;

		UINT32 IndexStartLocation;

		UINT32 IndexCount;

		std::string MaterialSymbol;

		void SetBuffersToDevice();
	};

	struct Geometry
	{
		std::vector< Mesh* > vMesh;

		std::string Name;
	};

	struct Material
	{
		std::vector< PhongParam* > vParams;
	};

	struct StaticGeometry
	{
		Geometry* pGeometry;

		Material* pMaterial;
	};

	struct Skeleton
	{
		std::vector< Node* > Joints;

		std::vector< std::string > JointNames;

		Node* SkeletonRoot;
	};

	struct Skin
	{		
		enum SkeletalBufferSemantic
		{
			/**
			* Collada use -1 in "v" element under "vertex_weights" to refer to bind shape matrix
			* We store bind shape matrix as the first matrix in the bind buffer and the original joint index are all added by 1
			*
			* Layout:
			*		[ Bind_Shape ][ Bind0 ][ Bind1 ]...
			*/
			SBS_Bind,

			SBS_Weight,

			/**
			* Per vertex joints info.
			* Layout:
			*		[ JointCount ][ JointOffset ]
			*/
			SBS_VertexJoints,

			/**
			* Store index to the bind buffer and weight buffer
			* Layout:
			*		[ Index2Joint ][Index2Weight ]
			*/
			SBS_Joints,

			SBS_End,
		};

		ID3D10Buffer* SkeletalBuffers[ SBS_End ];

		UINT32 SkeletalBufferStrides[ SBS_End ];

		ID3D10ShaderResourceView* SkeletalBufferSRVs[ SBS_End ];

		std::vector< D3DXMATRIX > Inv_Bind_Matrix;

		D3DXMATRIX Bind_Shape_Matrix;

	};

	struct SkeletalGeometry
	{
		Skin* SkinData;

		StaticGeometry* Mesh;

		Skeleton* Skel;
	};

	class Element
	{
	public:
		enum ElementType
		{
			ET_StaticMesh,

			ET_SkeletalMesh,

			ET_Camera,

			ET_Light,

			ET_END,
		};

		ElementType eType;

		union
		{
			StaticGeometry* pStaticMesh;

			SkeletalGeometry* pSkeletalMesh;

			Camera* pCamera;

			Light* pLight;
		};

		Node* pNode;
	};

	class Node
	{
	public:

		D3DXMATRIX mLocalTransform;

		D3DXMATRIX mAccumulatedTransform;

		Node* pParent;

		std::string sName;

		std::set< Node* > setChildren;

		std::set< Element* > setElements;

		void PropagateTransform();

		void UpdateTransform();

		void UpdateAndPropagateTransform();

		D3DXMATRIX& GetWorldMatrix();

		void GetLookAtMatrix( Node* n, D3DXMATRIX& m );
	};

	struct Scene
	{
		Node* pRootNode;
	};
	
	template< typename Input, typename Output >
	class AnimationSampler
	{
	public:

		AnimationSampler() : m_InputCurrentIndex( 0 ) {};

		bool GetOutput( const Input& i, Output& out );

		Output Interpolate( Output& v0, Output& v1, FLOAT32 s );

		struct Tangent
		{
			Input i;

			Output o;
		};

		enum AnimationSamplerInterpolationType
		{
			ASTT_Linear,

			ASTT_Bezier,

			ASTT_Hermite,

			ASTT_Step,
		};

		std::vector< Input > m_Inputs;

		std::vector< Output > m_Outputs;

		std::vector< Tangent > m_InTangents;

		std::vector< Tangent > m_OutTangents;

		AnimationSamplerInterpolationType eType;

		UINT32 m_InputCurrentIndex;
	};

	class Animation
	{
	public:

		Animation() : m_fCurrentTime( 0 ), m_fEndTime( FLT_MIN ), m_fBeginTime( FLT_MAX ), eAPT( APT_Loop ) {};

		void Advance( const FLOAT32 delta );

		AnimationSampler< FLOAT32, D3DXMATRIX >& AddSampler();

		std::vector< Animation* > m_vChildAnims;

		std::vector< AnimationSampler< FLOAT32, D3DXMATRIX > > m_vSamplers;

		std::vector< std::vector< D3DXMATRIX* > > m_vSamplerTargets;

		enum AnimationPlaybackType
		{
			APT_Loop,
		};

		AnimationPlaybackType eAPT;

		FLOAT32 m_fCurrentTime;

		FLOAT32 m_fBeginTime;

		FLOAT32 m_fEndTime;
	};

	struct ResourceDatabase
	{
		std::set< Scene* > setScene;

		std::set< Node* > setNodes;

		std::set< Light* > setLights;

		std::set< Camera* > setCameras;

		std::set< Animation* > setAnimations;

		std::set< Mesh* > setMeshes;

		std::set< StaticGeometry* > setStaticGeometries;

		std::set< SkeletalGeometry* > setSkeletalGeometries;

		std::set< Skin* > setSkins;

		std::set< Skeleton* > setSkeletons;

		std::set< Element* > setElements;

		std::set< PhongParam* > setPhongParams;

		std::set< Geometry* > setGeometries;

		std::set< Material* > setMaterials;

		std::set< Element* > setSceneCameras;

		std::set< Element* > setSceneLights;

		std::set< Element* > setSceneStaticGeometries;

		std::set< Element* > setSceneSkeletalGeometries;
	};
};

#endif