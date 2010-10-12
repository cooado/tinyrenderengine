#ifndef _TRE_IMAGESPACESUBSURFACESCATTERING_H__
#define _TRE_IMAGESPACESUBSURFACESCATTERING_H__

#include "RenderPipeline.h"
#include "ControllerManager.h"

namespace TRE
{
	class ImageSpaceSubsurfaceScattering : public RenderPipeline, KeyboardListener
	{
	public:

		INT32 Initialize();

		INT32 Finalize();

		INT32 Render();

		void OnKeyboard( OIS::Keyboard* kb );

	private:
		ID3D10Effect* pEffect;
		
		ID3D10EffectMatrixVariable* pWorld;
		ID3D10EffectMatrixVariable* pView;
		ID3D10EffectMatrixVariable* pProjection;
		ID3D10EffectMatrixVariable* pWorldViewProjection;
		ID3D10EffectVectorVariable* pLightPos;
		ID3D10EffectVectorVariable* pLightDiffuseColor;
		
		ID3D10EffectScalarVariable* pUseSSSS;
		ID3D10EffectVectorVariable* pEmission;
		ID3D10EffectVectorVariable* pAmbient;
		ID3D10EffectVectorVariable* pDiffuse;
		ID3D10EffectVectorVariable* pSpecular;
		ID3D10EffectScalarVariable* pShininess;
		ID3D10EffectScalarVariable* pUseDiffuseTexture;
		ID3D10EffectVectorVariable* pcatterRadiusSquared;
		ID3D10EffectVectorVariable* pLightSourceResolution;
		ID3D10EffectScalarVariable* pSplatRadius;
		ID3D10EffectVectorVariable* pTangentHalfFOVXY;

		ID3D10EffectShaderResourceVariable* pScatteredSubsurfaceLight;
		ID3D10EffectShaderResourceVariable* pLightSource;
		ID3D10EffectShaderResourceVariable* pLightDiffuse;
		ID3D10EffectShaderResourceVariable* pGBuffer;
		ID3D10EffectShaderResourceVariable* pScatterTexture;

		ID3D10Texture2D* pTexScatteredSubsurfaceLight;
		ID3D10Texture2D* pTexLightSource;
		ID3D10Texture2D* pTexLightDiffuse;
		ID3D10Texture2D* pTexGBuffer;

		ID3D10RenderTargetView* pRTVScatteredSubsurfaceLight;
		ID3D10RenderTargetView* pRTVLightSource;
		ID3D10RenderTargetView* pRTVLightDiffuse;
		ID3D10RenderTargetView* pRTVGBuffer;

		ID3D10ShaderResourceView* pSRVScatteredSubsurfaceLight;
		ID3D10ShaderResourceView* pSRVLightSource;
		ID3D10ShaderResourceView* pSRVLightDiffuse;
		ID3D10ShaderResourceView* pSRVGBuffer;

		// Depth stencil buffer
		ID3D10Texture2D* pDSLightSource;
		ID3D10DepthStencilView* pDSVLightSource;
		ID3D10Texture2D* pDSScattered;
		ID3D10DepthStencilView* pDSVScattered;

		ID3D10ShaderResourceView* pSRV;
		ID3D10InputLayout* pInputLayout;

		UINT32 LightSourceWidth;
		UINT32 LightSourceHeight;
		UINT32 ScatterWidth;
		UINT32 ScatterHeight;
		bool m_bUseSSSS;
	};
};

#endif