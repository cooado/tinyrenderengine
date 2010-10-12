#ifndef _COP_CHANGETEXTURECOORDINATEOPTIMIZER_H__
#define _COP_CHANGETEXTURECOORDINATEOPTIMIZER_H__

#include "OptimizerInterface.h"

namespace COP
{
	class ChangeTextureCoordinateOptimizer : public Optimizer
	{
	public:

		int Process( domCOLLADA* root );
	};
};

#endif