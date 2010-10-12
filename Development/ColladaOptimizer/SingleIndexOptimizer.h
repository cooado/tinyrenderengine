#ifndef _COP_SINGLEINDEX_H__
#define _COP_SINGLEINDEX_H__

#include "OptimizerInterface.h"

namespace COP
{
	class SingleIndexOptimizer : public Optimizer
	{
	public:

		int Process( domCOLLADA* root );

	protected:

		enum InputStream
		{
			IS_POSITION = 0,

			IS_NORMAL,

			IS_TEXCOORD0,

			IS_END,
		};

		struct Triangle_Index
		{
			domUint* Original_Index[ IS_END ];

			domUint* New_Index;

			domUint Index_Count;
		};

		struct MultiIndex
		{
			domUint Index_Position;

			domUint Index_Normal;

			domUint Index_Texcoord;
		};

		struct MultiIndexCompareFunctor
		{
			bool operator() ( const MultiIndex& _left, const MultiIndex& _right ) const
			{
				if( _left.Index_Position < _right.Index_Position )
				{
					return true;
				}
				else if( _left.Index_Position > _right.Index_Position )
				{
					return false;
				}
				else
				{
					if( _left.Index_Normal < _right.Index_Normal )
					{
						return true;
					}
					else if( _left.Index_Normal > _right.Index_Normal )
					{
						return false;
					}
					else
					{
						if( _left.Index_Texcoord < _right.Index_Texcoord )
						{
							return true;
						}
						else if( _left.Index_Texcoord > _right.Index_Texcoord )
						{
							return false;
						}
						else
						{
							return false;
						};
					};
				};
				//if( ( _left.Index_Position == _right.Index_Position ) &&
				//	( _left.Index_Normal == _right.Index_Normal ) &&
				//	( _left.Index_Texcoord == _right.Index_Texcoord ) )
				//{
				//	return false;
				//}
				//else
				//{
				//	if( ( _left.Index_Position <= _right.Index_Position ) &&
				//		( _left.Index_Normal <= _right.Index_Normal ) &&
				//		( _left.Index_Texcoord <= _right.Index_Texcoord ) )
				//	{
				//		return true;
				//	}
				//	else if( ( _left.Index_Position >= _right.Index_Position ) &&
				//			( _left.Index_Normal >= _right.Index_Normal ) &&
				//			( _left.Index_Texcoord >= _right.Index_Texcoord ) )
				//	{
				//		return false;
				//	};
				//};
			};
		};

		std::map< MultiIndex, domUint, MultiIndexCompareFunctor > mapIndex;

		bool CreateSingleIndex( Triangle_Index& tri );

		/**
		* Collada defaults to CCW winding orfer of triangle, change it to CW
		*/
		bool ChangeWindingOrder( Triangle_Index& tri );

		bool ReorderJointWeightsWithNewIndex( domListOfUInts& vcounts, domListOfInts& vs, domUint stride );
	};
};

#endif