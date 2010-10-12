#ifndef _TRE_PREQUISITES_H__
#define _TRE_PREQUISITES_H__

#include "Platform.h"

//#include "COLLADAFW.h"
//#include "COLLADASaxFWLLoader.h"
//#include "COLLADAFWIWriter.h"

#include <dae.h>
#include <dom/domCOLLADA.h>
#include <dom/domConstants.h>
#include <dom/domTypes.h>
#include <dom/domProfile_COMMON.h>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <fstream>
#include "TCHAR.h"

#include "D3D10.h"
#include "D3DX10.h"
#include "DxErr.h"

#include "ConvertUTF.h"

/**
*	Value	Description
*	0		Disable logging
*	1		Enable logging
*/
#define TRE_USE_LOG 1


namespace TRE
{

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#define FAILED(hr) (((HRESULT)(hr)) < 0)

#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return false; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    

	class Light;

	struct Camera;

	class Mesh;

	struct StaticGeometry;

	struct SkeletalGeometry;

	struct Skin;

	struct Skeleton;

	class Element;

	class Node;

	struct PhongParam;

	struct Scene;

	class Animation;

	struct ResourceDatabase;

	class RenderPipeline;

	class ResourceFactoryController;

	static void SplitString( std::string input, char seperator, std::vector< std::string >& out )
	{
		UINT32 start = 0;
		UINT32 end = 0;
		for( ; end < input.size(); end++ )
		{
			if( input[ end ] == seperator )
			{
				if( end - start > 0 )
				{
					out.push_back( input.substr( start, end - start ) );
					start = end + 1;
				}
				else
				{
					start = end + 1;
				};
			};
		};
		if( end - start > 0 )
		{
			out.push_back( input.substr( start, end - start ) );
		};
	};

	static 	D3DXVECTOR4 VectorMatrixMultiply( D3DXVECTOR4& v, D3DXMATRIX& m )
	{
		D3DXVECTOR4 vec;
		vec.x = v.x * m._11 + v.y * m._21 + v.z * m._31 + v.w * m._41;
		vec.y = v.x * m._12 + v.y * m._22 + v.z * m._32 + v.w * m._42;
		vec.z = v.x * m._13 + v.y * m._23 + v.z * m._33 + v.w * m._43;
		vec.w = v.x * m._14 + v.y * m._24 + v.z * m._34 + v.w * m._44;
		
		return vec;
	};

};

#include "Log.h"
#include "ResourceManager.h"
#include "RenderPipeline.h"
#include "Importer_Dom.h"
#include "TinyRenderEngine.h"
#include "ControllerManager.h"
#include "ImageSpaceSubsurfaceScattering.h"
#include "VolumeDecalRenderer.h"
#include "UtilityFunctions.h"
#include "VSM.h"
#include "ProgressiveMesh.h"

#endif