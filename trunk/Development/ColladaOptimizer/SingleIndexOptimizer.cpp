#include "SingleIndexOptimizer.h"

namespace COP
{

	bool SingleIndexOptimizer::ChangeWindingOrder(COP::SingleIndexOptimizer::Triangle_Index &tri)
	{
		for( domUint i = 0; i < tri.Index_Count / 3; i++ )
		{
			domUint temp = tri.New_Index[ 3 * i + 1 ];
			tri.New_Index[ 3 * i + 1 ] = tri.New_Index[ 3 * i + 2 ];
			tri.New_Index[ 3 * i + 2 ] = temp;
		};
		return true;
	};

	/**
	* Replace the old index with a single new index to all components.
	*/
	bool SingleIndexOptimizer::CreateSingleIndex( Triangle_Index& tri )
	{
		for( domUint i = 0; i < tri.Index_Count; i++ )
		{
			MultiIndex old_index;
			old_index.Index_Position = *( tri.Original_Index[ IS_POSITION ] + i );
			old_index.Index_Normal = *( tri.Original_Index[ IS_NORMAL ] + i );
			old_index.Index_Texcoord = *( tri.Original_Index[ IS_TEXCOORD0 ] + i );

			std::map< MultiIndex, domUint, MultiIndexCompareFunctor >::iterator iMap = mapIndex.find( old_index );
			if( iMap != mapIndex.end() )
			{
				*( tri.New_Index + i ) = iMap->second;
			}
			else
			{
				domUint newindex = mapIndex.size();
				*( tri.New_Index + i ) = newindex;
				mapIndex.insert( std::pair< MultiIndex, domUint >( old_index, newindex ) );
			};
		};

		return true;
	};

