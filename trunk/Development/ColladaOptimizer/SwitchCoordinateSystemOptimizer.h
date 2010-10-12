#ifndef _COP_SWITCHCOORDINATESYSTEMOPTIMIZER_H__
#define _COP_SWITCHCOORDINATESYSTEMOPTIMIZER_H__

#include "OptimizerInterface.h"

namespace COP
{
	/**
	* X_UP
	*		x   z  
	*		|  /
	*		| /
	*		o - - y
	*
	* Y_UP													
	*		y   x											y   z
	*		|  /			==========================>		|  /	
	*		| /												| /
	*		o - - z											o - - x
	*
	* Z_UP
	*		z   y
	*		|  /
	*		| /
	*		o - - x
	*/

	class SwitchCoordinateSystemOptimizer : public Optimizer
	{
	public:

		int Process( domCOLLADA* root );

	protected:

		enum FloatListType
		{
			Translate,

			Rotate,

			Matrix,
		};

		void SwitchCoordinateSystemForFloatList( domListOfFloats& floatlist, FloatListType type, const domUpAxisType upaxis );

		bool SwitchCoordinateSystemForGeometryLibrary( domLibrary_geometriesRef geometries, const domUpAxisType upaxis );

		bool SwitchCoordinateSystemForVisualSceneLibrary( domLibrary_visual_scenesRef visualscenes, const domUpAxisType upaxis );

		bool SwitchCoordinateSystemForAnimationLibrary( domLibrary_animationsRef animations, const domUpAxisType upaxis );
			
		bool SwitchCoordinateSystemForControllerLibrary( domLibrary_controllersRef controllers, const domUpAxisType upaxis );

		bool SwitchCoordinateSystemForNode( domNodeRef node, const domUpAxisType upaxis );

		bool SwitchCoordinateSystemForAnimation( domAnimationRef anim, const domUpAxisType upaxis );
	};
};

#endif