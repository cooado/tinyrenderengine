#ifndef _TRE_PLATFORM_H__
#define _TRE_PLATFORM_H__

#ifndef BOOST_ALL_NO_LIB
#define BOOST_ALL_NO_LIB
#endif

/************************************************************
 Nameless struct/union used in OpenCollada
************************************************************/
#pragma warning( disable : 4201 )
/************************************************************
 COLLADASaxFWL::SidTreeNode::SidIdentifier : assignment operator could not be generated
************************************************************/
#pragma warning( disable : 4512 )

//  The file contains a character that cannot be represented in the current code page (936)
#pragma warning ( disable : 4819 )

// warning C4355: 'this' : used in base member initializer list
#pragma warning ( disable : 4355 )

// For PIX
//#define D3D10_SHADER_DEBUG
//#define D3D10_SHADER_SKIP_OPTIMIZATION

#include <assert.h>

namespace TRE
{
	typedef int INT32;

	typedef unsigned int UINT32;

	typedef unsigned char UINT8;

	typedef float FLOAT32;

	typedef double FLOAT64;

	typedef unsigned short UINT16;

	#define DECLARE_SINGLETON(ClassName) \
	private: \
		ClassName(){}; \
		ClassName( const ClassName& c){}; \
		ClassName& operator=( const ClassName& ){ return *this; }; \
		~ClassName(){}; \
	public: \
	static ClassName* GetInstance() \
	{ \
		static ClassName sInstance; \
		return &sInstance; \
	}; 

	#define TRE_ASSERT(x) assert(x)

};

#endif