	int SingleIndexOptimizer::Process(domCOLLADA *root)
	{
		domLibrary_geometries_Array & library_geometries = root->getLibrary_geometries_array();
		for( unsigned int iGeometries = 0; iGeometries < library_geometries.getCount(); iGeometries++ )
		{
			domGeometry_Array geometries = library_geometries[ iGeometries ]->getGeometry_array();;
			for( unsigned int iGeom = 0; iGeom < geometries.getCount(); iGeom++ )
			{
				domGeometryRef geometry = geometries[ iGeom ];
				domMeshRef mesh = geometry->getMesh();
				
				domSource* VBs[ IS_END ];
				VBs[ IS_POSITION ] = NULL;
				VBs[ IS_NORMAL ] = NULL;
				VBs[ IS_TEXCOORD0 ] = NULL;
				domTriangles_Array arrTriangle = mesh->getTriangles_array();
				for( unsigned int iTriangle = 0; iTriangle < arrTriangle.getCount(); iTriangle++ )
				{
					domTrianglesRef triangle = arrTriangle[ iTriangle ];
					domInputLocalOffset_Array& arrInput = triangle->getInput_array();
					Triangle_Index tri_index;
					domUint offsets[ IS_END ];
					offsets[ IS_POSITION ] = 0;
					offsets[ IS_NORMAL ] = 0;
					offsets[ IS_TEXCOORD0 ] = 0;
					for(unsigned int iInput = 0; iInput < arrInput.getCount(); iInput++ )
					{
						domInputLocalOffsetRef input = arrInput[ iInput ];
						if( std::string( input->getSemantic() ) == std::string( "VERTEX" ) )
						{
							domVertices* vertices = static_cast< domVertices* >( input->getSource().getElement().cast() );
							domInputLocal_Array &v_inputs = vertices->getInput_array();
							for( unsigned int iV_Input = 0; iV_Input < v_inputs.getCount(); iV_Input++ )
							{
								domInputLocalRef vinput = v_inputs[ iV_Input ];
								if( std::string( vinput->getSemantic() ) == std::string( "POSITION" ) )
								{
									domSource* source = static_cast< domSource* >( vinput->getSource().getElement().cast() );
									offsets[ IS_POSITION ] = input->getOffset();
									if( VBs[ IS_POSITION ] == NULL )
									{
										VBs[ IS_POSITION ] = source;
									}
									else if( VBs[ IS_POSITION ] != source )
									{
										COP_LOG( " NOT SUPPORTED: Inputs with the same semantic refer to different sources in a mesh" );
										return -1;
									};
								}
								else if( std::string( vinput->getSemantic() ) == std::string( "NORMAL" ) )
								{
									domSource* source = static_cast< domSource* >( vinput->getSource().getElement().cast() );
									offsets[ IS_NORMAL ] = input->getOffset();
									if( VBs[ IS_NORMAL ] == NULL )
									{
										VBs[ IS_NORMAL ] = source;
									}
									else if( VBs[ IS_NORMAL ] != source )
									{
										COP_LOG( " NOT SUPPORTED: Inputs with the same semantic refer to different sources in a mesh" );
										return -1;
									};
								}
								else if( std::string( vinput->getSemantic() ) == std::string( "TEXCOORD" ) )
								{
									domSource* source = static_cast< domSource* >( vinput->getSource().getElement().cast() );
									offsets[ IS_TEXCOORD0 ] = input->getOffset();
									if( VBs[ IS_TEXCOORD0 ] == NULL )
									{
										VBs[ IS_TEXCOORD0 ] = source;
									}
									else if( VBs[ IS_TEXCOORD0 ] != source )
									{
										COP_LOG( " NOT SUPPORTED: Inputs with the same semantic refer to different sources in a mesh" );
										return -1;
									};
								};
							};
						}
						else if( std::string( input->getSemantic() ) == std::string( "NORMAL" ) )
						{
							domSource* source = static_cast< domSource* >( input->getSource().getElement().cast() );
							offsets[ IS_NORMAL ] = input->getOffset();
							if( VBs[ IS_NORMAL ] == NULL )
							{
								VBs[ IS_NORMAL ] = source;
							}
							else if( VBs[ IS_NORMAL ] != source )
							{
								COP_LOG( " NOT SUPPORTED: Inputs with the same semantic refer to different sources in a mesh" );
								return -1;
							};
						}
						else if( std::string( input->getSemantic() ) == std::string( "TEXCOORD" ) )
						{
							domSource* source = static_cast< domSource* >( input->getSource().getElement().cast() );
							offsets[ IS_TEXCOORD0 ] = input->getOffset();
							if( VBs[ IS_TEXCOORD0 ] == NULL )
							{
								VBs[ IS_TEXCOORD0 ] = source;
							}
							else if( VBs[ IS_TEXCOORD0 ] != source )
							{
								COP_LOG( " NOT SUPPORTED: Inputs with the same semantic refer to different sources in a mesh" );
								return -1;
							};
						};
					};

					// Create new index
					domListOfUInts& listIndex = triangle->getP()->getValue();
					domUint stride = listIndex.getCount() /( triangle->getCount() * 3 );

					tri_index.Index_Count = triangle->getCount() * 3;
					tri_index.Original_Index[ IS_POSITION ] = new domUint[ ( unsigned int )tri_index.Index_Count ];
					tri_index.Original_Index[ IS_NORMAL ] = new domUint[ ( unsigned int )tri_index.Index_Count ];
					tri_index.Original_Index[ IS_TEXCOORD0 ] = new domUint[ ( unsigned int )tri_index.Index_Count ];
					tri_index.New_Index = new domUint[ ( unsigned int )tri_index.Index_Count ];
					for( domUint iVert = 0; iVert < tri_index.Index_Count; iVert++ )
					{
						*( tri_index.Original_Index[ IS_POSITION ] + iVert ) = listIndex.get( ( unsigned int )( iVert * stride + offsets[ IS_POSITION ] ) );
						*( tri_index.Original_Index[ IS_NORMAL ] + iVert ) = listIndex.get( ( unsigned int )( iVert * stride + offsets[ IS_NORMAL ] ) );
						*( tri_index.Original_Index[ IS_TEXCOORD0 ] + iVert ) = listIndex.get( ( unsigned int )( iVert * stride + offsets[ IS_TEXCOORD0 ] ) );
					};
					CreateSingleIndex( tri_index );
					ChangeWindingOrder( tri_index );

					// Add new index to collada
					listIndex.clear();
					for( domUint iVert = 0; iVert < tri_index.Index_Count; iVert++ )
					{
						listIndex.append( *( tri_index.New_Index + iVert ) );
					};
					for(unsigned int iInput = 0; iInput < arrInput.getCount(); iInput++ )
					{
						domInputLocalOffsetRef input = arrInput[ iInput ];
						input->setOffset( 0 );
					};

					// Delete temporary data
					delete tri_index.New_Index;
					delete tri_index.Original_Index[ IS_POSITION ];
					delete tri_index.Original_Index[ IS_NORMAL ];
					delete tri_index.Original_Index[ IS_TEXCOORD0 ];
				};

				// Create new vertex buffer source
				domListOfFloats & pos_source = VBs[ IS_POSITION ]->getFloat_array()->getValue();
				domListOfFloats & normal_source = VBs[ IS_NORMAL ]->getFloat_array()->getValue();
				domListOfFloats & texcoord_source = VBs[ IS_TEXCOORD0 ]->getFloat_array()->getValue();
				domUint pos_stride = VBs[ IS_POSITION ]->getTechnique_common()->getAccessor()->getStride();
				domUint normal_stride = VBs[ IS_NORMAL ]->getTechnique_common()->getAccessor()->getStride();
				domUint texcoord_stride = VBs[ IS_TEXCOORD0 ]->getTechnique_common()->getAccessor()->getStride();

				unsigned int NewVertCount = mapIndex.size();
				domFloat* new_pos = new domFloat[ mapIndex.size() * ( unsigned int )pos_stride ];
				domFloat* new_normal = new domFloat[ mapIndex.size() * ( unsigned int )normal_stride ];
				domFloat* new_texcoord = new domFloat[ mapIndex.size() * ( unsigned int )texcoord_stride ];

				for( std::map< MultiIndex, domUint, MultiIndexCompareFunctor >::iterator iNewIndex = mapIndex.begin(); iNewIndex != mapIndex.end(); iNewIndex++ )
				{
					for( domUint iStride = 0; iStride < pos_stride; iStride++ )
					{
						*( new_pos + iNewIndex->second * pos_stride + iStride ) = pos_source.get( ( unsigned int )( iNewIndex->first.Index_Position * pos_stride + iStride ) );
					};
					for( domUint iStride = 0; iStride < normal_stride; iStride++ )
					{
						*( new_normal + iNewIndex->second * normal_stride + iStride ) = normal_source.get( ( unsigned int )( iNewIndex->first.Index_Normal * normal_stride + iStride ) );
					};
					for( domUint iStride = 0; iStride < texcoord_stride; iStride++ )
					{
						*( new_texcoord + iNewIndex->second * texcoord_stride + iStride ) = texcoord_source.get( ( unsigned int )( iNewIndex->first.Index_Texcoord * texcoord_stride + iStride ) );
					};
				};

				// Add new vertex buffer source to collada
				pos_source.clear();
				normal_source.clear();
				texcoord_source.clear();
				for( unsigned int iF = 0; iF < NewVertCount * pos_stride; iF++ )
				{
					pos_source.append( *( new_pos + iF ) );
				};
				for( unsigned int iF = 0; iF < NewVertCount * normal_stride; iF++ )
				{
					normal_source.append( *( new_normal + iF ) );
				};
				for( unsigned int iF = 0; iF < NewVertCount * texcoord_stride; iF++ )
				{
					texcoord_source.append( *( new_texcoord + iF ) );
				};
				VBs[ IS_POSITION ]->getFloat_array()->setCount( NewVertCount * pos_stride );
				VBs[ IS_POSITION ]->getTechnique_common()->getAccessor()->setCount( NewVertCount );
				VBs[ IS_NORMAL ]->getFloat_array()->setCount( NewVertCount * normal_stride );
				VBs[ IS_NORMAL ]->getTechnique_common()->getAccessor()->setCount( NewVertCount );
				VBs[ IS_TEXCOORD0 ]->getFloat_array()->setCount( NewVertCount * texcoord_stride );
				VBs[ IS_TEXCOORD0 ]->getTechnique_common()->getAccessor()->setCount( NewVertCount );


				// Controller
				domLibrary_controllers_Array &arrControllers = root->getLibrary_controllers_array();
				for( unsigned int iC = 0; iC < arrControllers.getCount(); iC++ )
				{
					domController_Array &arrConts = arrControllers[ iC ]->getController_array();
					for( unsigned int i = 0; i < arrConts.getCount(); i++ )
					{
						if( arrConts[ i ]->getSkin().cast() != NULL )
						{
							if( arrConts[ i ]->getSkin()->getSource().getElement().cast() == geometry.cast() )
							{
								domSkin::domVertex_weightsRef vweights = arrConts[ i ]->getSkin()->getVertex_weights();
								domUint stride = 1;
								domInputLocalOffset_Array &arrInputs = vweights->getInput_array();
								for( unsigned int iInput = 0; iInput < arrInputs.getCount(); iInput++ )
								{
									if( stride < arrInputs[ iInput ]->getOffset() + 1 )
									{
										stride = arrInputs[ iInput ]->getOffset() + 1;
									};
								};
								domListOfUInts &vcounts = vweights->getVcount()->getValue();
								domListOfInts& vs = vweights->getV()->getValue();

								ReorderJointWeightsWithNewIndex( vcounts, vs, stride );

								vweights->setCount( mapIndex.size() );
							};
						};
					};
				};

				// Clear temporary mesh index data
				mapIndex.clear();
			};
		};

		return true;
	};

