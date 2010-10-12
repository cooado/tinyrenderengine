#include "ColladaOptimizer.h"
#include "OptimizerCommonHeader.h"
#include "ChangeTextureCoordinateOptimizer.h"
#include "SingleIndexOptimizer.h"
#include "SwitchCoordinateSystemOptimizer.h"

namespace COP
{
	bool OptimizeColladaFile( std::string input, std::string output, OptimizerOption options )
	{
		DAE dae;
		domCOLLADA* root = dae.open( input );
		if (!root) {
			return false;
		};

		if( ( options & OO_SINGLE_INDEX_MESH_BUFFERS ) != 0 )
		{
			SingleIndexOptimizer sio;
			sio.Process( root );
		};

		if( ( options & OO_CHANGE_COORDINATE_SYSTEM ) != 0 )
		{
			SwitchCoordinateSystemOptimizer scso;
			scso.Process( root );
		};

		if( ( options & OO_CHANGE_TEXTURE_COORDINATE ) != 0 )
		{
			ChangeTextureCoordinateOptimizer ctco;
			ctco.Process( root );
		};

		// Add modification information to collada

		dae.add( output );
		dae.writeTo( input, output );

		return true;
	};

};
