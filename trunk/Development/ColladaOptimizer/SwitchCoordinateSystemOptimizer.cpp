#include "SwitchCoordinateSystemOptimizer.h"

namespace COP
{

	/**************************************************************************
	* Collada defaults to column major matrix with column vector:
	*		0	4	8	12		x
	*		1	5	9	13	*	y	
	*		2	6	10	14		z
	*		3	7	11	15		w
	*
	* Convert it to:
	*							0	1	2	3
	*		x	y	z	w	*	4	5	6	7	=	x'	y'	z'	w'
	*							8	9	10	11
	*							12	13	14	15
	*
	* X_UP:
	*							5	4	6	7
	*		y	x	z	w	*	1	0	2	3	=	y'	x'	z'	w'
	*							9	8	10	11
	*							13	12	14	15
	*
	* Y_UP:
	*							10	9	8	11
	*		z	y	x	w	*	6	5	4	7	=	z'	y'	x'	w'
	*							2	1	0	3
	*							14	13	12	15
	*
	* Z_UP:
	*							0	2	1	3
	*		x	z	y	w	*	8	10	9	11	=	x'	z'	y'	w'
	*							4	6	5	7
	*							12	14	13	15
	*
	* Finally, we store the matrix in row major order, which is:
	*
	* However, OpenCollada exporter for MAX outputs column major matrix with left row vector
	*
	*							0	4	8	12
	*		x	y	z	w	*	1	5	9	13	=	x'	y'	z'	w'
	*							2	6	10	14
	*							3	7	11	15
	*
	* X_UP:
	*							5	1	9	13
	*		y	x	z	w	*	4	0	8	12	=	y'	x'	z'	w'
	*							6	2	10	14
	*							7	3	11	15
	*
	* Y_UP:
	*							10	6	2	14
	*		z	y	x	w	*	9	5	1	13	=	z'	y'	x'	w'
	*							8	4	0	12
	*							11	7	3	15
	*
	* Z_UP:
	*							0	8	4	12
	*		x	z	y	w	*	2	10	6	14	=	x'	z'	y'	w'
	*							1	9	5	13
	*							3	11	7	15
	**************************************************************************/
	void SwitchCoordinateSystemOptimizer::SwitchCoordinateSystemForFloatList( domListOfFloats& values, FloatListType type, const domUpAxisType upaxis )
	{
		switch( type )
		{
		case Translate:
			{
				unsigned int iStride = 3;
				unsigned int Count = values.getCount() / iStride ;
				switch( upaxis )
				{
				case UPAXISTYPE_X_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat oldx = values.get( i * iStride );
						values.set( i * iStride , values.get( i * iStride + 1 ) );
						values.set( i * iStride + 1, oldx );
					};
					break;
				case UPAXISTYPE_Y_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat oldx = values.get( i * iStride );
						values.set( i * iStride , values.get( i * iStride + 2 ) );
						values.set( i * iStride + 2, oldx );
					};
					break;
				case UPAXISTYPE_Z_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat oldy = values.get( i * iStride + 1 );
						values.set( i * iStride + 1, values.get( i * iStride + 2 ) );
						values.set( i * iStride + 2, oldy );
					};
					break;
				default:
					break;
				};
			};
			break;
		case Rotate:
			{
				unsigned int iStride = 4;
				unsigned int Count = values.getCount() / iStride ;
				switch( upaxis )
				{
				case UPAXISTYPE_X_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat oldx = values.get( i * iStride );
						values.set( i * iStride , values.get( i * iStride + 1 ) );
						values.set( i * iStride + 1, oldx );
					};
					break;
				case UPAXISTYPE_Y_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat oldx = values.get( i * iStride );
						values.set( i * iStride , values.get( i * iStride + 2 ) );
						values.set( i * iStride + 2, oldx );
					};
					break;
				case UPAXISTYPE_Z_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat oldy = values.get( i * iStride + 1 );
						values.set( i * iStride + 1, values.get( i * iStride + 2 ) );
						values.set( i * iStride + 2, oldy );
					};
					break;
				default:
					break;
				};
			};
			break;
		case Matrix:
			{
				unsigned int iStride = 16;
				unsigned int Count = values.getCount() / iStride ;
				switch( upaxis )
				{
				case UPAXISTYPE_X_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat m[ 16 ];
						m[ 0 ] = values.get( i * iStride + 5 );
						m[ 1 ] = values.get( i * iStride + 1 );
						m[ 2 ] = values.get( i * iStride + 9 );
						m[ 3 ] = values.get( i * iStride + 13 );

						m[ 4 ] = values.get( i * iStride + 4 );
						m[ 5 ] = values.get( i * iStride + 0 );
						m[ 6 ] = values.get( i * iStride + 8 );
						m[ 7 ] = values.get( i * iStride + 12 );
						
						m[ 8 ] = values.get( i * iStride + 6 );
						m[ 9 ] = values.get( i * iStride + 2 );
						m[ 10 ] = values.get( i * iStride + 10 );
						m[ 11 ] = values.get( i * iStride + 14 );
						
						m[ 12 ] = values.get( i * iStride + 7 );
						m[ 13 ] = values.get( i * iStride + 3 );
						m[ 14 ] = values.get( i * iStride + 11 );
						m[ 15 ] = values.get( i * iStride + 15 );
						
						values.set( i * iStride + 0, m[ 0 ] );
						values.set( i * iStride + 1, m[ 1 ] );
						values.set( i * iStride + 2, m[ 2 ] );
						values.set( i * iStride + 3, m[ 3 ] );

						values.set( i * iStride + 4, m[ 4 ] );
						values.set( i * iStride + 5, m[ 5 ] );
						values.set( i * iStride + 6, m[ 6 ] );
						values.set( i * iStride + 7, m[ 7 ] );

						values.set( i * iStride + 8, m[ 8 ] );
						values.set( i * iStride + 9, m[ 9 ] );
						values.set( i * iStride + 10, m[ 10 ] );
						values.set( i * iStride + 11, m[ 11 ] );

						values.set( i * iStride + 12, m[ 12 ] );
						values.set( i * iStride + 13, m[ 13 ] );
						values.set( i * iStride + 14, m[ 14 ] );
						values.set( i * iStride + 15, m[ 15 ] );	
					};
					break;
				case UPAXISTYPE_Y_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat m[ 16 ];
						m[ 0 ] = values.get( i * iStride + 10 );
						m[ 1 ] = values.get( i * iStride + 6 );
						m[ 2 ] = values.get( i * iStride + 2 );
						m[ 3 ] = values.get( i * iStride + 14 );

						m[ 4 ] = values.get( i * iStride + 9 );
						m[ 5 ] = values.get( i * iStride + 5 );
						m[ 6 ] = values.get( i * iStride + 1 );
						m[ 7 ] = values.get( i * iStride + 13 );
						
						m[ 8 ] = values.get( i * iStride + 8 );
						m[ 9 ] = values.get( i * iStride + 4 );
						m[ 10 ] = values.get( i * iStride + 0 );
						m[ 11 ] = values.get( i * iStride + 12 );
						
						m[ 12 ] = values.get( i * iStride + 11 );
						m[ 13 ] = values.get( i * iStride + 7 );
						m[ 14 ] = values.get( i * iStride + 3 );
						m[ 15 ] = values.get( i * iStride + 15 );
						
						values.set( i * iStride + 0, m[ 0 ] );
						values.set( i * iStride + 1, m[ 1 ] );
						values.set( i * iStride + 2, m[ 2 ] );
						values.set( i * iStride + 3, m[ 3 ] );

						values.set( i * iStride + 4, m[ 4 ] );
						values.set( i * iStride + 5, m[ 5 ] );
						values.set( i * iStride + 6, m[ 6 ] );
						values.set( i * iStride + 7, m[ 7 ] );

						values.set( i * iStride + 8, m[ 8 ] );
						values.set( i * iStride + 9, m[ 9 ] );
						values.set( i * iStride + 10, m[ 10 ] );
						values.set( i * iStride + 11, m[ 11 ] );

						values.set( i * iStride + 12, m[ 12 ] );
						values.set( i * iStride + 13, m[ 13 ] );
						values.set( i * iStride + 14, m[ 14 ] );
						values.set( i * iStride + 15, m[ 15 ] );		
					};
					break;
				case UPAXISTYPE_Z_UP:
					for( unsigned int i = 0; i < Count; i++ )
					{
						domFloat m[ 16 ];
						m[ 0 ] = values.get( i * iStride + 0 );
						m[ 1 ] = values.get( i * iStride + 8 );
						m[ 2 ] = values.get( i * iStride + 4 );
						m[ 3 ] = values.get( i * iStride + 12 );

						m[ 4 ] = values.get( i * iStride + 2 );
						m[ 5 ] = values.get( i * iStride + 10 );
						m[ 6 ] = values.get( i * iStride + 6 );
						m[ 7 ] = values.get( i * iStride + 14 );
						
						m[ 8 ] = values.get( i * iStride + 1 );
						m[ 9 ] = values.get( i * iStride + 9 );
						m[ 10 ] = values.get( i * iStride + 5 );
						m[ 11 ] = values.get( i * iStride + 13 );
						
						m[ 12 ] = values.get( i * iStride + 3 );
						m[ 13 ] = values.get( i * iStride + 11 );
						m[ 14 ] = values.get( i * iStride + 7 );
						m[ 15 ] = values.get( i * iStride + 15 );
						
						values.set( i * iStride + 0, m[ 0 ] );
						values.set( i * iStride + 1, m[ 1 ] );
						values.set( i * iStride + 2, m[ 2 ] );
						values.set( i * iStride + 3, m[ 3 ] );

						values.set( i * iStride + 4, m[ 4 ] );
						values.set( i * iStride + 5, m[ 5 ] );
						values.set( i * iStride + 6, m[ 6 ] );
						values.set( i * iStride + 7, m[ 7 ] );

						values.set( i * iStride + 8, m[ 8 ] );
						values.set( i * iStride + 9, m[ 9 ] );
						values.set( i * iStride + 10, m[ 10 ] );
						values.set( i * iStride + 11, m[ 11 ] );

						values.set( i * iStride + 12, m[ 12 ] );
						values.set( i * iStride + 13, m[ 13 ] );
						values.set( i * iStride + 14, m[ 14 ] );
						values.set( i * iStride + 15, m[ 15 ] );	
					};
					break;
				default:
					break;
				};
			};
			break;
		default:
			break;
		};
	};

	bool SwitchCoordinateSystemOptimizer::SwitchCoordinateSystemForGeometryLibrary( domLibrary_geometriesRef geometries, const domUpAxisType upaxis )
	{
		domGeometry_Array &arrGeometry = geometries->getGeometry_array();
		for( unsigned int iGeom = 0; iGeom < arrGeometry.getCount(); iGeom++ )
		{
			std::set< domSource* > setSourcesForSwitch;
			domTriangles_Array &arrTriangles = arrGeometry[ iGeom ]->getMesh()->getTriangles_array();
			for( unsigned int iTriangle = 0; iTriangle < arrTriangles.getCount(); iTriangle++ )
			{
				domInputLocalOffset_Array &arrInputs = arrTriangles[ iTriangle ]->getInput_array();
				for( unsigned int iInput = 0; iInput < arrInputs.getCount(); iInput++ )
				{
					xsNMTOKEN semantic = arrInputs[ iInput ]->getSemantic();
					if( std::string( semantic ) == std::string( "VERTEX" ) )
					{
						domVertices* vertices = static_cast< domVertices* >( arrInputs[ iInput ]->getSource().getElement().cast() );
						domInputLocal_Array &vinputs = vertices->getInput_array();
						for( unsigned int iVI = 0; iVI < vinputs.getCount(); iVI++ )
						{
							if( std::string( vinputs[ iVI ]->getSemantic() ) == std::string( "POSITION" ) ||
								std::string( vinputs[ iVI ]->getSemantic() ) == std::string( "NORMAL" ) )
							{
								domSource* vsource = static_cast< domSource* >( vinputs[ iVI ]->getSource().getElement().cast() );
								setSourcesForSwitch.insert( vsource );
							};
						};
					}
					else if( std::string( semantic ) == std::string( "POSITION" ) ||
						std::string( semantic ) == std::string( "NORMAL" ) )
					{
						domSource* source = static_cast< domSource* >( arrInputs[ iInput ]->getSource().getElement().cast() );
						setSourcesForSwitch.insert( source );
					};
				};
			};
			
			for( std::set< domSource* >::iterator i = setSourcesForSwitch.begin(); i != setSourcesForSwitch.end(); i++ )
			{
				SwitchCoordinateSystemForFloatList( ( *i )->getFloat_array()->getValue(), Translate, upaxis );
			};
		};

		return true;
	};

	bool SwitchCoordinateSystemOptimizer::SwitchCoordinateSystemForNode( domNodeRef node, const domUpAxisType upaxis )
	{
		// Translate
		domTranslate_Array &arrTrans = node->getTranslate_array();
		for( unsigned int iTrans = 0; iTrans < arrTrans.getCount(); iTrans++ )
		{
			SwitchCoordinateSystemForFloatList( arrTrans[ iTrans ]->getValue(), Translate, upaxis );
		};

		// Rotate
		domRotate_Array &arrRots = node->getRotate_array();
		for( unsigned int iRot = 0; iRot < arrRots.getCount(); iRot++ )
		{
			SwitchCoordinateSystemForFloatList( arrRots[ iRot ]->getValue(), Rotate, upaxis );
		};

		// Matrix
		domMatrix_Array& arrMatrix = node->getMatrix_array();
		for( unsigned int iM = 0; iM < arrMatrix.getCount(); iM++ )
		{
			SwitchCoordinateSystemForFloatList( arrMatrix[ iM ]->getValue(), Matrix, upaxis );
		};

		/** Camera node
		* OpenCollada exporter for max exports camera in right handed system with
		*		Up axis: +Y
		*		Direction: -Z
		*
		* We have to convert the camera to left handed system with
		*		Up axis: +Y
		*		Direction: +Z
		*
		*		Y				Z				Y	Z
		*		|				|				| /
		*		0--X	==>		0--X	==>		0--X
		*	   /			   /
		*	  Z				  Y					
		*
		* So we need to modify the node transformation matrix:
		*
		*				1	0	0	0
		*		M'	=	0	0	1	0	*	M, where M is the transformed matrix in the last step
		*				0	-1	0	0
		*				0	0	0	1
		*
		*/
		if( node->getInstance_camera_array().getCount() > 0 )
		{
			domFloat4x4 &mat = arrMatrix[ 0 ]->getValue();
			domFloat4x4 m;

			m.append( mat.get( 0 ) );
			m.append( mat.get( 1 ) );
			m.append( mat.get( 2 ) );
			m.append( mat.get( 3 ) );

			m.append( mat.get( 8 ) );
			m.append( mat.get( 9 ) );
			m.append( mat.get( 10 ) );
			m.append( mat.get( 11 ) );

			m.append( -1 * mat.get( 4 ) );
			m.append( -1 * mat.get( 5 ) );
			m.append( -1 * mat.get( 6 ) );
			m.append( -1 * mat.get( 7 ) );

			m.append( mat.get( 12 ) );
			m.append( mat.get( 13 ) );
			m.append( mat.get( 14 ) );
			m.append( mat.get( 15 ) );

			mat.set( 0, m.get( 0 ) );
			mat.set( 1, m.get( 1 ) );
			mat.set( 2, m.get( 2 ) );
			mat.set( 3, m.get( 3 ) );
			mat.set( 4, m.get( 4 ) );
			mat.set( 5, m.get( 5 ) );
			mat.set( 6, m.get( 6 ) );
			mat.set( 7, m.get( 7 ) );
			mat.set( 8, m.get( 8 ) );
			mat.set( 9, m.get( 9 ) );
			mat.set( 10, m.get( 10 ) );
			mat.set( 11, m.get( 11 ) );
			mat.set( 12, m.get( 12 ) );
			mat.set( 13, m.get( 13 ) );
			mat.set( 14, m.get( 14 ) );
			mat.set( 15, m.get( 15 ) );
		};

		// Child nodes
		domNode_Array &arrChildren = node->getNode_array();
		for( unsigned int iN = 0; iN < arrChildren.getCount(); iN++ )
		{
			SwitchCoordinateSystemForNode( arrChildren[ iN ], upaxis );
		};

		return true;
	};

	bool SwitchCoordinateSystemOptimizer::SwitchCoordinateSystemForVisualSceneLibrary( domLibrary_visual_scenesRef visualscenes, const domUpAxisType upaxis )
	{
		domVisual_scene_Array &arrVS = visualscenes->getVisual_scene_array();
		for( unsigned int iVS = 0; iVS < arrVS.getCount(); iVS++ )
		{
			domNode_Array &arrNodes = arrVS[ iVS ]->getNode_array();
			for( unsigned int iNode = 0; iNode < arrNodes.getCount(); iNode++ )
			{
				SwitchCoordinateSystemForNode( arrNodes[ iNode ], upaxis );
			};
		};

		return true;
	};

	bool SwitchCoordinateSystemOptimizer::SwitchCoordinateSystemForAnimation( domAnimationRef anim, const domUpAxisType upaxis )
	{
		domSampler_Array &arrSamplers = anim->getSampler_array();
		for( unsigned int iSampler = 0; iSampler < arrSamplers.getCount(); iSampler++ )
		{
			domSamplerRef sampler = arrSamplers[ iSampler ];
			domInputLocal_Array &arrInputs = sampler->getInput_array();
			for( unsigned int iInput = 0; iInput < arrInputs.getCount(); iInput++ )
			{
				domInputLocalRef input = arrInputs[ iInput ];
				if( std::string( input->getSemantic() ) == std::string( "OUTPUT" ) )
				{
					domSource* source = static_cast< domSource* >( input->getSource().getElement().cast() );
					if( std::string( source->getTechnique_common()->getAccessor()->getParam_array()[ 0 ]->getType() ) == std::string( "float4x4" ) )
					{
						SwitchCoordinateSystemForFloatList( source->getFloat_array()->getValue(), Matrix, upaxis );
					};
				};
			};
		};

		// child animations
		domAnimation_Array &children = anim->getAnimation_array();
		for( unsigned int iAnim = 0; iAnim < children.getCount(); iAnim++ )
		{
			SwitchCoordinateSystemForAnimation( children[ iAnim ], upaxis );
		};

		return true;
	};

	bool SwitchCoordinateSystemOptimizer::SwitchCoordinateSystemForAnimationLibrary( domLibrary_animationsRef animations, const domUpAxisType upaxis )
	{
		domAnimation_Array& arrAnims = animations->getAnimation_array();
		for( unsigned int iAnim = 0; iAnim < arrAnims.getCount(); iAnim++ )
		{
			SwitchCoordinateSystemForAnimation( arrAnims[ iAnim ], upaxis );
		};

		return true;
	};

	bool SwitchCoordinateSystemOptimizer::SwitchCoordinateSystemForControllerLibrary( domLibrary_controllersRef controllers, const domUpAxisType upaxis )
	{
		domController_Array& arrControllers = controllers->getController_array();
		for( unsigned int iC = 0; iC < arrControllers.getCount(); iC++ )
		{
			domSkinRef skin = arrControllers[ iC ]->getSkin();
			
			// Bind shape matrix
			SwitchCoordinateSystemForFloatList( skin->getBind_shape_matrix()->getValue(), Matrix, upaxis );

			domInputLocal_Array &arrJointInputs = skin->getJoints()->getInput_array();
			for( unsigned int iJI = 0; iJI < arrJointInputs.getCount(); iJI++ )
			{
				if( std::string( arrJointInputs[ iJI ]->getSemantic() ) == std::string( "INV_BIND_MATRIX" ) )
				{
					// Inverse bind matrix
					domSource* source = static_cast< domSource* >( arrJointInputs[ iJI ]->getSource().getElement().cast() );
					SwitchCoordinateSystemForFloatList( source->getFloat_array()->getValue(), Matrix, upaxis );
				};
			};
		};

		return true;
	};

	int SwitchCoordinateSystemOptimizer::Process( domCOLLADA* root )
	{
		domUpAxisType ColladaUpAxis = root->getAsset()->getUp_axis()->getValue();
		
		// Geometry
		domLibrary_geometries_Array &arrGeoms = root->getLibrary_geometries_array();
		for( unsigned int iLibraryGeom = 0; iLibraryGeom < arrGeoms.getCount(); iLibraryGeom++ )
		{
			SwitchCoordinateSystemForGeometryLibrary( arrGeoms[ iLibraryGeom ], ColladaUpAxis );
		};

		// Visual scene
		domLibrary_visual_scenes_Array &arrVisualScenes = root->getLibrary_visual_scenes_array();
		for( unsigned int iLibraryVS = 0; iLibraryVS < arrVisualScenes.getCount(); iLibraryVS++ )
		{
			SwitchCoordinateSystemForVisualSceneLibrary( arrVisualScenes[ iLibraryVS ], ColladaUpAxis );
		};

		// Animation
		domLibrary_animations_Array& arrAnimationLibs = root->getLibrary_animations_array();
		for( unsigned int iLibraryAnim = 0; iLibraryAnim < arrAnimationLibs.getCount(); iLibraryAnim++ )
		{
			SwitchCoordinateSystemForAnimationLibrary( arrAnimationLibs[ iLibraryAnim ], ColladaUpAxis );
		};

		// Controller
		domLibrary_controllers_Array& arrControllerLibs = root->getLibrary_controllers_array();
		for( unsigned int iLibraryController = 0; iLibraryController < arrControllerLibs.getCount(); iLibraryController++ )
		{
			SwitchCoordinateSystemForControllerLibrary( arrControllerLibs[ iLibraryController ], ColladaUpAxis );
		};
		return true;
	};
};