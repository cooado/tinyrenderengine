#ifndef _TRE_IMPORTER_DOM_H__
#define _TRE_IMPORTER_DOM_H__

#include "Platform.h"
#include "ResourceManager.h"

#include <dae.h>
#include <dom/domCOLLADA.h>
#include <dom/domConstants.h>
#include <dom/domTypes.h>

namespace TRE
{
	class Importer_Dom
	{
		DECLARE_SINGLETON( Importer_Dom )

	public:
	
		INT32 ImportFromDaeFile( std::string f );

	private:

		INT32 ImportLight( domCOLLADA* root );

		INT32 ImportCamera( domCOLLADA* root );

		INT32 ImportGeometry( domCOLLADA* root );

		INT32 ImportSkinController( domCOLLADA* root );

		INT32 ImportNode( domCOLLADA* root );

		INT32 ImportAnimation( domCOLLADA* root );;

		/**
		* @param
		*	m	Row major matrix with left vector as output from collada optimizer	
		*/
		void CopyMatrixToDirectX( D3DXMATRIX& mOut, domListOfFloats& m, UINT32 startindex );

		void CopyColorToDirectX( D3DXCOLOR& cOut, domListOfFloats& color );

		INT32 ImportOneNode( domNode* node, Node* parent );

		INT32 ImportJoint( Skeleton* skel, domNode* joint, bool isroot );

		INT32 ImportOneAnimation( domAnimation* anim, Animation* parent );

		INT32 ImportMaterial( domCOLLADA* root );

		ResourceFactoryController* m_pRFC;

		std::map< domLight*, Light* > mapLights;

		std::map< domNode*, Node* > mapNodes;

		std::map< std::string, Node* > mapID2Nodes;

		std::map< Skeleton*, domNode* > mapSkeleton2Node;

		std::map< domCamera*, Camera* > mapCameras;

		std::map< domGeometry*, Geometry* > mapGeometries;

		std::map< domController*, Skin* > mapSkinData;

		std::map< domMaterial*, PhongParam* > mapParams;
	};

};

#endif