#ifndef _TRE_RENDERPIPELINE_H__
#define _TRE_RENDERPIPELINE_H__

#include "Platform.h"
#include "ControllerManager.h"

namespace TRE
{
	class RenderPipeline
	{
	public:
		virtual INT32 Initialize() = 0; 

		virtual INT32 Finalize() = 0;

		virtual INT32 Render() = 0;

		virtual ~RenderPipeline() {};
	};

	class ForwardRenderPipeline : public RenderPipeline
	{
	public:
		INT32 Initialize();

		INT32 Finalize();

		INT32 Render();

	private:
		ID3D10Effect* pEffect;
		
		ID3D10EffectMatrixVariable* pWorld;
		ID3D10EffectMatrixVariable* pView;
		ID3D10EffectMatrixVariable* pWorldViewProjection;
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

		ID3D10InputLayout* pInputLayout;
		ID3D10InputLayout* pInputLayoutSkeletal;

		ID3D10EffectTechnique* pSkeletalEffect;
	};

	class MouseRender : public MouseListener
	{
	public:
		INT32 Initialize();

		INT32 Finalize();

		INT32 Render();

		void OnMouse( const OIS::Mouse* m );

	private:

		ID3D10Effect* pEffect;

		ID3D10EffectPass* pPass;

		ID3D10EffectShaderResourceVariable* pMouseTexture;

		ID3D10EffectVectorVariable* pMouseParam;

		ID3D10ShaderResourceView* pSRVMouse;

		FLOAT32 fMousePosX;

		FLOAT32 fMousePosY;

		FLOAT32 fMouseWidth;

		FLOAT32 fMouseHeight;
	};
};

#endif
