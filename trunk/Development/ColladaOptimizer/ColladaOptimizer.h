#ifndef _COLLADA_OPTIMIZER_H__
#define _COLLADA_OPTIMIZER_H__

#include <string>

namespace COP
{

	enum OptimizerOption
	{
		/************************************************************************************
		* Re-arrange mesh buffers so that all vertex buffer sources have a single index buffer, as DirectX 
		* only supports one index buffer for the IA stage
		************************************************************************************/
		OO_SINGLE_INDEX_MESH_BUFFERS = 1 << 0,

		/***********************************************************************************
		* Collada defaults to right-handed coordinate system.
		* Change to left-handed system with Y axis as the up axis.
		*
		* @Note: it no longer conforms to collada specifications any more
		***********************************************************************************/
		OO_CHANGE_COORDINATE_SYSTEM = 1 << 1,

		/**********************************************************************************
		* Collada defaults to OpenGL style texture coordinate.
		* Change to DirectX style.
		*
		* @Note: it no longer conforms to collada specifications any more
		***********************************************************************************/
		OO_CHANGE_TEXTURE_COORDINATE = 1 << 2,
	};

	bool OptimizeColladaFile( std::string input, std::string output, OptimizerOption options);

};

#endif