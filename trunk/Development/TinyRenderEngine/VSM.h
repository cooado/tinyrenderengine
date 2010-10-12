#ifndef _TRE_VSM_H__
#define _TRE_VSM_H__

#include "RenderPipeline.h"
#include "ControllerManager.h"

namespace TRE
{
	class VSMRender : public RenderPipeline, KeyboardListener
	{
	public:

		INT32 Initialize();

		INT32 Finalize();

		INT32 Render();

		void OnKeyboard( OIS::Keyboard* kb );

	private:

		/**
		*	0	VSM
		*	1	ESM
		*	2	SM
		*/
		UINT32 ShadowTechnique;

		UINT32 VSMWidth;
		UINT32 VSMHeight;

		ID3D10Texture2D* pVSMTex;
		ID3D10Texture2D* pVSMDepthStencilTex;
		ID3D10RenderTargetView* pVSMRTV;
		ID3D10ShaderResourceView* pVSMSRV;
		ID3D10DepthStencilView* pVSMDSV;

		ID3D10Effect* pEffect;
		ID3D10EffectPass* pCreateVSM;
		ID3D10EffectPass* pSkeletalVSM;
		ID3D10EffectPass* pStaticVSM;

		ID3D10EffectScalarVariable* pShadowTechnique;
		ID3D10EffectMatrixVariable* pWorld;
		ID3D10EffectMatrixVariable* pView;
		ID3D10EffectMatrixVariable* pWorldViewProjection;
		ID3D10EffectMatrixVariable* pLightView;
		ID3D10EffectMatrixVariable* pLightProjection;
		ID3D10EffectMatrixVariable* pLightWorldViewProjection;
		ID3D10EffectMatrixVariable* pLightViewProjection;

		ID3D10EffectVectorVariable* pDirLight;
		ID3D10EffectVectorVariable* pLightDiffuse;

		ID3D10EffectVectorVariable* pEmission;
		ID3D10EffectVectorVariable* pAmbient;
		ID3D10EffectVectorVariable* pDiffuse;
		ID3D10EffectVectorVariable* pSpecular;
		ID3D10EffectScalarVariable* pShininess;
		ID3D10EffectScalarVariable* pUseDiffuseTexture;

		ID3D10EffectShaderResourceVariable* pDiffuseTex;
		ID3D10EffectShaderResourceVariable* pJointIndexBuffer;
		ID3D10EffectShaderResourceVariable* pJointWeightBuffer;
		ID3D10EffectShaderResourceVariable* pJointBindBuffer;

		ID3D10EffectShaderResourceVariable* pVSM;

		ID3D10InputLayout* pInputLayout;
		ID3D10InputLayout* pInputLayoutSkeletal;

	};
};

#endif