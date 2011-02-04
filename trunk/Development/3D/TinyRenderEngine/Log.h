#ifndef _TRE_LOG_H__
#define _TRE_LOG_H__

#if TRE_USE_LOG == 1
#	define TRE_LOG( str ) \
		FILE* fp_log; \
		fopen_s( &fp_log, "TRE_Log.log", "a" ); \
		fprintf( fp_log, str );\
		fflush( fp_log ); \
		fclose( fp_log );
#else
#	define TRE_LOG( str )
#endif

#endif 