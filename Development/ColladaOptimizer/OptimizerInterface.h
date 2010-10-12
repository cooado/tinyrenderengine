#ifndef _COP_OPTIMIZERINTERFACE_H__
#define _COP_OPTIMIZERINTERFACE_H__

#include "OptimizerCommonHeader.h"

namespace COP
{
	class Optimizer
	{
	public:

		virtual int Process( domCOLLADA* root ) = 0;

		virtual ~Optimizer(){};
	};
};

#endif