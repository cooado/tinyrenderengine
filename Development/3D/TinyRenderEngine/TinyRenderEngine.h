#ifndef _TRE_TINYRENDERENGNE_H__
#define _TRE_TINYRENDERENGNE_H__

#include "Platform.h"
#include "RenderPipeline.h"
#include <string>

namespace TRE
{
	//--------------------------------------------------------------------------------------
	// Manages the insertion point when drawing text
	//--------------------------------------------------------------------------------------
	class TextRenderCL
	{
	public:
		TextRenderCL( ID3DX10Font* pFont10, ID3DX10Sprite* pSprite10, int nLineHeight = 15 );
		~TextRenderCL();

		void    Init(ID3DX10Font* pFont10 = NULL, ID3DX10Sprite* pSprite10 = NULL, int nLineHeight = 15 );

		void    SetInsertionPos( int x, int y )
		{
			m_pt.x = x; m_pt.y = y;
		}
		void    SetForegroundColor( D3DXCOLOR clr )
		{
			m_clr = clr;
		}

		void    Begin();
		HRESULT DrawFormattedTextLine( const WCHAR* strMsg, ... );
		HRESULT DrawTextLine( const WCHAR* strMsg );
		HRESULT DrawFormattedTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg, ... );
		HRESULT DrawTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg );
		void    End();

	protected:
		ID3DX10Font* m_pFont10;
		ID3DX10Sprite* m_pSprite10;
		D3DXCOLOR m_clr;
		POINT m_pt;
		int m_nLineHeight;

		ID3D10BlendState* m_pFontBlendState10;
	};

	struct DX10Renderer
	{
		ID3D10Device* pDevice;

		TextRenderCL* pTextHelper;

		ID3D10RenderTargetView* pRTV;

		ID3D10DepthStencilView* pDSV;

		HWND HWin;

		IDXGISwapChain*	pSwapChain;

		ID3DX10Font* pFont;

		ID3DX10Sprite* pSprite;
	};

	extern DX10Renderer g_Renderer;

	struct EngineConfig
	{
		std::string FilePath;

		INT ResX;

		INT ResY;
	};

	class _declspec(dllexport) TinyRenderEngine
	{
		DECLARE_SINGLETON( TinyRenderEngine )

	public:
		INT32 Initialize( HWND hwnd );
		
		INT32 CreateRenderDevice( HWND whnd );

		INT32 Finalize();

		INT32 LoadScene();

		INT32 UpdateOneFrame( FLOAT64 fTime, FLOAT32 fElapsed );

		INT32 RenderOneFrame();

		void GetScreenResolution( INT32& x, INT32& y );

	private:

		EngineConfig m_Config;

		RenderPipeline* m_pRenderPipeline;

		MouseRender* m_pMouseRender;

	};
};

#endif