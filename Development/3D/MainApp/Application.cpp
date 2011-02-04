#include "Application.h"

#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3dx10.lib")


Application::Application()
{
}

Application::~Application()
{
	Finalize();
}

LRESULT CALLBACK Application::WndProc( HWND hWnd , UINT message , WPARAM wParam , LPARAM lParam)
{ 
    switch(message){ 
    case WM_CLOSE: 
    case WM_DESTROY: 
        PostQuitMessage(0); 
        break;; 
    } 
    return DefWindowProc(hWnd,message,wParam,lParam); 
} 

bool Application::CreateD3DWindow(int width, int height)
{ 
	m_Width = width;
	m_Height = height;
	
    // Create A Window Class Structure 
    WNDCLASSEX wc; 
	ZeroMemory(&wc, sizeof(wc));		
    wc.cbSize = sizeof(wc);				
    wc.hInstance = GetModuleHandle(NULL);		
    wc.lpfnWndProc = WndProc;					
    wc.lpszClassName = L"GPORG";						
    wc.style = CS_HREDRAW | CS_VREDRAW;			
	
    // Register Window Class 
    RegisterClassEx(&wc); 
	
    // Create Window 
    m_Hwnd = CreateWindowEx(0, 
        L"GPORG", L"GameProgrammer.org Direct3D 10 Tutorial", 
        WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 
        NULL,NULL,wc.hInstance,0); 
	
    return true; 
} 

bool Application::Initialize()
{
	TRE::TinyRenderEngine::GetInstance()->Initialize( m_Hwnd );
	TRE::TinyRenderEngine::GetInstance()->LoadScene();

	return true; 
} 

void Application::Render( TRE::FLOAT64 time, TRE::FLOAT32 deltatime )
{
	TRE::TinyRenderEngine::GetInstance()->UpdateOneFrame( time, deltatime );
	TRE::TinyRenderEngine::GetInstance()->RenderOneFrame();
} 

void Application::MainLoop()
{ 
    MSG msg; 
	long prevTime = GetTickCount(), curTime = GetTickCount();
	TRE::FLOAT64 GlobalTime = 0.f;
	TRE::FLOAT32 DeltaTime = 0.f;
	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message==WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{
			curTime = GetTickCount();
			DeltaTime = ( curTime - prevTime )/ 1000.f;
			Render( GlobalTime, DeltaTime );
			prevTime = curTime;
			GlobalTime += DeltaTime;
		}
	}
} 

void Application::Finalize()
{
	TRE::TinyRenderEngine::GetInstance()->Finalize();
}

INT WINAPI WinMain( HINSTANCE , HINSTANCE , LPSTR , INT )
{ 
	Application app;
	
	// Border takes up 16 pixels, and title bar 38 pixels
	if( app.CreateD3DWindow( 656, 518 ) )
	{
		if(app.Initialize())
		{
			app.MainLoop();
		}
	}
	
    return 0; 
}
