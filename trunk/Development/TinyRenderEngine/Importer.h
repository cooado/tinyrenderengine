#ifndef _TRE_IMPORTER_H__
#define _TRE_IMPORTER_H__

#include "COLLADAFW.h"
#include "COLLADASaxFWLLoader.h"
#include "COLLADAFWIWriter.h"

#include "Platform.h"
#include "ResourceManager.h"

namespace TRE
{
	class Importer : public COLLADAFW::IWriter
	{
		DECLARE_SINGLETON( Importer )

	public:

		INT32 ImportFromDaeFile( std::string f );

		/** This method will be called if an error in the loading process occurred and the loader cannot
		continue to to load. The writer should undo all operations that have been performed.
		@param errorMessage A message containing informations about the error that occurred.
		*/
		void cancel(const COLLADAFW::String& errorMessage);

		/** This is the method called. The writer hast to prepare to receive data.*/
		void start();

		/** This method is called after the last write* method. No other methods will be called after this.*/
		void finish();

        /** When this method is called, the writer must write the global document asset.
        @return The writer should return true, if writing succeeded, false otherwise.*/
        bool writeGlobalAsset ( const COLLADAFW::FileInfo* asset );

        /** When this method is called, the writer must write the scene.
        @return The writer should return true, if writing succeeded, false otherwise.*/
        bool writeScene ( const COLLADAFW::Scene* scene );

		/** When this method is called, the writer must write the entire visual scene.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeVisualScene ( const COLLADAFW::VisualScene* visualScene );

		/** When this method is called, the writer must handle all nodes contained in the 
		library nodes.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeLibraryNodes ( const COLLADAFW::LibraryNodes* libraryNodes );

		/** When this method is called, the writer must write the geometry.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeGeometry ( const COLLADAFW::Geometry* geometry );

		/** When this method is called, the writer must write the material.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeMaterial( const COLLADAFW::Material* material );

		/** When this method is called, the writer must write the effect.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeEffect( const COLLADAFW::Effect* effect );

		/** When this method is called, the writer must write the camera.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeCamera( const COLLADAFW::Camera* camera );

		/** When this method is called, the writer must write the image.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeImage( const COLLADAFW::Image* image );

		/** When this method is called, the writer must write the light.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeLight( const COLLADAFW::Light* light );

		/** When this method is called, the writer must write the Animation.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeAnimation( const COLLADAFW::Animation* animation );

		/** When this method is called, the writer must write the AnimationList.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeAnimationList( const COLLADAFW::AnimationList* animationList );

		/** When this method is called, the writer must write the skin controller data.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeSkinControllerData( const COLLADAFW::SkinControllerData* skinControllerData );

		/** When this method is called, the writer must write the controller.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeController( const COLLADAFW::Controller* controller );

        /** When this method is called, the writer must write the formulas. All the formulas of the entire
		COLLADA file are contained in @a formulas.
        @return The writer should return true, if writing succeeded, false otherwise.*/
        bool writeFormulas( const COLLADAFW::Formulas* formulas );

		/** When this method is called, the writer must write the kinematics scene. 
		@return The writer should return true, if writing succeeded, false otherwise.*/
		bool writeKinematicsScene( const COLLADAFW::KinematicsScene* kinematicsScene );

	private:

		bool ImportVisualSceneNode( const COLLADAFW::Node* node, Node* parent );

		void ConnectInstances();

		/**
		* Matrix is read in column-major order, but ColladaOptimizer outputs them in row major order,
		* Transpose the matrix read in by OpenCollada
		*/
		void CopyMatrixToDirectX( D3DXMATRIX& mOut, const COLLADABU::Math::Matrix4& m );

		enum IMPORT_RUN
		{
			// Reserved
			IR_VALIDATE,

			// Reserved
			IR_PRE_LOAD,

			IR_RESOURCE,

			// Reserved
			IR_Instance,
		};

		IMPORT_RUN mRun;

		std::map< COLLADAFW::UniqueId, Light* > mapLights;

		std::map< COLLADAFW::UniqueId, Camera* > mapCameras;

		std::map< COLLADAFW::UniqueId, StaticGeometry* > mapStaticMeshes;

		std::map< COLLADAFW::UniqueId, SkeletalGeometry* > mapSkeletalControllers;

		std::map< COLLADAFW::UniqueId, SkinData* > mapSkinDatas;

		std::map< COLLADAFW::UniqueId, Node* > mapNodes;

		std::map< COLLADAFW::UniqueId, COLLADAFW::UniqueId > mapMaterials2Effects;

		std::map< COLLADAFW::UniqueId, PhongParam* > mapPhongParams;

		std::map< COLLADAFW::UniqueId, COLLADAFW::UniqueId > mapEffectDiffuse2Images;

		std::map< COLLADAFW::UniqueId, std::string > mapImageID2ImageURL;

		std::map< Mesh*, COLLADAFW::MaterialId > mapStaticMeshes2Material;

		std::map< Scene*, COLLADAFW::UniqueId > mapScene2VisualNodes;

		std::map< Node*, COLLADAFW::UniqueId > mapNodes2Cameras;

		std::map< Node*, COLLADAFW::UniqueId > mapNodes2Lights;
	
		std::map< Node*, COLLADAFW::UniqueId > mapNodes2Geometries;
	
		std::map< Node*, COLLADAFW::UniqueId > mapNodes2Controllers;

		struct MatBind
		{
			COLLADAFW::MaterialId;
			
			COLLADAFW::UniqueId; 
		};

		std::map< std::vector< MatBind > mapMatID2Mat;

		std::map< COLLADAFW::UniqueId, Animation* > mapAnimations;

		std::map< COLLADAFW::UniqueId, COLLADAFW::UniqueId > mapAnimationList2Animation;
	
		std::map< SkeletalGeometry*, COLLADAFW::UniqueId > mapSkeletalMesh2Geometry;

		std::map< SkeletalGeometry*, COLLADAFW::UniqueId > mapSkeletalMesh2SkinData;

		std::map< SkeletalGeometry*, COLLADAFW::UniqueIdArray > mapSkeletalMesh2Joints;
		
		ResourceFactoryController* m_pRFC;
	};
};

#endif