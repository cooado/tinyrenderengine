#ifndef _TRE_VOLUMEDECALRENDERER_H__
#define _TRE_VOLUMEDECALRENDERER_H__

#include "RenderPipeline.h"
#include "ControllerManager.h"
#include <vector>

namespace TRE
{
	class VolumeDecalRenderer : public RenderPipeline, public MouseListener
	{
	public:

		INT32 Initialize();

		INT32 Finalize();

		INT32 Render();

		void OnMouse( const OIS::Mouse* m );

	private:
		
		struct Decal
		{
			D3DXMATRIX matrix;
			D3DXVECTOR3 position;
			FLOAT32 radius;
			D3DXCOLOR color;
		};

		std::vector< Decal > m_vDecals;

		ID3D10Texture3D* pDecalTexture;
		ID3D10Texture2D* pDepthTexture;

		ID3D10RenderTargetView* pRTVDepth;

		ID3D10Effect* pEffect;
		ID3D10EffectPass* pScenePass;
		ID3D10EffectPass* pDecalPass;

		ID3D10EffectMatrixVariable* pWorldViewProjection;
		ID3D10EffectMatrixVariable* pViewProjection;

		ID3D10EffectVectorVariable* pDecalPos;
		ID3D10EffectScalarVariable* pDecalRadius;
		ID3D10EffectVectorVariable* pDecalColor;
		ID3D10EffectMatrixVariable* pDecalMatrix;
		ID3D10EffectVectorVariable* pPixelSize;

		ID3D10EffectShaderResourceVariable* pBase;
		ID3D10EffectShaderResourceVariable* pDepth;
		ID3D10EffectShaderResourceVariable* pDecal;

		ID3D10ShaderResourceView* pSRVDepth;
		ID3D10ShaderResourceView* pSRVDecal;

		ID3D10InputLayout* pILScene;

		FLOAT32 fPixelSize[ 2 ];
	};
};

#endif