#include "Prerequisites.h"
#include "ColladaOptimizer.h"

namespace TRE
{

	DX10Renderer g_Renderer;


	TextRenderCL::TextRenderCL( ID3DX10Font* pFont, ID3DX10Sprite* pSprite, int nLineHeight )
	{
		Init( pFont, pSprite, nLineHeight );
	}
	TextRenderCL::~TextRenderCL()
	{
		SAFE_RELEASE( m_pFontBlendState10 );
	}

	//--------------------------------------------------------------------------------------
	void TextRenderCL::Init( ID3DX10Font* pFont10, ID3DX10Sprite* pSprite10,
		int nLineHeight )
	{
		m_pFont10 = pFont10;
		m_pSprite10 = pSprite10;
		m_clr = D3DXCOLOR( 1, 1, 1, 1 );
		m_pt.x = 0;
		m_pt.y = 0;
		m_nLineHeight = nLineHeight;
		m_pFontBlendState10 = NULL;

		// Create a blend state if a sprite is passed in
		if( pSprite10 )
		{
			ID3D10Device* pDev = NULL;
			pSprite10->GetDevice( &pDev );
			if( pDev )
			{
				D3D10_BLEND_DESC StateDesc;
				ZeroMemory( &StateDesc, sizeof( D3D10_BLEND_DESC ) );
				StateDesc.AlphaToCoverageEnable = FALSE;
				StateDesc.BlendEnable[0] = TRUE;
				StateDesc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
				StateDesc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
				StateDesc.BlendOp = D3D10_BLEND_OP_ADD;
				StateDesc.SrcBlendAlpha = D3D10_BLEND_ZERO;
				StateDesc.DestBlendAlpha = D3D10_BLEND_ZERO;
				StateDesc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
				StateDesc.RenderTargetWriteMask[0] = 0xf;
				pDev->CreateBlendState( &StateDesc, &m_pFontBlendState10 );

				pDev->Release();
			}
		}
	}


	//--------------------------------------------------------------------------------------
	HRESULT TextRenderCL::DrawFormattedTextLine( const WCHAR* strMsg, ... )
	{
		WCHAR strBuffer[512];

		va_list args;
		va_start( args, strMsg );
		vswprintf_s( strBuffer, 512, strMsg, args );
		strBuffer[511] = L'\0';
		va_end( args );

		return DrawTextLine( strBuffer );
	}


	//--------------------------------------------------------------------------------------
	HRESULT TextRenderCL::DrawTextLine( const WCHAR* strMsg )
	{
		HRESULT hr;
		RECT rc;
		SetRect( &rc, m_pt.x, m_pt.y, 0, 0 );
		hr = m_pFont10->DrawText( m_pSprite10, strMsg, -1, &rc, DT_NOCLIP, m_clr );
		if( FAILED( hr ) )
			return DXTRACE_ERR_MSGBOX( L"DrawText", hr );

		m_pt.y += m_nLineHeight;

		return S_OK;
	}


