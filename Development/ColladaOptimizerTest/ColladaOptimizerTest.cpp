#include "ColladaOptimizer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>



int main(const int argc, const char* argv[] )
{
	//if( argc != 4 )
	//{
	//	return -1;
	//};

	//std::string input( argv[ 1 ] );
	//std::string output( argv[ 2 ] );
	//unsigned int iMode = atoi( argv[ 3 ] );
	//COP::OptimizeColladaFile( input, output, static_cast< COP::OptimizerOption >( iMode ) );

	COP::OptimizeColladaFile( "D:\\Game_Development\\VS_Workspace\\TinyRenderEngine_v2.0\\Media\\Collada\\Capsule.DAE", 
		"D:\\Game_Development\\VS_Workspace\\TinyRenderEngine_v2.0\\Media\\Collada\\Capsule_Optimized.dae",
		static_cast< COP::OptimizerOption >( 7 ) );
	return 0;
};