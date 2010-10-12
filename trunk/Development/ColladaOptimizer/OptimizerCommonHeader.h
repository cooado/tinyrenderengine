#ifndef _COP_OPTIMIZERCOMMONHEADER_H__
#define _COP_OPTIMIZERCOMMONHEADER_H__

#ifndef BOOST_ALL_NO_LIB
#define BOOST_ALL_NO_LIB
#endif

//  The file contains a character that cannot be represented in the current code page (936)
#pragma warning ( disable : 4819 )

// warning C4355: 'this' : used in base member initializer list
#pragma warning ( disable : 4355 )

#include <vector>
#include <map>
#include <set>

#include <dae.h>
#include <dom/domCOLLADA.h>
#include <dom/domConstants.h>
#include <dom/domTypes.h>

#define COP_USE_LOG 1

#if COP_USE_LOG == 1
#	define COP_LOG( str ) \
		FILE* fp_log; \
		fopen_s( &fp_log, "COP_Log.log", "a" ); \
		fprintf( fp_log, str );\
		fflush( fp_log ); \
		fclose( fp_log );
#else
#	define COP_LOG( str )
#endif

#endif