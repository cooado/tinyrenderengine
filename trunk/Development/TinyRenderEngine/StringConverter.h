#include <string>
#include <tchar.h>
#include <windows.h>

char* WideToUTF8(const WCHAR* src)   
{   
    int nBufSize = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, 0, FALSE);   
    char *szBuf = new char[nBufSize];   
    WideCharToMultiByte(CP_UTF8, 0, src, -1, szBuf, nBufSize, 0, FALSE);   

    return szBuf;   
}  

WCHAR* UTF8ToWide(const char* src)   
{   
    int nBufSize = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);   
    WCHAR *szBuf = new WCHAR[nBufSize];   
    MultiByteToWideChar(CP_UTF8, 0, src, -1, szBuf, nBufSize);   

    return szBuf;   
}  

//void main()
//{
//	std::string utf8 = std::string( "some utf8 text" );
//	std::wstring utf16 = std::wstring( L"this is wide characters" );
//
//	char* res8 = WideToUTF8( utf16.c_str() );
//	WCHAR* res16 = UTF8ToWide( utf8.c_str() );
//};