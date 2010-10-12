#include "ChangeTextureCoordinateOptimizer.h"

namespace COP
{
	int ChangeTextureCoordinateOptimizer::Process( domCOLLADA* root )
	{
		std::set< domSource* > setSources;
		domLibrary_geometries_Array &arrGeoms = root->getLibrary_geometries_array();
		for( unsigned int iLibraryGeom = 0; iLibraryGeom < arrGeoms.getCount(); iLibraryGeom++ )
		{
			domLibrary_geometriesRef libgeoms = arrGeoms[ iLibraryGeom ];
			domGeometry_Array &geoms = libgeoms->getGeometry_array();
			for( unsigned int iGeom = 0; iGeom < geoms.getCount(); iGeom++ )
			{
				domGeometryRef geom = geoms[ iGeom ];
				domTriangles_Array &triangles = geom->getMesh()->getTriangles_array();
				for( unsigned int iTri = 0; iTri < triangles.getCount(); iTri++ )
				{
					domTrianglesRef triangle = triangles[ iTri ];
					domInputLocalOffset_Array &inputs = triangle->getInput_array();
					for( unsigned int iInput = 0; iInput < inputs.getCount(); iInput++ )
					{
						domInputLocalOffsetRef input = inputs[ iInput ];
						if( std::string( input->getSemantic() ) == std::string( "TEXCOORD" ) )
						{
							domSource* s = static_cast< domSource* >( input->getSource().getElement().cast() );
							if( setSources.find( s ) == setSources.end() )
							{
								domListOfFloats &arrFloat = s->getFloat_array()->getValue();
								domUint stride = s->getTechnique_common()->getAccessor()->getStride();
								domUint count = s->getTechnique_common()->getAccessor()->getCount();
								for( domUint iUV = 0; iUV < count; iUV++ )
								{
									domFloat TexV = 1.0f - arrFloat.get( static_cast< size_t >( iUV * stride + 1 ) );
									arrFloat.set( static_cast< size_t >( iUV * stride + 1 ), TexV );
								};
								setSources.insert( s );
							}
							else
							{
								break;
							};
						};
					};
				};
			};
		};

		return 0;
	};
};