	HRESULT TextRenderCL::DrawFormattedTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg, ... )
	{
		WCHAR strBuffer[512];

		va_list args;
		va_start( args, strMsg );
		vswprintf_s( strBuffer, 512, strMsg, args );
		strBuffer[511] = L'\0';
		va_end( args );

		return DrawTextLine( rc, dwFlags, strBuffer );
	}


	HRESULT TextRenderCL::DrawTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg )
	{
		HRESULT hr;
		hr = m_pFont10->DrawText( m_pSprite10, strMsg, -1, &rc, dwFlags, m_clr );
		if( FAILED( hr ) )
			return DXTRACE_ERR_MSGBOX( L"DrawText", hr );

		m_pt.y += m_nLineHeight;

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	void TextRenderCL::Begin()
	{
		if( m_pSprite10 )
		{
			D3D10_VIEWPORT VPs[D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			UINT cVPs = 1;
			ID3D10Device* pd3dDevice = NULL;
			m_pSprite10->GetDevice( &pd3dDevice );
			if( pd3dDevice )
			{
				// Set projection
				pd3dDevice->RSGetViewports( &cVPs, VPs );
				D3DXMATRIXA16 matProjection;
				D3DXMatrixOrthoOffCenterLH( &matProjection, ( FLOAT )VPs[0].TopLeftX, ( FLOAT )
					( VPs[0].TopLeftX + VPs[0].Width ), ( FLOAT )VPs[0].TopLeftY, ( FLOAT )
					( VPs[0].TopLeftY + VPs[0].Height ), 0.1f, 10 );
				m_pSprite10->SetProjectionTransform( &matProjection );

				m_pSprite10->Begin( D3DX10_SPRITE_SORT_TEXTURE );
				SAFE_RELEASE( pd3dDevice );
			}
		}


	}
	void TextRenderCL::End()
	{
		if( m_pSprite10 )
		{
			FLOAT OriginalBlendFactor[4];
			UINT OriginalSampleMask = 0;
			ID3D10BlendState* pOriginalBlendState10 = NULL;
			ID3D10Device* pd3dDevice = NULL;

			m_pSprite10->GetDevice( &pd3dDevice );
			if( pd3dDevice )
			{
				// Get the old blend state and set the new one
				pd3dDevice->OMGetBlendState( &pOriginalBlendState10, OriginalBlendFactor, &OriginalSampleMask );
				if( m_pFontBlendState10 )
				{
					FLOAT NewBlendFactor[4] = {0,0,0,0};
					pd3dDevice->OMSetBlendState( m_pFontBlendState10, NewBlendFactor, 0xffffffff );
				}
			}

			m_pSprite10->End();

			// Reset the original blend state
			if( pd3dDevice && pOriginalBlendState10 )
			{
				pd3dDevice->OMSetBlendState( pOriginalBlendState10, OriginalBlendFactor, OriginalSampleMask );
			}
			SAFE_RELEASE( pOriginalBlendState10 );
			SAFE_RELEASE( pd3dDevice );
		}
	}

	INT32 TinyRenderEngine::Initialize( HWND hwnd )
	{
		// test progressive mesh
		UINT32* index = new UINT32[3*6];
		FLOAT* vertex = new FLOAT[3*6];
		vertex[0] = 1;
		vertex[1] = 4;
		vertex[2] = 1;
		vertex[3] = 0;
		vertex[4] = 2;
		vertex[5] = 0;
		vertex[6] = 1;
		vertex[7] = 3;
		vertex[8] = 0;
		vertex[9] = 1;
		vertex[10] = 1;
		vertex[11] = 0;
		vertex[12] = 1;
		vertex[13] = 0;
		vertex[14] = 0;
		vertex[15] = 2;
		vertex[16] = 2;
		vertex[17] = 0;
		index[0] = 0;
		index[1] = 2;
		index[2] = 1;
		index[3] = 0;
		index[4] = 5;
		index[5] = 2;
		index[6] = 1;
		index[7] = 2;
		index[8] = 3;
		index[9] = 2;
		index[10] = 5;
		index[11] = 3;
		index[12] = 3;
		index[13] = 4;
		index[14] = 1;
		index[15] = 5;
		index[16] = 4;
		index[17] = 3;
		TRE::ProgressiveMesh pm;
		pm.SetInitData( index, vertex, 6, 6 );
		TRE::UINT32 fcount = 5;
		TRE::UINT32 vcount = 6;
		pm.Generate( fcount, vcount );
		UINT32* iout = 0;
		FLOAT32* vout = 0;
		UINT32 fout = 0;
		UINT32 vercout = 0;
		pm.RetrieveData( iout, vout, fout, vercout );

		// end test

		std::ifstream fin( "..\\..\\Media\\Config\\TREConfig.ini" ); 
		char path[1000];
		fin.getline( path, 1000, '\n' );

		std::string inputfile = std::string( path );
		std::string outputfile = inputfile.substr( 0, inputfile.size() - 4 ) + std::string( "_Auto_Optimized.DAE" );
		COP::OptimizeColladaFile( inputfile, outputfile, static_cast< COP::OptimizerOption >( 7 ) );

		m_Config.FilePath = outputfile;
		m_Config.ResX = 640;
		m_Config.ResY = 480;

		CreateRenderDevice( hwnd );

		return 0;
	};

	INT32 TinyRenderEngine::Finalize()
	{
		m_pRenderPipeline->Finalize();
		m_pMouseRender->Finalize();
		delete m_pRenderPipeline;
		delete m_pMouseRender;
		ControllerManager::GetInstance()->Finalize();

		SAFE_RELEASE( g_Renderer.pFont );
		SAFE_RELEASE( g_Renderer.pSprite );
		SAFE_DELETE( g_Renderer.pTextHelper );

		return 0;
	};

	INT32 TinyRenderEngine::LoadScene()
	{
		ControllerManager::GetInstance()->Initialize();
		Importer_Dom::GetInstance()->ImportFromDaeFile( m_Config.FilePath );
		m_pRenderPipeline = new ForwardRenderPipeline();
		m_pRenderPipeline->Initialize();
		m_pMouseRender = new MouseRender();
		m_pMouseRender->Initialize();

		return 0;
	};


	INT32 TinyRenderEngine::UpdateOneFrame( FLOAT64 fTime, FLOAT32 fElapsed )
	{
		return ControllerManager::GetInstance()->UpdateControllers( fTime, fElapsed );
	};

	INT32 TinyRenderEngine::RenderOneFrame()
	{
		m_pRenderPipeline->Render();
		m_pMouseRender->Render();

		Sleep( 20 );
		g_Renderer.pSwapChain->Present( 0, 0 );

		return 0;
	};

	void TinyRenderEngine::GetScreenResolution(INT32 &x, INT32 &y)
	{
		x = m_Config.ResX;
		y = m_Config.ResY;
	};

	INT32 TinyRenderEngine::CreateRenderDevice( HWND whnd )
	{
		RECT wrect;
		GetClientRect( whnd, &wrect );
		g_Renderer.HWin = whnd;

		// Swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory( &swapChainDesc, sizeof(swapChainDesc) );
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = wrect.right - wrect.left;
		swapChainDesc.BufferDesc.Height = wrect.bottom - wrect.top;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = whnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;

		if( FAILED( D3D10CreateDeviceAndSwapChain( NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 
			0, D3D10_SDK_VERSION, &swapChainDesc, &g_Renderer.pSwapChain, &g_Renderer.pDevice ) ) )
		{
			TRE_LOG("Failed to create device and swap chain.");
			return -1;
		}

		// Render target
		ID3D10Texture2D *pBackBuffer;
		if( FAILED( g_Renderer.pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), (LPVOID*)&pBackBuffer ) ) )
		{
			TRE_LOG("Failed to create back buffer.");
			return -1;
		}

		if(FAILED( g_Renderer.pDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_Renderer.pRTV )))
		{
			TRE_LOG("Failed to create render target view.");
			return false;
		}

		pBackBuffer->Release();

		// Depth stencil
		ID3D10Texture2D* pDepthStencil = NULL;
		D3D10_TEXTURE2D_DESC descDepth;
		descDepth.Width = wrect.right - wrect.left;
		descDepth.Height = wrect.bottom - wrect.top;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D10_USAGE_DEFAULT;
		descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		if( FAILED( g_Renderer.pDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil ) ) )
			return -1;

		// Create the depth stencil view
		D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
		descDSV.Format = descDepth.Format;
		if( descDepth.SampleDesc.Count > 1 )
			descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
		else
			descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		if( FAILED( g_Renderer.pDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &g_Renderer.pDSV ) ) )
			return -1;

		// Rasterizer state that enables MSAA
		D3D10_RASTERIZER_DESC RSDesc;
		RSDesc.FillMode = D3D10_FILL_SOLID;
		RSDesc.CullMode = D3D10_CULL_BACK;
		RSDesc.FrontCounterClockwise = FALSE;
		RSDesc.DepthBias = 0;
		RSDesc.SlopeScaledDepthBias = 0.0f;
		RSDesc.DepthBiasClamp= 0;
		RSDesc.DepthClipEnable = FALSE;
		RSDesc.ScissorEnable = TRUE;
		RSDesc.AntialiasedLineEnable = FALSE;
		if( descDepth.SampleDesc.Count > 1 )
			RSDesc.MultisampleEnable = TRUE;
		else
			RSDesc.MultisampleEnable = FALSE;

		ID3D10RasterizerState* pRState = NULL;
		if( FAILED( g_Renderer.pDevice->CreateRasterizerState( &RSDesc, &pRState ) ) )
			return -1;

		g_Renderer.pDevice->RSSetState( pRState );

		g_Renderer.pDevice->OMSetRenderTargets( 1, &g_Renderer.pRTV, g_Renderer.pDSV );

		D3D10_VIEWPORT vp = { 0, 0, wrect.right - wrect.left, wrect.bottom - wrect.top, 0, 1 };
		g_Renderer.pDevice->RSSetViewports( 1, &vp );

		// Text render helper
		HRESULT hr;
		V_RETURN( D3DX10CreateSprite( g_Renderer.pDevice, 500, &g_Renderer.pSprite ) );
		V_RETURN( D3DX10CreateFont( g_Renderer.pDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			L"Arial", &g_Renderer.pFont ) );
		g_Renderer.pTextHelper = new TRE::TextRenderCL( g_Renderer.pFont, g_Renderer.pSprite, 15 );

		return 0;
	}
};