	bool SingleIndexOptimizer::ReorderJointWeightsWithNewIndex( domListOfUInts& vcounts, domListOfInts& vs, domUint stride )
	{
		domListOfUInts oldvcountoffset;
		oldvcountoffset.append( 0 );
		for( unsigned int iV = 1; iV < vcounts.getCount(); iV++ )
		{
			oldvcountoffset.append( vcounts.get( iV - 1 ) + oldvcountoffset.get( iV - 1 ) );
		};

		domListOfUInts newvcounts;
		domListOfInts newvs;
		
		std::vector< MultiIndex > vOldIndex;
		for( unsigned int i = 0; i < mapIndex.size(); i++ )
		{
			MultiIndex mi;
			vOldIndex.push_back( mi );
		};
		for( std::map< MultiIndex, domUint, MultiIndexCompareFunctor >::iterator iNewIndex = mapIndex.begin(); iNewIndex != mapIndex.end(); iNewIndex++ )
		{
			vOldIndex[ static_cast< unsigned int >( iNewIndex->second ) ].Index_Position = iNewIndex->first.Index_Position;
		};

		for( unsigned int i = 0; i < vOldIndex.size(); i++ )
		{
			domUint oldindex = vOldIndex[ i ].Index_Position;

			domUint jointcount = vcounts[ static_cast< unsigned int >( oldindex ) ];
			newvcounts.append( jointcount );

			domUint vcount = jointcount * stride;
			domUint voffset = oldvcountoffset[ static_cast< unsigned int >( oldindex ) ] * stride;
			for( unsigned int iv = 0; iv < vcount; iv++ )
			{
				newvs.append( vs.get( static_cast< unsigned int >( voffset + iv ) ) );
			};
		};

		// copy new data
		vcounts.clear();
		vs.clear();
		for( unsigned int i = 0; i < newvcounts.getCount(); i++ )
		{
			vcounts.append( newvcounts.get( i ) );
		};
		for( unsigned int i = 0; i < newvs.getCount(); i++ )
		{
			vs.append( newvs.get( i ) );
		};

		return true;
	};
};