#pragma once

#include <windows.h> 

#include "..\TinyRenderEngine\Prerequisites.h"

#define Error(X) MessageBox(NULL, X, L"Error", MB_OK)

class Application
{
public:

	Application();
	~Application();

	bool CreateD3DWindow(int width, int height);

	bool Initialize(); 
	void Render( TRE::FLOAT64 time, TRE::FLOAT32 deltatime);
	void MainLoop();

private:

	static LRESULT CALLBACK WndProc( HWND hWnd , UINT message , WPARAM wParam , LPARAM lParam);
	void Finalize();

	HWND						m_Hwnd; 
	int							m_Width, m_Height;
};
